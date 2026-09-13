#!/usr/bin/env python3
"""Prompt03 D2 shadow analysis.

This module keeps the estimator trajectory fixed and treats every D2 quantity
as a diagnostic.  It recomputes the Prompt02 within-frame correction, uses a
single global SE(3) alignment, provides overlapping and non-overlapping local
windows, and uses deterministic contiguous-block bootstrap intervals.
"""

import argparse
import csv
import json
import math
import pathlib
import random
import statistics
import sys
from collections import defaultdict

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3

DELTAS = (1.0, 5.0, 10.0)
REFERENCES = (3.0, 5.0, 10.0, 20.0)
PRIOR_THRESHOLDS = (0.1, 0.3, 1.0, 3.0, 10.0)
ASSOCIATION_MAX_DIFF = 0.1
BOOTSTRAP_SEED = 20260913
BOOTSTRAP_REPS = 1000


def number(row, name):
    try:
        return float(row.get(name, ""))
    except (TypeError, ValueError):
        return math.nan


def integer(row, name):
    value = number(row, name)
    return int(value) if math.isfinite(value) else -1


def finite(values):
    return [float(value) for value in values if math.isfinite(float(value))]


def percentile(values, fraction):
    values = finite(values)
    return float(np.percentile(np.asarray(values), fraction * 100.0)) if values else None


def average_rank(values):
    order = sorted(range(len(values)), key=lambda index: values[index])
    ranks = [0.0] * len(values)
    cursor = 0
    while cursor < len(order):
        end = cursor + 1
        while end < len(order) and values[order[end]] == values[order[cursor]]:
            end += 1
        rank = 0.5 * (cursor + end - 1) + 1.0
        for position in range(cursor, end):
            ranks[order[position]] = rank
        cursor = end
    return ranks


def spearman(x_values, y_values):
    pairs = [(float(x), float(y)) for x, y in zip(x_values, y_values)
             if math.isfinite(float(x)) and math.isfinite(float(y))]
    if len(pairs) < 2:
        return {"rho": None, "n": len(pairs)}
    x_rank = np.asarray(average_rank([pair[0] for pair in pairs]), dtype=float)
    y_rank = np.asarray(average_rank([pair[1] for pair in pairs]), dtype=float)
    x_rank -= x_rank.mean()
    y_rank -= y_rank.mean()
    denominator = np.linalg.norm(x_rank) * np.linalg.norm(y_rank)
    if denominator <= 0.0:
        return {"rho": None, "n": len(pairs)}
    return {"rho": float(x_rank.dot(y_rank) / denominator), "n": len(pairs)}


def load_rows(path, version):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or "schema_version" not in reader.fieldnames:
            raise ValueError(f"{path}: missing schema_version")
        rows = [row for row in reader if row.get("frame", "") != ""]
    versions = {integer(row, "schema_version") for row in rows}
    if versions != {version}:
        raise ValueError(f"{path}: expected schema_version={version}, got {sorted(versions)}")
    rows.sort(key=lambda row: number(row, "timestamp"))
    return rows


def load_d2(path):
    rows = load_rows(path, 3)
    if any(integer(row, "iteration") != 0 for row in rows):
        raise ValueError(f"{path}: D2 rows must be first native iteration only")
    frames = [integer(row, "frame") for row in rows]
    if len(frames) != len(set(frames)):
        raise ValueError(f"{path}: expected one D2 row per frame")
    return rows


def projector_from_raw(row, mode):
    values = [number(row, f"P_weak_{mode}_{index}") for index in range(9)]
    return np.asarray(values, dtype=float).reshape((3, 3))


def weak_basis_from_raw(row, mode):
    rank = integer(row, f"weak_rank_{mode}")
    if rank <= 0:
        return np.zeros((3, 0))
    normalized = [number(row, f"normalized_lambda_{mode}_{index}") for index in range(3)]
    basis = np.asarray([number(row, f"raw_{mode}_basis_{index}") for index in range(9)], dtype=float)
    basis = basis.reshape((3, 3))
    indices = [index for index, value in enumerate(normalized) if value < 0.1]
    return basis[:, indices[:rank]]


def valid_projector(value, rank):
    return (rank > 0 and np.isfinite(value).all() and
            np.linalg.norm(value - value.T) <= 1e-6 and
            np.linalg.norm(value @ value - value) <= 1e-5 and
            abs(float(np.trace(value)) - rank) <= 1e-5)


def summary_stats(values):
    values = finite(values)
    if not values:
        return {"n": 0, "median": None, "p2_5": None, "p95": None,
                "max": None}
    return {"n": len(values), "median": statistics.median(values),
            "p2_5": percentile(values, 0.025), "p95": percentile(values, 0.95),
            "max": max(values)}


