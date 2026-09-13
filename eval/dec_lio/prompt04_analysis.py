#!/usr/bin/env python3
"""Prompt04 causal analysis for Tunnel2 and matched Alpha scenes.

This is an evaluation-only consumer of the D1/D2 CSVs.  It preserves the
Prompt03 association, single global SE(3) alignment, and window definitions,
then adds coupled-Schur zeta and a documented error-onset report.  Nothing in
this module is linked into the estimator.
"""

import argparse
import csv
import json
import math
import pathlib
import statistics
import sys

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3
from eval.dec_lio.prompt03_analysis import (
    clean_json,
    finite,
    integer,
    load_d2,
    load_rows,
    nearest_index,
    number,
    percentile,
    rotation_error_degrees,
    rotation_matrix,
    summary_stats,
)

DELTAS = (1.0, 5.0, 10.0)
ASSOCIATION_MAX_DIFF = 0.1


def valid_quaternion(quaternion):
    values = np.asarray(quaternion, dtype=float)
    norm = float(np.linalg.norm(values))
    return bool(np.isfinite(norm) and norm > 1.0e-12)


def weak_eta_values(row, mode):
    rank = max(0, integer(row, f"d1_weak_rank_{mode}"))
    return finite([number(row, f"eta_{mode}_{index}") for index in range(min(3, rank))])


def weak_zeta_values(row, mode):
    rank = max(0, integer(row, f"d1_weak_rank_{mode}"))
    return [number(row, f"zeta_{mode}_{index}") for index in range(min(3, rank))
            if integer(row, f"zeta_valid_{mode}_{index}") == 1 and
            math.isfinite(number(row, f"zeta_{mode}_{index}"))]


