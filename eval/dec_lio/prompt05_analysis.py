#!/usr/bin/env python3
"""Prompt05 consistency/innovation shadow analysis.

This is an evaluation-only consumer of schema-4 consistency CSVs.  It keeps
the Prompt03 single global SE(3) alignment and deterministic local-window
definitions, then adds candidate consistency variables, correlations, and a
fixed pre-declared Tunnel2 onset comparison.  It never feeds a result back to
the estimator.
"""

import argparse
import csv
import json
import math
import pathlib
import random
import statistics
import sys

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3

DELTAS = (1.0, 5.0, 10.0)
MAX_DIFF = 0.1
BOOTSTRAP_SEED = 20260913
BOOTSTRAP_REPS = 1000
TUNNEL2_ONSET = (1706584541.828, 1706584579.030)


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


def summary(values):
    values = finite(values)
    return {
        "n": len(values),
        "median": statistics.median(values) if values else None,
        "p95": percentile(values, 0.95),
        "max": max(values) if values else None,
    }


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


def nearest_index(times, target):
    if len(times) == 0:
        return None
    index = int(np.searchsorted(times, target))
    candidates = [candidate for candidate in (index - 1, index)
                  if 0 <= candidate < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda candidate: abs(float(times[candidate] - target)))
    return best if abs(float(times[best] - target)) <= MAX_DIFF else None


def rotation_matrix(quaternion):
    x, y, z, w = np.asarray(quaternion, dtype=float)
    norm = np.linalg.norm([x, y, z, w])
    if not math.isfinite(norm) or norm <= 0.0:
        raise ValueError("invalid quaternion")
    x, y, z, w = np.asarray([x, y, z, w]) / norm
    return np.asarray([
        [1 - 2 * (y * y + z * z), 2 * (x * y - z * w),
         2 * (x * z + y * w)],
        [2 * (x * y + z * w), 1 - 2 * (x * x + z * z),
         2 * (y * z - x * w)],
        [2 * (x * z - y * w), 2 * (y * z + x * w),
         1 - 2 * (x * x + y * y)],
    ])


def valid_quaternion(quaternion):
    norm = np.linalg.norm(np.asarray(quaternion, dtype=float))
    return bool(math.isfinite(norm) and norm > 1.0e-12)


def rotation_error_degrees(est_start, est_end, gt_start, gt_end):
    relative_error = (gt_start.T @ gt_end).T @ (est_start.T @ est_end)
    cosine = np.clip((np.trace(relative_error) - 1.0) * 0.5, -1.0, 1.0)
    return float(np.degrees(np.arccos(cosine)))