def angle_stats(previous, current):
    if previous.shape[1] == 0 or current.shape[1] == 0 or previous.shape[1] != current.shape[1]:
        return None
    singular = np.linalg.svd(previous.T @ current, compute_uv=False)
    angles = np.degrees(np.arccos(np.clip(singular, 0.0, 1.0)))
    return {"max_deg": float(np.max(angles)), "mean_deg": float(np.mean(angles))}


def corrected_prompt02(raw_rows):
    grouped = defaultdict(list)
    for row in raw_rows:
        grouped[integer(row, "frame")].append(row)
    result = {"raw_rows": len(raw_rows), "modes": {}}
    for mode in ("rot", "trans"):
        transitions = []
        rank_zero_same = 0
        same_nonzero = {"1": [], "2": []}
        rank_change = 0
        for frame_rows in grouped.values():
            frame_rows.sort(key=lambda row: integer(row, "iteration"))
            for previous, current in zip(frame_rows, frame_rows[1:]):
                if integer(previous, "valid") != 1 or integer(current, "valid") != 1:
                    continue
                previous_rank = integer(previous, f"weak_rank_{mode}")
                current_rank = integer(current, f"weak_rank_{mode}")
                transitions.append((previous_rank, current_rank))
                if previous_rank != current_rank:
                    rank_change += 1
                    continue
                if current_rank == 0:
                    rank_zero_same += 1
                    continue
                if current_rank not in (1, 2):
                    continue
                previous_projector = projector_from_raw(previous, mode)
                current_projector = projector_from_raw(current, mode)
                if valid_projector(previous_projector, previous_rank) and valid_projector(current_projector, current_rank):
                    distance = float(np.linalg.norm(current_projector - previous_projector))
                    angle = angle_stats(weak_basis_from_raw(previous, mode),
                                        weak_basis_from_raw(current, mode))
                    if angle is not None:
                        same_nonzero[str(current_rank)].append((distance, angle))
        physical = {}
        for rank, values in same_nonzero.items():
            physical[f"rank{rank}_to_rank{rank}"] = {
                "count": len(values),
                "projector_distance": summary_stats([value[0] for value in values]),
                "principal_angle_max_deg": summary_stats([value[1]["max_deg"] for value in values]),
                "principal_angle_mean_deg": summary_stats([value[1]["mean_deg"] for value in values]),
            }
        denominator = len(transitions)
        result["modes"][mode] = {
            "valid_adjacent_transition_denominator": denominator,
            "rank_change_numerator": rank_change,
            "rank_change_rate": rank_change / denominator if denominator else None,
            "rank0_to_rank0_separate": rank_zero_same,
            "same_nonzero_rank": physical,
        }
    return result


def run_summary(active, times):
    lengths = []
    seconds = []
    cursor = 0
    for index, value in enumerate(active + [False]):
        if value and cursor == 0:
            cursor = index + 1
        if not value and cursor:
            length = index + 1 - cursor
            lengths.append(length)
            if length > 1 and index - length >= 0:
                seconds.append(float(times[index - 1] - times[index - length]))
            else:
                seconds.append(0.0)
            cursor = 0
    return {"runs": len(lengths), "median_frames": statistics.median(lengths) if lengths else 0,
            "p95_frames": percentile(lengths, 0.95), "max_frames": max(lengths) if lengths else 0,
            "max_seconds": max(seconds) if seconds else 0.0}


def gamma_report(rows, prefix):
    times = np.asarray([number(row, "timestamp") for row in rows], dtype=float)
    result = {"counterfactual_label": prefix}
    for mode in ("rot", "trans"):
        mode_report = {}
        kappas = [number(row, f"{prefix}_{mode}") for row in rows]
        if prefix == "dcreg_schur_kappa":
            kappas = [number(row, f"dcreg_schur_kappa_{mode}") for row in rows]
        for reference in REFERENCES:
            gammas = [0.0 if value == math.inf else
                      (min(1.0, math.sqrt(reference / value))
                       if value > 0 and math.isfinite(value) else math.nan)
                      for value in kappas]
            valid = [value for value in gammas if math.isfinite(value)]
            active = [value < 1.0 for value in gammas]
            mode_report[str(int(reference))] = {
                "frames": len(valid), "fraction_attenuated": sum(active) / len(valid) if valid else None,
                "fraction_below_0_75": sum(value < 0.75 for value in valid) / len(valid) if valid else None,
                "fraction_below_0_5": sum(value < 0.5 for value in valid) / len(valid) if valid else None,
                "fraction_below_0_25": sum(value < 0.25 for value in valid) / len(valid) if valid else None,
                "median": statistics.median(valid) if valid else None,
                "p10": percentile(valid, 0.10), "minimum": min(valid) if valid else None,
                "longest_continuous": run_summary(active, times),
            }
        result[mode] = mode_report
    return result