def interval_geometry(rows, start, end):
    """Compute Prompt03 geometry plus zeta over one time interval."""
    interval = [row for row in rows
                if start - 1.0e-9 <= number(row, "timestamp") <= end + 1.0e-9]
    if not interval:
        return None
    result = {"frame_count": len(interval)}
    for mode in ("rot", "trans"):
        kappa = finite([number(row, f"dcreg_schur_kappa_{mode}") for row in interval])
        raw_kappa = finite([number(row, f"raw_block_kappa_{mode}") for row in interval])
        lambda_min = []
        lambda_normalized = []
        lambda_per_used = []
        lc_values, ls_values = [], []
        mu_min, mu_median, mu_max = [], [], []
        eta_weak, zeta_weak = [], []
        class_counts = {label: 0 for label in ("FULL", "PARTIAL", "NONE")}
        class_total = 0
        for row in interval:
            spectrum = finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)])
            if spectrum:
                smallest = min(abs(value) for value in spectrum)
                largest = max(abs(value) for value in spectrum)
                lambda_min.append(smallest)
                lambda_normalized.append(smallest / largest if largest > 0.0 else 0.0)
                used = number(row, "used_residual_count")
                if math.isfinite(used) and used > 0.0:
                    lambda_per_used.append(smallest / used)
            lc_values.extend(finite([number(row, f"xicp_lc_{mode}_{index}") for index in range(3)]))
            ls_values.extend(finite([number(row, f"xicp_ls_{mode}_{index}") for index in range(3)]))
            for index in range(3):
                label = row.get(f"xicp_class_{mode}_{index}", "")
                if label in class_counts:
                    class_counts[label] += 1
                    class_total += 1
            mu_values = finite([number(row, f"mu_{index}") for index in range(6)])
            if mu_values:
                mu_min.append(min(mu_values))
                mu_median.append(statistics.median(mu_values))
                mu_max.append(max(mu_values))
            eta_weak.extend(weak_eta_values(row, mode))
            zeta_weak.extend(weak_zeta_values(row, mode))
        result.update({
            f"max_log_kappa_{mode}": max((math.log(max(value, 1.0e-300)) for value in kappa),
                                           default=math.nan),
            f"max_log_raw_kappa_{mode}": max((math.log(max(value, 1.0e-300)) for value in raw_kappa),
                                               default=math.nan),
            f"min_abs_lambda_{mode}": min(lambda_min, default=math.nan),
            f"median_abs_lambda_{mode}": statistics.median(lambda_min) if lambda_min else math.nan,
            f"min_normalized_lambda_{mode}": min(lambda_normalized, default=math.nan),
            f"min_abs_lambda_per_used_{mode}": min(lambda_per_used, default=math.nan),
            f"xicp_lc_min_{mode}": min(lc_values, default=math.nan),
            f"xicp_ls_min_{mode}": min(ls_values, default=math.nan),
            f"xicp_class_full_fraction_{mode}": class_counts["FULL"] / class_total if class_total else math.nan,
            f"xicp_class_partial_fraction_{mode}": class_counts["PARTIAL"] / class_total if class_total else math.nan,
            f"xicp_class_none_fraction_{mode}": class_counts["NONE"] / class_total if class_total else math.nan,
            f"xicp_nonfull_fraction_{mode}": (class_counts["PARTIAL"] + class_counts["NONE"]) / class_total
            if class_total else math.nan,
            f"mu_min_{mode}": min(mu_min, default=math.nan),
            f"mu_median_{mode}": statistics.median(mu_median) if mu_median else math.nan,
            f"mu_max_{mode}": max(mu_max, default=math.nan),
            f"eta_weak_min_{mode}": min(eta_weak, default=math.nan),
            f"eta_weak_max_{mode}": max(eta_weak, default=math.nan),
            f"zeta_weak_min_{mode}": min(zeta_weak, default=math.nan),
            f"zeta_weak_median_{mode}": statistics.median(zeta_weak) if zeta_weak else math.nan,
            f"zeta_weak_max_{mode}": max(zeta_weak, default=math.nan),
            f"zeta_weak_valid_fraction_{mode}": len(zeta_weak) /
            max(1, sum(max(0, integer(row, f"d1_weak_rank_{mode}")) for row in interval)),
        })
        result[f"weak_fraction_{mode}"] = sum(
            integer(row, f"d1_weak_rank_{mode}") > 0 for row in interval) / len(interval)
    used = finite([number(row, "used_residual_count") for row in interval])
    result["median_used_residual_count"] = statistics.median(used) if used else math.nan
    result["min_used_residual_count"] = min(used) if used else math.nan
    return result


def make_windows(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                 gt_rotation_available, delta, max_diff, nonoverlap):
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
        geometry = interval_geometry(rows, start_time, end_time)
        if geometry is None:
            continue
        displacement = position_error[end_index] - position_error[start_index]
        rotation_error = None
        if gt_rotation_available:
            rotation_error = rotation_error_degrees(
                pair_est_rot[start_index], pair_est_rot[end_index],
                pair_gt_rot[start_index], pair_gt_rot[end_index])
        windows.append({
            "start_time": start_time,
            "end_time": end_time,
            "translation_error_m": float(np.linalg.norm(displacement)),
            "rotation_error_deg": rotation_error,
            "geometry": geometry,
            "frame": integer(row, "frame"),
        })
    return windows


