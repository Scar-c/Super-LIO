#!/usr/bin/env python3
"""Prompt02 local trajectory-error and shadow-gate analysis.

The trajectory is aligned once with a global SE(3) Umeyama transform. Every
local metric is then computed from the globally aligned trajectory; no local
window is re-aligned. D1 projectors are transformed by that same alignment
rotation before directional drift is measured.
"""

import argparse
import csv
import json
import math
import pathlib
import statistics
import sys

import numpy as np

# Keep both direct script execution and unittest/module execution reproducible.
REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3


DELTAS = (1.0, 5.0, 10.0)
REFERENCES = (3.0, 5.0, 10.0, 20.0)
ASSOCIATION_MAX_DIFF = 0.1


def number(row, name):
    try:
        return float(row.get(name, ""))
    except (TypeError, ValueError):
        return math.nan


def finite(values):
    return [value for value in values if math.isfinite(value)]


def percentile(values, fraction):
    values = sorted(finite(values))
    if not values:
        return None
    return float(np.percentile(np.asarray(values), fraction * 100.0))


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


def load_schema_v2(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames is None or "schema_version" not in reader.fieldnames:
            raise ValueError(f"{path}: missing schema_version")
        rows = list(reader)
    versions = {int(float(row["schema_version"])) for row in rows if row.get("frame")}
    if versions != {2}:
        raise ValueError(f"{path}: expected schema_version=2, got {sorted(versions)}")
    rows = [row for row in rows if row.get("frame") and row.get("valid") == "1"]
    rows.sort(key=lambda row: number(row, "timestamp"))
    return rows


def quat_to_matrix(quaternion):
    x, y, z, w = np.asarray(quaternion, dtype=float)
    norm = np.linalg.norm([x, y, z, w])
    if not math.isfinite(norm) or norm <= 0.0:
        raise ValueError("invalid quaternion")
    x, y, z, w = np.asarray([x, y, z, w]) / norm
    return np.asarray([
        [1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - z * w),
         2.0 * (x * z + y * w)],
        [2.0 * (x * y + z * w), 1.0 - 2.0 * (x * x + z * z),
         2.0 * (y * z - x * w)],
        [2.0 * (x * z - y * w), 2.0 * (y * z + x * w),
         1.0 - 2.0 * (x * x + y * y)],
    ])


def projector(row, mode):
    values = [number(row, f"P_weak_{mode}_{index}") for index in range(9)]
    return np.asarray(values, dtype=float).reshape((3, 3))


def projector_is_valid(value, rank):
    if rank <= 0 or not np.isfinite(value).all():
        return False
    return (np.linalg.norm(value - value.T) <= 1e-6 and
            np.linalg.norm(value @ value - value) <= 1e-5 and
            abs(float(np.trace(value)) - rank) <= 1e-5)


def nearest_index(times, target, max_diff=ASSOCIATION_MAX_DIFF):
    if len(times) == 0:
        return None
    index = int(np.searchsorted(times, target))
    candidates = [candidate for candidate in (index - 1, index)
                  if 0 <= candidate < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda candidate: abs(float(times[candidate] - target)))
    if abs(float(times[best] - target)) > max_diff:
        return None
    return best


def _rotation_error_degrees(est_start, est_end, gt_start, gt_end):
    est_relative = est_start.T @ est_end
    gt_relative = gt_start.T @ gt_end
    relative_error = gt_relative.T @ est_relative
    cosine = np.clip((np.trace(relative_error) - 1.0) * 0.5, -1.0, 1.0)
    return float(np.degrees(np.arccos(cosine)))


def _geometry(rows, start, end):
    interval = [row for row in rows if start - 1e-9 <= number(row, "timestamp") <= end + 1e-9]
    if not interval:
        return None
    result = {"frame_count": len(interval)}
    for mode in ("rot", "trans"):
        kappas = finite([number(row, f"cond_{mode}") for row in interval])
        logs = [math.log(max(value, 1e-300)) for value in kappas]
        lambda_min = []
        normalized_min = []
        lambda_per_used = []
        for row in interval:
            spectrum = [number(row, f"lambda_{mode}_{index}") for index in range(3)]
            spectrum = finite(spectrum)
            if not spectrum:
                continue
            absolute_min = min(abs(value) for value in spectrum)
            scale = max(abs(value) for value in spectrum)
            lambda_min.append(absolute_min)
            normalized_min.append(absolute_min / scale if scale > 0.0 else 0.0)
            used = number(row, "used_residual_count")
            if math.isfinite(used) and used > 0.0:
                lambda_per_used.append(absolute_min / used)
        weak_fraction = sum(number(row, f"weak_rank_{mode}") > 0 for row in interval) / len(interval)
        used_values = finite([number(row, "used_residual_count") for row in interval])
        result.update({
            f"max_log_kappa_{mode}": max(logs) if logs else math.nan,
            f"mean_log_kappa_{mode}": statistics.mean(logs) if logs else math.nan,
            f"min_abs_lambda_{mode}": min(lambda_min) if lambda_min else math.nan,
            f"median_abs_lambda_{mode}": statistics.median(lambda_min) if lambda_min else math.nan,
            f"min_normalized_lambda_{mode}": min(normalized_min) if normalized_min else math.nan,
            f"min_abs_lambda_per_used_{mode}": min(lambda_per_used) if lambda_per_used else math.nan,
            f"weak_fraction_{mode}": weak_fraction,
        })
    result["median_used_residual_count"] = statistics.median(used_values) if used_values else math.nan
    result["min_used_residual_count"] = min(used_values) if used_values else math.nan
    return result