def nearest_index(times, target, max_diff=ASSOCIATION_MAX_DIFF):
    if len(times) == 0:
        return None
    index = int(np.searchsorted(times, target))
    candidates = [candidate for candidate in (index - 1, index)
                  if 0 <= candidate < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda candidate: abs(float(times[candidate] - target)))
    return best if abs(float(times[best] - target)) <= max_diff else None


def rotation_error_degrees(est_start, est_end, gt_start, gt_end):
    relative_error = gt_start.T @ gt_end
    relative_error = relative_error.T @ (est_start.T @ est_end)
    cosine = np.clip((np.trace(relative_error) - 1.0) * 0.5, -1.0, 1.0)
    return float(np.degrees(np.arccos(cosine)))


def row_window_geometry(rows, start, end):
    interval = [row for row in rows if start - 1e-9 <= number(row, "timestamp") <= end + 1e-9]
    if not interval:
        return None
    result = {"frame_count": len(interval)}
    for mode in ("rot", "trans"):
        kappas = finite([number(row, f"dcreg_schur_kappa_{mode}") for row in interval])
        raw_kappas = finite([number(row, f"raw_block_kappa_{mode}") for row in interval])
        abs_lambda = []
        normalized = []
        per_used = []
        lc_values, ls_values = [], []
        nonfull, partial, total = 0, 0, 0
        mu_min, mu_median, mu_max = [], [], []
        eta_weak = []
        for row in interval:
            spectrum = finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)])
            if spectrum:
                smallest, largest = min(abs(value) for value in spectrum), max(abs(value) for value in spectrum)
                abs_lambda.append(smallest)
                normalized.append(smallest / largest if largest > 0 else 0.0)
                used = number(row, "used_residual_count")
                if used > 0 and math.isfinite(used):
                    per_used.append(smallest / used)
            lc_values.extend(finite([number(row, f"xicp_lc_{mode}_{index}") for index in range(3)]))
            ls_values.extend(finite([number(row, f"xicp_ls_{mode}_{index}") for index in range(3)]))
            for index in range(3):
                label = row.get(f"xicp_class_{mode}_{index}", "")
                if label:
                    total += 1
                    nonfull += label != "FULL"
                    partial += label == "PARTIAL"
            mus = finite([number(row, f"mu_{index}") for index in range(6)])
            if mus:
                mu_min.append(min(mus)); mu_median.append(statistics.median(mus)); mu_max.append(max(mus))
            rank = integer(row, f"d1_weak_rank_{mode}")
            eta_weak.extend(finite([number(row, f"eta_{mode}_{index}") for index in range(max(0, rank))]))
        result.update({
            f"max_log_kappa_{mode}": max([math.log(max(value, 1e-300)) for value in kappas], default=math.nan),
            f"mean_log_kappa_{mode}": statistics.mean([math.log(max(value, 1e-300)) for value in kappas]) if kappas else math.nan,
            f"max_log_raw_kappa_{mode}": max([math.log(max(value, 1e-300)) for value in raw_kappas], default=math.nan),
            f"min_abs_lambda_{mode}": min(abs_lambda, default=math.nan),
            f"median_abs_lambda_{mode}": statistics.median(abs_lambda) if abs_lambda else math.nan,
            f"min_normalized_lambda_{mode}": min(normalized, default=math.nan),
            f"min_abs_lambda_per_used_{mode}": min(per_used, default=math.nan),
            f"xicp_lc_min_{mode}": min(lc_values, default=math.nan),
            f"xicp_ls_min_{mode}": min(ls_values, default=math.nan),
            f"xicp_nonfull_fraction_{mode}": nonfull / total if total else math.nan,
            f"xicp_partial_fraction_{mode}": partial / total if total else math.nan,
            f"mu_min_{mode}": min(mu_min, default=math.nan),
            f"mu_median_{mode}": statistics.median(mu_median) if mu_median else math.nan,
            f"mu_max_{mode}": max(mu_max, default=math.nan),
            f"eta_weak_min_{mode}": min(eta_weak, default=math.nan),
            f"eta_weak_max_{mode}": max(eta_weak, default=math.nan),
        })
        result[f"weak_fraction_{mode}"] = sum(integer(row, f"d1_weak_rank_{mode}") > 0 for row in interval) / len(interval)
    used = finite([number(row, "used_residual_count") for row in interval])
    result["median_used_residual_count"] = statistics.median(used) if used else math.nan
    result["min_used_residual_count"] = min(used) if used else math.nan
    return result