def window_sample(window):
    geometry = window["geometry"]
    sample = {
        "start_time": window["start_time"],
        "end_time": window["end_time"],
        "kappa_rot": math.exp(geometry.get("max_log_kappa_rot", math.nan)),
        "kappa_trans": math.exp(geometry.get("max_log_kappa_trans", math.nan)),
        "translation_error_m": window["translation_error_m"],
        "rotation_error_deg": window["rotation_error_deg"],
        "lambda_min_rot": geometry.get("min_abs_lambda_rot", math.nan),
        "lambda_min_trans": geometry.get("min_abs_lambda_trans", math.nan),
        "lambda_per_used_rot": geometry.get("min_abs_lambda_per_used_rot", math.nan),
        "lambda_per_used_trans": geometry.get("min_abs_lambda_per_used_trans", math.nan),
        "xicp_lc_min_rot": geometry.get("xicp_lc_min_rot", math.nan),
        "xicp_lc_min_trans": geometry.get("xicp_lc_min_trans", math.nan),
        "xicp_ls_min_rot": geometry.get("xicp_ls_min_rot", math.nan),
        "xicp_ls_min_trans": geometry.get("xicp_ls_min_trans", math.nan),
        "xicp_nonfull_fraction_rot": geometry.get("xicp_nonfull_fraction_rot", math.nan),
        "xicp_nonfull_fraction_trans": geometry.get("xicp_nonfull_fraction_trans", math.nan),
        "xicp_partial_fraction_rot": geometry.get("xicp_class_partial_fraction_rot", math.nan),
        "xicp_partial_fraction_trans": geometry.get("xicp_class_partial_fraction_trans", math.nan),
        "xicp_none_fraction_rot": geometry.get("xicp_class_none_fraction_rot", math.nan),
        "xicp_none_fraction_trans": geometry.get("xicp_class_none_fraction_trans", math.nan),
        "mu_min_rot": geometry.get("mu_min_rot", math.nan),
        "mu_min_trans": geometry.get("mu_min_trans", math.nan),
        "eta_weak_min_rot": geometry.get("eta_weak_min_rot", math.nan),
        "eta_weak_min_trans": geometry.get("eta_weak_min_trans", math.nan),
        "zeta_weak_min_rot": geometry.get("zeta_weak_min_rot", math.nan),
        "zeta_weak_min_trans": geometry.get("zeta_weak_min_trans", math.nan),
        "median_used_residual_count": geometry.get("median_used_residual_count", math.nan),
    }
    return sample


def error_summary(windows):
    return {
        "translation_error_m": summary_stats([window["translation_error_m"] for window in windows]),
        "rotation_error_deg": summary_stats([window["rotation_error_deg"] for window in windows]),
    }


SIGNALS_BY_MODE = {
    "rot": {
        "kappa": "kappa_rot", "lambda": "lambda_min_rot",
        "lambda_per_used": "lambda_per_used_rot", "xicp_lc": "xicp_lc_min_rot",
        "xicp_ls": "xicp_ls_min_rot", "mu": "mu_min_rot",
        "eta": "eta_weak_min_rot", "zeta": "zeta_weak_min_rot",
        "N_used": "median_used_residual_count",
    },
    "trans": {
        "kappa": "kappa_trans", "lambda": "lambda_min_trans",
        "lambda_per_used": "lambda_per_used_trans", "xicp_lc": "xicp_lc_min_trans",
        "xicp_ls": "xicp_ls_min_trans", "mu": "mu_min_trans",
        "eta": "eta_weak_min_trans", "zeta": "zeta_weak_min_trans",
        "N_used": "median_used_residual_count",
    },
}


def median_field(samples, field):
    return statistics.median(finite([sample.get(field, math.nan) for sample in samples])) \
        if finite([sample.get(field, math.nan) for sample in samples]) else None


def contiguous_runs(indices):
    if not indices:
        return []
    runs = []
    start = previous = indices[0]
    for index in indices[1:]:
        if index != previous + 1:
            runs.append((start, previous))
            start = index
        previous = index
    runs.append((start, previous))
    return runs