def _window_event_metrics(windows, error_name, mode, threshold):
    errors = [window[error_name] for window in windows]
    if not errors:
        return {"n": 0, "predicted": 0, "high_error": 0, "intersection": 0,
                "precision": None, "recall": None, "event_overlap": None,
                "false_positive_fraction": None}
    top_count = max(1, int(math.ceil(len(errors) * 0.10)))
    high_indices = set(sorted(range(len(errors)), key=lambda index: errors[index], reverse=True)[:top_count])
    predicted = {
        index for index, window in enumerate(windows)
        if window["geometry"] and window["geometry"][f"max_log_kappa_{mode}"] > math.log(threshold)
    }
    intersection = predicted & high_indices
    union = predicted | high_indices
    false_positive = predicted - high_indices
    return {
        "n": len(errors),
        "predicted": len(predicted),
        "high_error": len(high_indices),
        "intersection": len(intersection),
        "precision": len(intersection) / len(predicted) if predicted else 0.0,
        "recall": len(intersection) / len(high_indices),
        "event_overlap": len(intersection) / len(union) if union else 0.0,
        "false_positive_fraction": len(false_positive) / len(windows),
    }


def _coverage(windows, error_name, mode, reference, fraction, reverse=True):
    if not windows:
        return {"windows": 0, "attenuated_windows": 0, "coverage": None}
    ordered = sorted(range(len(windows)), key=lambda index: windows[index][error_name], reverse=reverse)
    count = max(1, int(math.ceil(len(windows) * fraction)))
    selected = [windows[index] for index in ordered[:count]]
    active = 0
    for window in selected:
        rows = window["geometry_rows"]
        if any(number(row, f"cond_{mode}") > reference for row in rows):
            active += 1
    return {"windows": len(selected), "attenuated_windows": active,
            "coverage": active / len(selected)}