def make_windows(rows, pair_time, pair_est_pos, pair_gt_pos, pair_est_rot, pair_gt_rot,
                 position_error, delta, max_diff, nonoverlap):
    starts = []
    for row in rows:
        index = nearest_index(pair_time, number(row, "timestamp"), max_diff)
        if index is not None:
            starts.append((index, row))
    starts.sort(key=lambda item: item[0])
    selected = []
    cursor = -1
    for start_index, row in starts:
        if nonoverlap and start_index <= cursor:
            continue
        end_index = nearest_index(pair_time, pair_time[start_index] + delta, max_diff)
        if end_index is None or end_index <= start_index:
            continue
        selected.append((start_index, end_index, row))
        cursor = end_index
    windows = []
    for start_index, end_index, row in selected:
        start_time, end_time = float(pair_time[start_index]), float(pair_time[end_index])
        geometry = row_window_geometry(rows, start_time, end_time)
        if geometry is None:
            continue
        displacement = position_error[end_index] - position_error[start_index]
        windows.append({
            "start_time": start_time, "end_time": end_time,
            "translation_error_m": float(np.linalg.norm(displacement)),
            "rotation_error_deg": rotation_error_degrees(
                pair_est_rot[start_index], pair_est_rot[end_index],
                pair_gt_rot[start_index], pair_gt_rot[end_index]),
            "geometry": geometry,
            "geometry_rows": [candidate for candidate in rows
                              if start_time - 1e-9 <= number(candidate, "timestamp") <= end_time + 1e-9],
            "frame": integer(row, "frame"),
        })
    return windows


def bootstrap_correlations(windows, target, metrics, delta):
    n = len(windows)
    if n < 3:
        return {"seed": BOOTSTRAP_SEED, "replicates": 0, "block_windows": max(1, int(math.ceil(10.0 / delta))),
                "metrics": {metric: {"median": None, "p2_5": None, "p97_5": None} for metric in metrics}}
    block = max(1, int(math.ceil(10.0 / delta)))
    rng = random.Random(BOOTSTRAP_SEED)
    samples = {metric: [] for metric in metrics}
    for _ in range(BOOTSTRAP_REPS):
        indices = []
        while len(indices) < n:
            start = rng.randrange(max(1, n - block + 1))
            indices.extend(range(start, min(start + block, n)))
        indices = indices[:n]
        for metric in metrics:
            x = [windows[index]["geometry"].get(metric, math.nan) for index in indices]
            y = [windows[index][target] for index in indices]
            value = spearman(x, y)["rho"]
            if value is not None:
                samples[metric].append(value)
    return {
        "seed": BOOTSTRAP_SEED, "replicates": BOOTSTRAP_REPS, "block_windows": block,
        "metrics": {metric: {"median": statistics.median(values) if values else None,
                              "p2_5": percentile(values, 0.025), "p97_5": percentile(values, 0.975),
                              "defined_replicates": len(values)} for metric, values in samples.items()}
    }


def binary_metrics(predicted, high):
    tp = sum(bool(p) and bool(h) for p, h in zip(predicted, high))
    fp = sum(bool(p) and not bool(h) for p, h in zip(predicted, high))
    tn = sum(not bool(p) and not bool(h) for p, h in zip(predicted, high))
    fn = sum(not bool(p) and bool(h) for p, h in zip(predicted, high))
    precision = tp / (tp + fp) if tp + fp else None
    recall = tp / (tp + fn) if tp + fn else None
    specificity = tn / (tn + fp) if tn + fp else None
    balanced = ((recall + specificity) / 2.0 if recall is not None and specificity is not None else None)
    f1 = (2 * precision * recall / (precision + recall)
          if precision is not None and recall is not None and precision + recall else None)
    return {"tp": tp, "fp": fp, "tn": tn, "fn": fn, "precision": precision,
            "recall": recall, "balanced_accuracy": balanced, "f1": f1,
            "high_error_coverage": recall}


def event_report(windows, target, predicted):
    if not windows:
        return {"n": 0, "top10": binary_metrics([], []), "bottom50_activation": None}
    errors = [window[target] for window in windows]
    finite_indices = [index for index, value in enumerate(errors) if math.isfinite(value)]
    top_count = max(1, int(math.ceil(len(finite_indices) * 0.10)))
    low_count = max(1, int(math.ceil(len(finite_indices) * 0.50)))
    top = set(sorted(finite_indices, key=lambda index: errors[index], reverse=True)[:top_count])
    low = set(sorted(finite_indices, key=lambda index: errors[index])[:low_count])
    top_report = binary_metrics([index in predicted for index in finite_indices],
                                [index in top for index in finite_indices])
    low_activation = len(predicted & low) / len(low) if low else None
    top_report.update({"n": len(finite_indices), "predicted": len(predicted),
                       "top10_count": len(top), "bottom50_count": len(low),
                       "bottom50_activation": low_activation})
    return top_report