def error_onset(windows_by_delta):
    windows = windows_by_delta["5.0"]["_nonoverlap_objects"]
    errors = [window["translation_error_m"] for window in windows]
    rotation_errors = [window["rotation_error_deg"] for window in windows]
    rotation_valid = finite(rotation_errors)
    rotation_cut = percentile(rotation_valid, 0.90)
    valid = finite(errors)
    if not valid:
        return {"defined": False, "reason": "no finite translation local-error windows"}
    median = statistics.median(valid)
    mad = statistics.median([abs(value - median) for value in valid])
    robust_threshold = median + 3.0 * mad
    p90 = percentile(valid, 0.90)
    above = [index for index, value in enumerate(errors)
             if math.isfinite(value) and value > robust_threshold]
    sustained = [(start, end) for start, end in contiguous_runs(above) if end - start + 1 >= 3]
    top = [index for index, value in enumerate(errors)
           if math.isfinite(value) and p90 is not None and value >= p90]
    first_index = (sustained[0][0] if sustained else (top[0] if top else None))
    result = {
        "defined": True,
        "window": "non-overlapping 5 s windows",
        "high_error_definition": "top 10 percent by local translation error",
        "sustained_definition": "at least 3 consecutive windows above median + 3*MAD",
        "sequence_median_m": median,
        "sequence_mad_m": mad,
        "sustained_threshold_m": robust_threshold,
        "top10_cut_m": p90,
        "rotation_error": {
            "defined": bool(rotation_valid),
            "reason": None if rotation_valid else "Tunnel2 GT is position-only (quaternion columns are zero)",
            "top10_cut_deg": rotation_cut,
            "top10_window_count": len([value for value in rotation_valid
                                        if rotation_cut is not None and value >= rotation_cut]),
        },
        "top10_intervals": [
            {"start_time": windows[index]["start_time"], "end_time": windows[index]["end_time"],
             "translation_error_m": errors[index]}
            for index in top
        ],
        "sustained_intervals": [
            {"start_time": windows[start]["start_time"], "end_time": windows[end]["end_time"],
             "window_count": end - start + 1,
             "max_translation_error_m": max(errors[start:end + 1])}
            for start, end in sustained
        ],
        "first_onset_window": (None if first_index is None else {
            "start_time": windows[first_index]["start_time"],
            "end_time": windows[first_index]["end_time"],
            "translation_error_m": errors[first_index],
            "source": "sustained_above_robust_threshold" if sustained and first_index == sustained[0][0]
            else "first_top10_window",
        }),
        "lead_signals": {},
    }
    if first_index is None:
        return result
    onset_start = windows[first_index]["start_time"]
    onset_end = windows[first_index]["end_time"]
    one_second = windows_by_delta["1.0"]["_overlap_objects"]
    onset_samples = [window_sample(window) for window in one_second
                     if window["start_time"] < onset_end and window["end_time"] > onset_start]
    for lead in (1.0, 3.0, 5.0):
        pre_samples = [window_sample(window) for window in one_second
                       if onset_start - lead <= window["end_time"] <= onset_start + 1.0e-9]
        report = {"lead_s": lead, "pre_window_count": len(pre_samples),
                  "onset_window_count": len(onset_samples)}
        report["translation_error_m"] = {
            "pre_median": median_field(pre_samples, "translation_error_m"),
            "onset_median": median_field(onset_samples, "translation_error_m"),
        }
        report["signals"] = {
            mode: {
                name: {"pre_median": median_field(pre_samples, field),
                       "onset_median": median_field(onset_samples, field)}
                for name, field in signal_fields.items()
            }
            for mode, signal_fields in SIGNALS_BY_MODE.items()
        }
        result["lead_signals"][str(int(lead))] = report
    return result