def analyze_sequence(estimate_path, ground_truth_path, summary_path,
                     max_diff=ASSOCIATION_MAX_DIFF, deltas=DELTAS):
    est_time, est_pos, est_quat = load_tum(estimate_path)
    gt_time, gt_pos, gt_quat = load_tum(ground_truth_path, allow_unsorted=True)
    pairs = associate(est_time, gt_time, max_diff)
    if len(pairs) < 3:
        raise ValueError(f"insufficient associated trajectory pairs: {len(pairs)}")
    est_indices = np.asarray([pair[0] for pair in pairs], dtype=int)
    gt_indices = np.asarray([pair[1] for pair in pairs], dtype=int)
    pair_time = est_time[est_indices]
    pair_est_pos = est_pos[est_indices]
    pair_gt_pos = gt_pos[gt_indices]
    pair_est_rot = np.asarray([quat_to_matrix(est_quat[index]) for index in est_indices])
    pair_gt_rot = np.asarray([quat_to_matrix(gt_quat[index]) for index in gt_indices])
    align_rotation, align_translation = umeyama_se3(pair_est_pos, pair_gt_pos)
    aligned_est_pos = (align_rotation @ pair_est_pos.T).T + align_translation
    position_error = aligned_est_pos - pair_gt_pos
    d1_rows = load_schema_v2(summary_path)
    d1_time = np.asarray([number(row, "timestamp") for row in d1_rows])
    windows_by_delta = {}
    for delta in deltas:
        windows = []
        for d1_index, row in enumerate(d1_rows):
            start_index = nearest_index(pair_time, d1_time[d1_index], max_diff)
            if start_index is None:
                continue
            end_index = nearest_index(pair_time, pair_time[start_index] + delta, max_diff)
            if end_index is None or end_index <= start_index:
                continue
            geometry = _geometry(d1_rows, float(pair_time[start_index]),
                                 float(pair_time[end_index]))
            if geometry is None:
                continue
            displacement_error = position_error[end_index] - position_error[start_index]
            rank = int(number(row, "weak_rank_trans"))
            weak_value = projector(row, "trans")
            weak_valid = projector_is_valid(weak_value, rank)
            if weak_valid:
                weak_gt = align_rotation @ weak_value @ align_rotation.T
                weak_drift = float(np.linalg.norm(weak_gt @ displacement_error))
                strong_drift = float(np.linalg.norm((np.eye(3) - weak_gt) @ displacement_error))
            else:
                weak_drift = math.nan
                strong_drift = math.nan
            windows.append({
                "start_time": float(pair_time[start_index]),
                "end_time": float(pair_time[end_index]),
                "translation_error_m": float(np.linalg.norm(displacement_error)),
                "rotation_error_deg": _rotation_error_degrees(
                    pair_est_rot[start_index], pair_est_rot[end_index],
                    pair_gt_rot[start_index], pair_gt_rot[end_index]),
                "weak_projected_drift_m": weak_drift,
                "strong_projected_drift_m": strong_drift,
                "geometry": geometry,
                "geometry_rows": [candidate for candidate in d1_rows
                                   if float(pair_time[start_index]) - 1e-9 <= number(candidate, "timestamp") <=
                                   float(pair_time[end_index]) + 1e-9],
            })
        windows_by_delta[str(delta)] = windows

    result = {
        "estimate": str(pathlib.Path(estimate_path).resolve()),
        "ground_truth": str(pathlib.Path(ground_truth_path).resolve()),
        "d1_summary": str(pathlib.Path(summary_path).resolve()),
        "association": {"max_diff_s": max_diff, "matched": len(pairs),
                         "overlap_s": float(min(est_time[-1], gt_time[-1]) -
                                              max(est_time[0], gt_time[0]))},
        "global_alignment": {"rotation": align_rotation.tolist(),
                              "translation": align_translation.tolist()},
        "windows": {},
    }
    geometry_names = {
        "rot": ("max_log_kappa_rot", "mean_log_kappa_rot",
                "min_abs_lambda_rot", "min_normalized_lambda_rot",
                "min_abs_lambda_per_used_rot", "weak_fraction_rot"),
        "trans": ("max_log_kappa_trans", "mean_log_kappa_trans",
                  "min_abs_lambda_trans", "min_normalized_lambda_trans",
                  "min_abs_lambda_per_used_trans", "weak_fraction_trans"),
    }
    for delta, windows in windows_by_delta.items():
        delta_report = {"count": len(windows), "error_stats": {},
                        "correlations": {}, "high_error_overlap": {},
                        "gate_vs_error_coverage": {}}
        for error_name in ("translation_error_m", "rotation_error_deg",
                           "weak_projected_drift_m", "strong_projected_drift_m"):
            values = [window[error_name] for window in windows]
            delta_report["error_stats"][error_name] = {
                "count": len(finite(values)), "median": statistics.median(finite(values))
                if finite(values) else None, "p90": percentile(values, 0.90),
                "max": max(finite(values)) if finite(values) else None,
            }
        for mode, names in geometry_names.items():
            target = "translation_error_m" if mode == "trans" else "rotation_error_deg"
            delta_report["correlations"][target] = {
                name: spearman([window["geometry"].get(name, math.nan) for window in windows],
                               [window[target] for window in windows]) for name in names
            }
            for threshold in (3, 5, 10, 20):
                delta_report["high_error_overlap"].setdefault(mode, {})[str(threshold)] = \
                    _window_event_metrics(windows, target, mode, threshold)
            for reference in REFERENCES:
                delta_report["gate_vs_error_coverage"].setdefault(mode, {})[str(int(reference))] = {
                    "top10": _coverage(windows, target, mode, reference, 0.10),
                    "bottom50": _coverage(
                        windows, target, mode, reference, 0.50, reverse=False),
                }
        result["windows"][delta] = delta_report
    return result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("estimate", type=pathlib.Path)
    parser.add_argument("ground_truth", type=pathlib.Path)
    parser.add_argument("d1_summary", type=pathlib.Path)
    parser.add_argument("--max-diff", type=float, default=ASSOCIATION_MAX_DIFF)
    parser.add_argument("--out", type=pathlib.Path)
    args = parser.parse_args(argv)
    try:
        report = json.dumps(analyze_sequence(args.estimate, args.ground_truth,
                                             args.d1_summary, args.max_diff),
                            indent=2, sort_keys=True) + "\n"
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"LOCAL_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2
    if args.out:
        args.out.write_text(report, encoding="utf-8")
    else:
        sys.stdout.write(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