def frame_event_persistence(rows, pair_time, position_error, pair_est_rot, pair_gt_rot, max_diff):
    frame_rows = []
    for index, row in enumerate(rows):
        start = nearest_index(pair_time, number(row, "timestamp"), max_diff)
        if start is None:
            continue
        end = nearest_index(pair_time, pair_time[start] + 1.0, max_diff)
        if end is None or end <= start:
            continue
        frame_rows.append({
            "time": number(row, "timestamp"),
            "translation_error_m": float(np.linalg.norm(position_error[end] - position_error[start])),
            "rotation_error_deg": rotation_error_degrees(pair_est_rot[start], pair_est_rot[end],
                                                           pair_gt_rot[start], pair_gt_rot[end]),
            "row": row,
        })
    frame_rows.sort(key=lambda item: item["time"])
    result = {}
    for mode, target in (("rot", "rotation_error_deg"), ("trans", "translation_error_m")):
        definitions = {
            "kappa_gt10": [number(item["row"], f"dcreg_schur_kappa_{mode}") > 10 for item in frame_rows],
            "xicp_nonfull": [any(item["row"].get(f"xicp_class_{mode}_{i}") != "FULL" for i in range(3))
                             for item in frame_rows],
            "mu_min_gt1": [number(item["row"], "mu_min") > 1 for item in frame_rows],
            "eta_weak_gt1": [any(number(item["row"], f"eta_{mode}_{i}") > 1
                                  for i in range(max(0, integer(item["row"], f"d1_weak_rank_{mode}"))))
                             for item in frame_rows],
        }
        definitions["C4_kappa_xicp_eta"] = [definitions["kappa_gt10"][i] and
                                               definitions["xicp_nonfull"][i] and
                                               definitions["eta_weak_gt1"][i]
                                               for i in range(len(frame_rows))]
        mode_result = {}
        for name, active in definitions.items():
            times = np.asarray([item["time"] for item in frame_rows])
            report = {"active_frames": sum(active), "run_lengths": run_summary(active, times)}
            for persistence in (2, 3, 5):
                persistent = [False] * len(active)
                cursor = 0
                while cursor < len(active):
                    end = cursor
                    while end < len(active) and active[end]:
                        end += 1
                    if end - cursor >= persistence:
                        for position in range(cursor, end): persistent[position] = True
                    cursor = end + 1 if end == cursor else end
                errors = [item[target] for item in frame_rows]
                finite_indices = [i for i, value in enumerate(errors) if math.isfinite(value)]
                high_n = max(1, int(math.ceil(len(finite_indices) * 0.10))) if finite_indices else 0
                high = set(sorted(finite_indices, key=lambda i: errors[i], reverse=True)[:high_n])
                low_n = max(1, int(math.ceil(len(finite_indices) * 0.50))) if finite_indices else 0
                low = set(sorted(finite_indices, key=lambda i: errors[i])[:low_n])
                active_indices = {i for i, value in enumerate(persistent) if value}
                mode_result[f"persist_{persistence}"] = {
                    "frames": len(active_indices), "false_positives": len(active_indices - high),
                    "high_error_coverage": len(active_indices & high) / len(high) if high else None,
                    "bottom50_activation": len(active_indices & low) / len(low) if low else None,
                }
            mode_result[name] = report
        result[mode] = mode_result
    return {"frame_count": len(frame_rows), "signals": result}


def empirical_roc(samples, target):
    samples = [(float(score), int(label)) for score, label in samples
               if math.isfinite(float(score))]
    positives = sum(label for _, label in samples)
    negatives = len(samples) - positives
    if not samples or not positives or not negatives:
        return {"defined": False, "n": len(samples), "positives": positives, "negatives": negatives}
    ordered = sorted(samples, key=lambda item: item[0], reverse=True)
    tp = fp = 0
    area = 0.0
    previous_fpr = previous_tpr = 0.0
    precision_recall = []
    for score, label in ordered:
        if label: tp += 1
        else: fp += 1
        tpr, fpr = tp / positives, fp / negatives
        area += (fpr - previous_fpr) * (tpr + previous_tpr) * 0.5
        previous_fpr, previous_tpr = fpr, tpr
        precision_recall.append((tpr, tp / (tp + fp)))
    auprc = 0.0
    previous_recall, previous_precision = 0.0, 1.0
    for recall, precision in precision_recall:
        auprc += max(0.0, recall - previous_recall) * precision
        previous_recall, previous_precision = recall, precision
    return {"defined": True, "n": len(samples), "positives": positives, "negatives": negatives,
            "auroc": area, "auprc": auprc}