def scene_summary(rows):
    result = {"d2_rows": len(rows), "d1_valid_fraction": sum(integer(row, "d1_valid") == 1 for row in rows) / len(rows)}
    for mode in ("rot", "trans"):
        classes = [row.get(f"xicp_class_{mode}_{index}") for row in rows for index in range(3)]
        result[mode] = {
            "kappa_schur": summary_stats([number(row, f"dcreg_schur_kappa_{mode}") for row in rows]),
            "kappa_raw": summary_stats([number(row, f"raw_block_kappa_{mode}") for row in rows]),
            "class_fractions": {label: classes.count(label) / len(classes) if classes else None
                                 for label in ("FULL", "PARTIAL", "NONE")},
            "Lc": summary_stats([number(row, f"xicp_lc_{mode}_{index}") for row in rows for index in range(3)]),
            "Ls": summary_stats([number(row, f"xicp_ls_{mode}_{index}") for row in rows for index in range(3)]),
            "lambda_min": summary_stats([
                min(abs(value) for value in finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)]))
                if finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)]) else math.nan
                for row in rows]),
            "lambda_per_used": summary_stats([
                min(abs(value) for value in finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)])) /
                number(row, "used_residual_count")
                if finite([number(row, f"d1_lambda_{mode}_{index}") for index in range(3)]) and
                number(row, "used_residual_count") > 0 else math.nan
                for row in rows]),
            "mu_min": summary_stats([number(row, "mu_min") for row in rows]),
            "eta_weak": summary_stats([min(weak_eta_values(row, mode), default=math.nan) for row in rows]),
            "zeta_weak": summary_stats([min(weak_zeta_values(row, mode), default=math.nan) for row in rows]),
            "zeta_valid_fraction": sum(bool(weak_zeta_values(row, mode)) for row in rows) / len(rows),
        }
    result["N_used"] = summary_stats([number(row, "used_residual_count") for row in rows])
    return result


def analyze_scene(name, estimate_path, ground_truth_path, d2_path, d1_raw_path, max_diff):
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
    gt_rotation_available = all(valid_quaternion(gt_quat[index]) for index in gi)
    pair_gt_rot = (np.asarray([rotation_matrix(gt_quat[index]) for index in gi])
                   if gt_rotation_available else None)
    alignment_rotation, alignment_translation = umeyama_se3(pair_est_pos, pair_gt_pos)
    aligned = (alignment_rotation @ pair_est_pos.T).T + alignment_translation
    position_error = aligned - pair_gt_pos
    rows = load_d2(d2_path)
    raw_rows = load_rows(d1_raw_path, 2)
    windows = {}
    for delta in DELTAS:
        overlap = make_windows(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                               gt_rotation_available, delta, max_diff, False)
        nonoverlap = make_windows(rows, pair_time, position_error, pair_est_rot, pair_gt_rot,
                                  gt_rotation_available, delta, max_diff, True)
        windows[str(delta)] = {
            "overlapping": {"count": len(overlap), "error_stats": error_summary(overlap),
                             "window_samples": [window_sample(window) for window in overlap]},
            "non_overlapping": {"count": len(nonoverlap), "error_stats": error_summary(nonoverlap),
                                "window_samples": [window_sample(window) for window in nonoverlap]},
        }
        windows[str(delta)]["_overlap_objects"] = overlap
        windows[str(delta)]["_nonoverlap_objects"] = nonoverlap
    output = {
        "sequence": name,
        "estimate": str(pathlib.Path(estimate_path).resolve()),
        "ground_truth": str(pathlib.Path(ground_truth_path).resolve()),
        "d2_summary": str(pathlib.Path(d2_path).resolve()),
        "d1_raw": str(pathlib.Path(d1_raw_path).resolve()),
        "association": {"max_diff_s": max_diff, "matched": len(pairs),
                         "global_alignment": "single SE3, no per-window realignment"},
        "ground_truth_attitude": {"available_for_all_matches": gt_rotation_available,
                                   "matched_with_valid_attitude": sum(
                                       valid_quaternion(gt_quat[index]) for index in gi)},
        "frame_authority": {"rows": len(rows), "all_iteration_zero": True,
                             "valid_rows": sum(integer(row, "d1_valid") == 1 for row in rows),
                             "invalid_rows": sum(integer(row, "d1_valid") != 1 for row in rows)},
        "raw_d1_rows": len(raw_rows),
        "scene_summary": scene_summary(rows),
        "local_errors": {key: {label: value["error_stats"] for label, value in report.items()
                                if not label.startswith("_")}
                         for key, report in windows.items()},
        "windows": {key: {label: {k: v for k, v in value.items() if not k.startswith("_")}
                           for label, value in report.items() if not label.startswith("_")}
                    for key, report in windows.items()},
    }
    output["error_onset"] = error_onset(windows)
    return output, windows