def load_consistency(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or "schema_version" not in reader.fieldnames:
            raise ValueError(f"{path}: missing schema_version")
        rows = [row for row in reader if row.get("frame", "") != ""]
    versions = {integer(row, "schema_version") for row in rows}
    if versions != {4}:
        raise ValueError(f"{path}: expected schema_version=4, got {sorted(versions)}")
    frames = [integer(row, "frame") for row in rows]
    if len(frames) != len(set(frames)):
        raise ValueError(f"{path}: duplicate frame rows")
    rows.sort(key=lambda row: number(row, "timestamp"))
    return rows


def metric_names():
    names = [
        "C_L", "C_F", "delta_C", "S_prior", "G", "G_per_used",
        "weak_a_max_R", "weak_a_median_R", "weak_a_max_t", "weak_a_median_t",
        "weak_chi_max_R", "weak_chi_median_R", "weak_chi_max_t", "weak_chi_median_t",
        "weak_psi_max_R", "weak_psi_median_R", "weak_psi_max_t", "weak_psi_median_t",
        "weak_dxL_R", "weak_dxF_R", "strong_dxL_R", "strong_dxF_R",
        "weak_dxL_t", "weak_dxF_t", "strong_dxL_t", "strong_dxF_t",
        "weak_ratio_dxL_R", "weak_ratio_dxF_R", "weak_ratio_dxL_t", "weak_ratio_dxF_t",
    ]
    for mode in ("R", "t"):
        names.extend([
            f"dcreg_schur_kappa_{mode}",
            f"lambda_{mode}_0", f"lambda_{mode}_1", f"lambda_{mode}_2",
            f"weak_rank_{mode}",
        ])
    return names


def interval_metrics(rows, start, end):
    interval = [row for row in rows
                if start - 1.0e-9 <= number(row, "timestamp") <= end + 1.0e-9]
    if not interval:
        return None
    output = {"frame_count": len(interval)}
    for name in metric_names():
        values = [number(row, name) for row in interval]
        if name.startswith("weak_rank_"):
            output[name] = max([int(value) for value in values if math.isfinite(value)] or [0])
        elif name.startswith("dcreg_schur_kappa_"):
            output[f"max_{name}"] = max(finite(values), default=math.nan)
            output[f"median_{name}"] = statistics.median(finite(values)) if finite(values) else math.nan
        elif name.startswith("lambda_"):
            output[f"min_{name}"] = min([abs(value) for value in finite(values)] or [math.nan])
        elif name in ("weak_a_max_R", "weak_a_max_t", "weak_chi_max_R",
                      "weak_chi_max_t", "weak_psi_max_R", "weak_psi_max_t"):
            output[f"max_{name}"] = max(finite(values), default=math.nan)
        else:
            valid = finite(values)
            output[f"median_{name}"] = statistics.median(valid) if valid else math.nan
            output[f"max_{name}"] = max(valid) if valid else math.nan
    for mode in ("R", "t"):
        spectra = []
        per_used = []
        normalized = []
        for row in interval:
            spectrum = finite([number(row, f"lambda_{mode}_{index}") for index in range(3)])
            if not spectrum:
                continue
            minimum = min(abs(value) for value in spectrum)
            maximum = max(abs(value) for value in spectrum)
            spectra.append(minimum)
            if maximum > 0.0:
                normalized.append(minimum / maximum)
            used = number(row, "N_used")
            if math.isfinite(used) and used > 0.0:
                per_used.append(minimum / used)
        output[f"min_lambda_{mode}"] = min(spectra, default=math.nan)
        output[f"min_lambda_per_used_{mode}"] = min(per_used, default=math.nan)
        output[f"min_normalized_lambda_{mode}"] = min(normalized, default=math.nan)
    output["median_N_used"] = statistics.median(
        finite([number(row, "N_used") for row in interval]))
    output["valid_fraction"] = sum(integer(row, "valid") == 1 for row in interval) / len(interval)
    return output


def make_windows(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                 gt_rotation_available, delta, nonoverlap):
    starts = []
    for row in rows:
        index = nearest_index(pair_time, number(row, "timestamp"))
        if index is not None:
            starts.append((index, row))
    starts.sort(key=lambda item: item[0])
    selected = []
    cursor = -1
    for start_index, row in starts:
        if nonoverlap and start_index <= cursor:
            continue
        end_index = nearest_index(pair_time, pair_time[start_index] + delta)
        if end_index is None or end_index <= start_index:
            continue
        selected.append((start_index, end_index, row))
        cursor = end_index
    windows = []
    for start_index, end_index, row in selected:
        start_time = float(pair_time[start_index])
        end_time = float(pair_time[end_index])
        geometry = interval_metrics(rows, start_time, end_time)
        if geometry is None:
            continue
        windows.append({
            "start_time": start_time,
            "end_time": end_time,
            "frame": integer(row, "frame"),
            "translation_error_m": float(np.linalg.norm(
                position_error[end_index] - position_error[start_index])),
            "rotation_error_deg": (
                rotation_error_degrees(pair_est_rot[start_index], pair_est_rot[end_index],
                                       pair_gt_rot[start_index], pair_gt_rot[end_index])
                if gt_rotation_available else math.nan),
            "geometry": geometry,
        })
    return windows


def bootstrap_correlations(windows, target, metrics, delta):
    n = len(windows)
    block = max(1, int(math.ceil(10.0 / delta)))
    samples = {metric: [] for metric in metrics}
    if n >= 3:
        rng = random.Random(BOOTSTRAP_SEED)
        for _ in range(BOOTSTRAP_REPS):
            indices = []
            while len(indices) < n:
                start = rng.randrange(max(1, n - block + 1))
                indices.extend(range(start, min(start + block, n)))
            indices = indices[:n]
            for metric in metrics:
                value = spearman(
                    [windows[index]["geometry"].get(metric, math.nan) for index in indices],
                    [windows[index][target] for index in indices])["rho"]
                if value is not None:
                    samples[metric].append(value)
    return {
        "seed": BOOTSTRAP_SEED,
        "replicates": BOOTSTRAP_REPS if n >= 3 else 0,
        "block_windows": block,
        "metrics": {
            metric: {"median": statistics.median(values) if values else None,
                     "p2_5": percentile(values, 0.025),
                     "p97_5": percentile(values, 0.975),
                     "defined_replicates": len(values)}
            for metric, values in samples.items()
        },
    }


def correlation_metrics():
    return [
        "median_C_L", "median_C_F", "median_delta_C", "median_S_prior",
        "median_G", "median_G_per_used", "max_weak_a_max_R", "max_weak_a_max_t",
        "max_weak_chi_max_R", "max_weak_chi_max_t", "max_weak_psi_max_R",
        "max_weak_psi_max_t", "max_weak_dxL_R", "max_weak_dxF_R",
        "max_weak_dxL_t", "max_weak_dxF_t", "max_weak_ratio_dxL_R",
        "max_weak_ratio_dxF_R", "max_weak_ratio_dxL_t", "max_weak_ratio_dxF_t",
        "max_dcreg_schur_kappa_R", "max_dcreg_schur_kappa_t",
        "min_lambda_per_used_R", "min_lambda_per_used_t",
        "min_normalized_lambda_R", "min_normalized_lambda_t", "median_N_used",
    ]


def events(windows, target):
    finite_indices = [index for index, window in enumerate(windows)
                      if math.isfinite(window[target])]
    if not finite_indices:
        return {"target": target, "n": 0, "top10": None, "bottom50": {}}
    top_count = max(1, int(math.ceil(len(finite_indices) * 0.10)))
    bottom_count = max(1, int(math.ceil(len(finite_indices) * 0.50)))
    top = set(sorted(finite_indices, key=lambda index: windows[index][target], reverse=True)[:top_count])
    result = {"target": target, "n": len(finite_indices),
              "top10_count": len(top), "bottom50": {}}
    for metric in correlation_metrics():
        values = [(index, windows[index]["geometry"].get(metric, math.nan))
                  for index in finite_indices]
        values = [(index, value) for index, value in values if math.isfinite(value)]
        if not values:
            result["bottom50"][metric] = {"defined": False}
            continue
        low_count = max(1, int(math.ceil(len(values) * 0.50)))
        low = set(index for index, _ in sorted(values, key=lambda item: item[1])[:low_count])
        intersection = top & low
        result["bottom50"][metric] = {
            "defined": True,
            "low_count": len(low),
            "top10_intersection": len(intersection),
            "top10_recall": len(intersection) / len(top),
            "bottom50_precision": len(intersection) / len(low),
        }
    return result


def local_analysis(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                   gt_rotation_available):
    output = {}
    for delta in DELTAS:
        overlap = make_windows(rows, pair_time, position_error, pair_est_rot,
                               pair_gt_rot, gt_rotation_available, delta, False)
        nonoverlap = make_windows(rows, pair_time, position_error, pair_est_rot,
                                  pair_gt_rot, gt_rotation_available, delta, True)
        report = {}
        for label, windows in (("overlapping", overlap), ("non_overlapping", nonoverlap)):
            correlation = {}
            bootstrap = {}
            for mode, target in (("translation", "translation_error_m"),
                                 ("rotation", "rotation_error_deg")):
                correlation[target] = {
                    metric: spearman(
                        [window["geometry"].get(metric, math.nan) for window in windows],
                        [window[target] for window in windows])
                    for metric in correlation_metrics()
                }
                if label == "non_overlapping":
                    bootstrap[target] = bootstrap_correlations(
                        windows, target, correlation_metrics(), delta)
            report[label] = {
                "count": len(windows),
                "error_stats": {
                    "translation_error_m": summary([window["translation_error_m"] for window in windows]),
                    "rotation_error_deg": summary([window["rotation_error_deg"] for window in windows]),
                },
                "correlations": correlation,
                "bootstrap": bootstrap,
                "events": {
                    "translation": events(windows, "translation_error_m"),
                    "rotation": events(windows, "rotation_error_deg"),
                },
            }
        output[str(delta)] = report
    return output


def scene_metrics(rows):
    result = {"rows": len(rows), "valid_fraction": sum(integer(row, "valid") == 1 for row in rows) / len(rows)}
    for name in metric_names():
        values = [number(row, name) for row in rows]
        if name.startswith("weak_rank_"):
            result[name] = {"median": statistics.median(finite(values)) if finite(values) else None,
                            "max": max(finite(values), default=None)}
        else:
            result[name] = summary(values)
    result["N_candidate"] = summary([number(row, "N_candidate") for row in rows])
    result["N_used"] = summary([number(row, "N_used") for row in rows])
    result["lidar_rank"] = summary([number(row, "lidar_rank") for row in rows])
    for mode in ("R", "t"):
        minima = []
        per_used = []
        normalized = []
        for row in rows:
            spectrum = finite([number(row, f"lambda_{mode}_{index}") for index in range(3)])
            if not spectrum:
                continue
            minimum = min(abs(value) for value in spectrum)
            maximum = max(abs(value) for value in spectrum)
            minima.append(minimum)
            if maximum > 0.0:
                normalized.append(minimum / maximum)
            used = number(row, "N_used")
            if math.isfinite(used) and used > 0.0:
                per_used.append(minimum / used)
        result[f"min_lambda_{mode}"] = summary(minima)
        result[f"min_lambda_per_used_{mode}"] = summary(per_used)
        result[f"min_normalized_lambda_{mode}"] = summary(normalized)
    return result


def onset_report(name, rows):
    if name != "tunnel2":
        return {"defined": False, "reason": "pre-declared onset is Tunnel2-only"}
    start, end = TUNNEL2_ONSET
    onset = interval_metrics(rows, start, end)
    result = {
        "defined": onset is not None,
        "authority": "Prompt04 pre-declared six-window Tunnel2 position-error onset",
        "start_time": start,
        "end_time": end,
        "onset": onset,
        "lead_intervals": {},
    }
    for lead in (5.0, 3.0, 1.0):
        result["lead_intervals"][str(int(lead))] = {
            "lead_s": lead,
            "pre": interval_metrics(rows, start - lead, start),
            "onset": onset,
        }
    return result


def analyze_scene(name, estimate, ground_truth, consistency):
    est_time, est_pos, est_quat = load_tum(estimate)
    gt_time, gt_pos, gt_quat = load_tum(ground_truth, allow_unsorted=True)
    pairs = associate(est_time, gt_time, MAX_DIFF)
    if len(pairs) < 3:
        raise ValueError(f"{name}: insufficient trajectory matches: {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    pair_time = est_time[ei]
    alignment_rotation, alignment_translation = umeyama_se3(est_pos[ei], gt_pos[gi])
    aligned = (alignment_rotation @ est_pos[ei].T).T + alignment_translation
    position_error = aligned - gt_pos[gi]
    pair_est_rot = np.asarray([rotation_matrix(est_quat[index]) for index in ei])
    gt_rotation_available = all(valid_quaternion(gt_quat[index]) for index in gi)
    pair_gt_rot = (np.asarray([rotation_matrix(gt_quat[index]) for index in gi])
                   if gt_rotation_available else None)
    rows = load_consistency(consistency)
    row_time = np.asarray([number(row, "timestamp") for row in rows])
    local = local_analysis(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                           gt_rotation_available)
    return {
        "sequence": name,
        "estimate": str(pathlib.Path(estimate).resolve()),
        "ground_truth": str(pathlib.Path(ground_truth).resolve()),
        "consistency_csv": str(pathlib.Path(consistency).resolve()),
        "association": {"max_diff_s": MAX_DIFF, "matched": len(pairs),
                         "global_alignment": "single SE3, no per-window realignment"},
        "ground_truth_attitude": {
            "available_for_all_matches": gt_rotation_available,
            "matched_with_valid_attitude": int(sum(valid_quaternion(gt_quat[index]) for index in gi)),
        },
        "frame_authority": {
            "schema_version": 4,
            "rows": len(rows),
            "timestamp_sorted": bool(np.all(np.diff(row_time) > 0)),
            "valid_rows": sum(integer(row, "valid") == 1 for row in rows),
            "first_native_measurement_iteration_only": True,
            "P_pred_capture": "read-only before native UpdateObserve",
        },
        "scene_metrics": scene_metrics(rows),
        "local_errors": local,
        "tunnel2_predeclared_onset": onset_report(name, rows),
    }


def clean(value):
    if isinstance(value, dict):
        return {key: clean(item) for key, item in value.items()}
    if isinstance(value, list):
        return [clean(item) for item in value]
    if isinstance(value, float) and not math.isfinite(value):
        return None
    if isinstance(value, np.generic):
        return clean(value.item())
    return value


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--scene", nargs=4, action="append", required=True,
                        metavar=("NAME", "ESTIMATE", "GT", "CONSISTENCY"))
    parser.add_argument("--out-dir", type=pathlib.Path, required=True)
    parser.add_argument("--combined-out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    try:
        args.out_dir.mkdir(parents=True, exist_ok=True)
        reports = {}
        for name, estimate, ground_truth, consistency in args.scene:
            report = analyze_scene(name, pathlib.Path(estimate), pathlib.Path(ground_truth),
                                   pathlib.Path(consistency))
            reports[name] = report
            (args.out_dir / f"{name}.json").write_text(
                json.dumps(clean(report), indent=2, sort_keys=True) + "\n", encoding="utf-8")
        combined = {
            "analysis": "Prompt05 consistency innovation shadow; evaluation only",
            "bootstrap": {"seed": BOOTSTRAP_SEED, "replicates": BOOTSTRAP_REPS,
                          "block_definition": "ceil(10 s / delta) contiguous windows"},
            "scenes": {name: {"scene_metrics": report["scene_metrics"],
                              "ground_truth_attitude": report["ground_truth_attitude"],
                              "frame_authority": report["frame_authority"],
                              "tunnel2_predeclared_onset": report["tunnel2_predeclared_onset"]}
                       for name, report in reports.items()},
        }
        args.combined_out.parent.mkdir(parents=True, exist_ok=True)
        args.combined_out.write_text(json.dumps(clean(combined), indent=2, sort_keys=True) + "\n",
                                       encoding="utf-8")
        print(f"PROMPT05_ANALYSIS_PASS scenes={','.join(sorted(reports))}")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"PROMPT05_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