def empirical_thresholds(rows, mode, raw=False):
    suffix = "raw_block_kappa" if raw else "dcreg_schur_kappa"
    result = {}
    for target_name, target_label in (("nonfull", lambda row, i: row.get(f"xicp_class_{mode}_{i}") != "FULL"),
                                      ("none", lambda row, i: row.get(f"xicp_class_{mode}_{i}") == "NONE")):
        report = {}
        for source_mode in (mode,):
            samples = []
            for row in rows:
                score = number(row, f"{suffix}_{source_mode}")
                for index in range(3):
                    samples.append((score, int(target_label(row, index))))
            curve = empirical_roc(samples, 1)
            values = sorted(set(score for score, _ in samples if math.isfinite(score)))
            candidates = values[::max(1, len(values) // 200)] if values else []
            best = None
            for threshold in candidates:
                predicted = [score > threshold for score, _ in samples]
                labels = [bool(label) for _, label in samples]
                metrics = binary_metrics(predicted, labels)
                value = metrics["balanced_accuracy"]
                if value is not None and (best is None or value > best["balanced_accuracy"]):
                    best = {"threshold": threshold, **metrics}
            report[target_name] = {"best_descriptive_threshold": best, **curve,
                                   "label": "EMPIRICAL_XICP_EQUIVALENT_KAPPA; not analytical or transferable"}
        result["schur" if not raw else "raw"] = report
    return result


def analyze_sequence(name, estimate_path, ground_truth_path, d2_path, d1_raw_path,
                     max_diff=ASSOCIATION_MAX_DIFF):
    est_time, est_pos, est_quat = load_tum(estimate_path)
    gt_time, gt_pos, gt_quat = load_tum(ground_truth_path, allow_unsorted=True)
    pairs = associate(est_time, gt_time, max_diff)
    if len(pairs) < 3:
        raise ValueError(f"{name}: insufficient trajectory matches: {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    pair_time = est_time[ei]
    pair_est_pos = est_pos[ei]
    pair_gt_pos = gt_pos[gi]
    pair_est_rot = np.asarray([rotation_matrix(est_quat[index]) for index in ei])
    pair_gt_rot = np.asarray([rotation_matrix(gt_quat[index]) for index in gi])
    alignment_rotation, alignment_translation = umeyama_se3(pair_est_pos, pair_gt_pos)
    aligned = (alignment_rotation @ pair_est_pos.T).T + alignment_translation
    position_error = aligned - pair_gt_pos
    rows = load_d2(d2_path)
    raw_rows = load_rows(d1_raw_path, 2)
    output = {
        "sequence": name,
        "estimate": str(pathlib.Path(estimate_path).resolve()),
        "ground_truth": str(pathlib.Path(ground_truth_path).resolve()),
        "d2_summary": str(pathlib.Path(d2_path).resolve()),
        "d1_raw": str(pathlib.Path(d1_raw_path).resolve()),
        "association": {"max_diff_s": max_diff, "matched": len(pairs),
                         "global_alignment": "single SE3, no per-window realignment"},
        "frame_authority": {"rows": len(rows), "all_iteration_zero": True,
                             "valid_rows": sum(integer(row, "d1_valid") == 1 for row in rows),
                             "invalid_rows": sum(integer(row, "d1_valid") != 1 for row in rows)},
        "prompt02_corrective": corrected_prompt02(raw_rows),
        "gamma_frame_level": gamma_report(rows, "dcreg_schur_kappa"),
        "raw_inner_iteration_counterfactual": gamma_report(
            [{**row, "dcreg_schur_kappa_rot": row.get("cond_rot", "nan"),
              "dcreg_schur_kappa_trans": row.get("cond_trans", "nan")} for row in raw_rows],
            "dcreg_schur_kappa"),
        "x_icp": {}, "prior_relative": {}, "windows": {}, "persistence": {},
        "empirical_xicp_equivalent_kappa": {
            "rot": empirical_thresholds(rows, "rot"),
            "trans": empirical_thresholds(rows, "trans"),
        },
    }
    for mode in ("rot", "trans"):
        classes = [row.get(f"xicp_class_{mode}_{index}") for row in rows for index in range(3)]
        lc = [number(row, f"xicp_lc_{mode}_{index}") for row in rows for index in range(3)]
        ls = [number(row, f"xicp_ls_{mode}_{index}") for row in rows for index in range(3)]
        output["x_icp"][mode] = {
            "class_fractions": {label: classes.count(label) / len(classes) if classes else None
                                 for label in ("FULL", "PARTIAL", "NONE")},
            "Lc_summary": summary_stats(lc), "Ls_summary": summary_stats(ls),
            "raw_block_kappa": summary_stats([number(row, f"raw_block_kappa_{mode}") for row in rows]),
            "schur_kappa": summary_stats([number(row, f"dcreg_schur_kappa_{mode}") for row in rows]),
            "block_equivalence": summary_stats([number(row, f"xicp_block_equivalence_{mode}") for row in rows]),
        }
        output["prior_relative"][mode] = {
            "mu_min": summary_stats([number(row, "mu_min") for row in rows]),
            "mu_median": summary_stats([statistics.median(finite([number(row, f"mu_{index}") for index in range(6)]))
                                         if finite([number(row, f"mu_{index}") for index in range(6)]) else math.nan
                                         for row in rows]),
            "mu_max": summary_stats([number(row, "mu_max") for row in rows]),
            "eta_weak_min": summary_stats([number(row, f"eta_weak_min_{mode}") for row in rows]),
            "eta_weak_max": summary_stats([number(row, f"eta_weak_max_{mode}") for row in rows]),
            "valid_fraction": sum(integer(row, "prior_relative_valid") == 1 for row in rows) / len(rows) if rows else None,
        }
    for delta in DELTAS:
        key = str(delta)
        overlap = make_windows(rows, pair_time, pair_est_pos, pair_gt_pos, pair_est_rot,
                               pair_gt_rot, position_error, delta, max_diff, False)
        nonoverlap = make_windows(rows, pair_time, pair_est_pos, pair_gt_pos, pair_est_rot,
                                  pair_gt_rot, position_error, delta, max_diff, True)
        delta_report = {}
        for label, windows in (("overlapping", overlap), ("non_overlapping", nonoverlap)):
            correlations = {}
            bootstrap = {}
            for mode, target in (("rot", "rotation_error_deg"), ("trans", "translation_error_m")):
                metrics = [f"max_log_kappa_{mode}", f"max_log_raw_kappa_{mode}",
                           f"min_abs_lambda_{mode}", f"min_abs_lambda_per_used_{mode}",
                           f"xicp_lc_min_{mode}", f"xicp_ls_min_{mode}",
                           f"mu_min_{mode}", f"mu_median_{mode}", f"eta_weak_min_{mode}",
                           f"eta_weak_max_{mode}"]
                correlations[target] = {metric: spearman(
                    [window["geometry"].get(metric, math.nan) for window in windows],
                    [window[target] for window in windows]) for metric in metrics}
                if label == "non_overlapping":
                    bootstrap[target] = bootstrap_correlations(windows, target, metrics, delta)
            delta_report[label] = {"count": len(windows), "correlations": correlations,
                                   "bootstrap": bootstrap}
        # Combined descriptive event families are reporting-only. Lambda cutoffs
        # are within-sequence percentiles and never become production settings.
        for mode, target in (("rot", "rotation_error_deg"), ("trans", "translation_error_m")):
            lambda_values = [window["geometry"].get(f"min_abs_lambda_{mode}", math.nan) for window in overlap]
            low10, low25 = percentile(lambda_values, 0.10), percentile(lambda_values, 0.25)
            event_families = {}
            for lambda_name, cutoff in (("bottom10", low10), ("bottom25", low25)):
                for eta_ref in (1.0,):
                    for family in ("C0", "C1", "C2", "C3", "C4"):
                        predicted = set()
                        for index, window in enumerate(overlap):
                            geometry = window["geometry"]
                            kappa = geometry.get(f"max_log_kappa_{mode}", math.nan) > math.log(10.0)
                            low = cutoff is not None and geometry.get(f"min_abs_lambda_{mode}", math.nan) <= cutoff
                            eta = geometry.get(f"eta_weak_max_{mode}", math.nan) > eta_ref
                            xicp = geometry.get(f"xicp_nonfull_fraction_{mode}", math.nan) > 0.0
                            active = {"C0": kappa, "C1": kappa and low, "C2": kappa and eta,
                                      "C3": kappa and xicp, "C4": kappa and xicp and eta}[family]
                            if active: predicted.add(index)
                        event_families[f"{family}_{lambda_name}_eta{eta_ref:g}"] = {
                            "lambda_cutoff": cutoff, "eta_ref": eta_ref,
                            **event_report(overlap, target, predicted)}
            delta_report.setdefault("combined_events", {})[mode] = event_families
            prior_sweeps = {}
            for threshold in PRIOR_THRESHOLDS:
                for signal in ("mu_min", "eta_weak_min"):
                    predicted = {index for index, window in enumerate(overlap)
                                 if window["geometry"].get(f"{signal}_{mode}", math.nan) > threshold}
                    prior_sweeps[f"{signal}_gt{threshold:g}"] = event_report(overlap, target, predicted)
            delta_report.setdefault("prior_relative_thresholds", {})[mode] = prior_sweeps
        delta_report["error_stats"] = {
            target: summary_stats([window[target] for window in overlap])
            for target in ("translation_error_m", "rotation_error_deg")}
        delta_report["window_samples"] = [
            {"kappa_rot": math.exp(window["geometry"].get("max_log_kappa_rot", math.nan)),
             "kappa_trans": math.exp(window["geometry"].get("max_log_kappa_trans", math.nan)),
             "translation_error_m": window["translation_error_m"],
             "rotation_error_deg": window["rotation_error_deg"],
             "xicp_nonfull_fraction_trans": window["geometry"].get("xicp_nonfull_fraction_trans", math.nan),
             "mu_min": window["geometry"].get("mu_min_trans", math.nan),
             "eta_weak_min_trans": window["geometry"].get("eta_weak_min_trans", math.nan)}
            for window in overlap]
        output["windows"][key] = delta_report
    output["persistence"] = frame_event_persistence(rows, pair_time, position_error,
                                                      pair_est_rot, pair_gt_rot, max_diff)
    return output


def rotation_matrix(quaternion):
    x, y, z, w = np.asarray(quaternion, dtype=float)
    norm = np.linalg.norm([x, y, z, w])
    if not math.isfinite(norm) or norm <= 0.0:
        raise ValueError("invalid quaternion")
    x, y, z, w = np.asarray([x, y, z, w]) / norm
    return np.asarray([[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
                       [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
                       [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]])


def matched_kappa_report(reports, delta="5.0"):
    result = {"delta": float(delta), "bins": {}}
    for lower, upper, label in ((5.0, 10.0, "5-10"), (10.0, 20.0, "10-20"), (20.0, math.inf, ">20")):
        bin_report = {}
        for name, report in reports.items():
            samples = report["windows"].get(delta, {}).get("window_samples", [])
            selected = [sample for sample in samples
                        if math.isfinite(sample.get("kappa_trans", math.nan)) and
                        lower < sample["kappa_trans"] <= upper]
            bin_report[name] = {
                "window_count": len(selected),
                "translation_error_m": summary_stats([sample["translation_error_m"] for sample in selected]),
                "rotation_error_deg": summary_stats([sample["rotation_error_deg"] for sample in selected]),
                "xicp_nonfull_fraction_trans": summary_stats(
                    [sample["xicp_nonfull_fraction_trans"] for sample in selected]),
                "mu_min": summary_stats([sample["mu_min"] for sample in selected]),
                "eta_weak_min_trans": summary_stats([sample["eta_weak_min_trans"] for sample in selected]),
            }
        result["bins"][label] = {"lower_exclusive": lower, "upper_inclusive": upper,
                                  "sequences": bin_report}
    return result


def clean_json(value):
    if isinstance(value, float) and not math.isfinite(value):
        return None
    if isinstance(value, dict):
        return {key: clean_json(entry) for key, entry in value.items()}
    if isinstance(value, list):
        return [clean_json(entry) for entry in value]
    return value


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", required=True)
    parser.add_argument("--estimate", type=pathlib.Path, required=True)
    parser.add_argument("--ground-truth", type=pathlib.Path, required=True)
    parser.add_argument("--d2", type=pathlib.Path, required=True)
    parser.add_argument("--d1-raw", type=pathlib.Path, required=True)
    parser.add_argument("--max-diff", type=float, default=ASSOCIATION_MAX_DIFF)
    parser.add_argument("--out", type=pathlib.Path)
    parser.add_argument("--peer-report", type=pathlib.Path)
    args = parser.parse_args(argv)
    try:
        report = analyze_sequence(args.name, args.estimate, args.ground_truth,
                                  args.d2, args.d1_raw, args.max_diff)
        if args.peer_report:
            peer = json.loads(args.peer_report.read_text(encoding="utf-8"))
            report["matched_kappa_peer"] = matched_kappa_report(
                {args.name: report, peer["sequence"]: peer})
        text = json.dumps(clean_json(report), indent=2, sort_keys=True) + "\n"
        if args.out:
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_text(text, encoding="utf-8")
        else:
            sys.stdout.write(text)
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"PROMPT03_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