def matched_bins(reports):
    bins = ((-math.inf, 5.0, "<=5"), (5.0, 10.0, "5-10"),
            (10.0, 20.0, "10-20"), (20.0, math.inf, ">20"))
    result = {"delta_s": 5.0, "kappa_bins": {}}
    for lower, upper, label in bins:
        result["kappa_bins"][label] = {"lower_exclusive": lower, "upper_inclusive": upper,
                                        "sequences": {}}
        for name, report in reports.items():
            samples = report["windows"]["5.0"]["overlapping"]["window_samples"]
            selected = {}
            for mode in ("rot", "trans"):
                key = f"kappa_{mode}"
                subset = [sample for sample in samples
                          if math.isfinite(sample.get(key, math.nan)) and
                          lower < sample[key] <= upper]
                selected[mode] = {
                    "window_count": len(subset),
                    "local_translation_error_m": summary_stats(
                        [sample["translation_error_m"] for sample in subset]),
                    "local_rotation_error_deg": summary_stats(
                        [sample["rotation_error_deg"] for sample in subset]),
                    "lambda_min": summary_stats([sample[f"lambda_min_{mode}"] for sample in subset]),
                    "lambda_min_per_used": summary_stats([sample[f"lambda_per_used_{mode}"] for sample in subset]),
                    "xicp_Lc": summary_stats([sample[f"xicp_lc_min_{mode}"] for sample in subset]),
                    "xicp_Ls": summary_stats([sample[f"xicp_ls_min_{mode}"] for sample in subset]),
                    "xicp_nonfull": summary_stats([sample[f"xicp_nonfull_fraction_{mode}"] for sample in subset]),
                    "xicp_partial": summary_stats([sample[f"xicp_partial_fraction_{mode}"] for sample in subset]),
                    "xicp_none": summary_stats([sample[f"xicp_none_fraction_{mode}"] for sample in subset]),
                    "mu_min": summary_stats([sample[f"mu_min_{mode}"] for sample in subset]),
                    "eta_weak": summary_stats([sample[f"eta_weak_min_{mode}"] for sample in subset]),
                    "zeta_weak": summary_stats([sample[f"zeta_weak_min_{mode}"] for sample in subset]),
                    "N_used": summary_stats([sample["median_used_residual_count"] for sample in subset]),
                }
            result["kappa_bins"][label]["sequences"][name] = selected
    return result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--scene", nargs=5, action="append", metavar=("NAME", "ESTIMATE", "GT", "D2", "D1"),
                        required=True, help="scene specification; may be repeated")
    parser.add_argument("--max-diff", type=float, default=ASSOCIATION_MAX_DIFF)
    parser.add_argument("--out-dir", type=pathlib.Path, required=True)
    parser.add_argument("--combined-out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    reports = {}
    try:
        args.out_dir.mkdir(parents=True, exist_ok=True)
        for name, estimate, ground_truth, d2, d1 in args.scene:
            report, _ = analyze_scene(name, pathlib.Path(estimate), pathlib.Path(ground_truth),
                                      pathlib.Path(d2), pathlib.Path(d1), args.max_diff)
            reports[name] = report
            (args.out_dir / f"{name}.json").write_text(
                json.dumps(clean_json(report), indent=2, sort_keys=True) + "\n", encoding="utf-8")
        combined = {"association_max_diff_s": args.max_diff, "scenes": sorted(reports),
                    "matched_kappa_bins": matched_bins(reports)}
        args.combined_out.parent.mkdir(parents=True, exist_ok=True)
        args.combined_out.write_text(json.dumps(clean_json(combined), indent=2, sort_keys=True) + "\n",
                                      encoding="utf-8")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"PROMPT04_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
