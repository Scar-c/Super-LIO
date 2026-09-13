#!/usr/bin/env python3
"""Prompt06 physical weak-axis and Tunnel2 course-causality audit.

The estimator CSV is schema 5 and contains only native pre-update state and
shadow quantities.  This evaluation-only consumer reconstructs the course
axes from the supplied trajectory/reference pair and one global SE(3)
alignment.  It never feeds a result back to the estimator.
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

MAX_DIFF = 0.1
ONSET = (1706584541.828, 1706584579.030)
WINDOWS = (0.5, 1.0, 2.0)
SPEED_THRESHOLDS = (0.05, 0.10, 0.20)
AXIS_GATE = 0.8


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


def pct(values, fraction):
    values = finite(values)
    return float(np.percentile(values, fraction * 100.0)) if values else None


def stats(values):
    values = finite(values)
    return {
        "n": len(values),
        "median": statistics.median(values) if values else None,
        "p10": pct(values, 0.10),
        "p75": pct(values, 0.75),
        "p90": pct(values, 0.90),
        "p95": pct(values, 0.95),
        "p99": pct(values, 0.99),
        "max": max(values) if values else None,
    }


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


def nearest_index(times, target, max_diff=MAX_DIFF):
    if len(times) == 0:
        return None
    index = int(np.searchsorted(times, target))
    candidates = [candidate for candidate in (index - 1, index)
                  if 0 <= candidate < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda item: abs(float(times[item] - target)))
    return best if abs(float(times[best] - target)) <= max_diff else None


def load_axis(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or "schema_version" not in reader.fieldnames:
            raise ValueError(f"{path}: missing schema_version")
        rows = [row for row in reader if row.get("frame", "") != ""]
    versions = {integer(row, "schema_version") for row in rows}
    if versions != {5}:
        raise ValueError(f"{path}: expected schema_version=5, got {sorted(versions)}")
    frames = [integer(row, "frame") for row in rows]
    if len(frames) != len(set(frames)):
        raise ValueError(f"{path}: duplicate frame rows")
    rows.sort(key=lambda row: number(row, "timestamp"))
    return rows


def matrix3(row, prefix):
    return np.asarray([number(row, f"{prefix}_{index}") for index in range(9)],
                      dtype=float).reshape(3, 3)


def vector3(row, prefix):
    return np.asarray([number(row, f"{prefix}_{index}") for index in range(3)],
                      dtype=float)


def fit_velocity(times, positions, center, half_window):
    mask = (times >= center - half_window - 1.0e-9) & \
           (times <= center + half_window + 1.0e-9)
    indices = np.flatnonzero(mask)
    if len(indices) < 3:
        return None
    x = times[indices] - center
    if np.ptp(x) <= 1.0e-9:
        return None
    design = np.column_stack((np.ones(len(indices)), x))
    coefficients, _, _, _ = np.linalg.lstsq(design, positions[indices], rcond=None)
    residual = design @ coefficients - positions[indices]
    velocity = coefficients[1]
    return {
        "velocity": velocity,
        "sample_count": int(len(indices)),
        "speed_mps": float(np.linalg.norm(velocity)),
        "fit_rmse_m": float(np.sqrt(np.mean(np.sum(residual * residual, axis=1)))),
    }


def horizontal(vector, gravity):
    unit = gravity / np.linalg.norm(gravity)
    return vector - unit * unit.dot(vector)


def course_error(gt_velocity, estimator_velocity, gravity, threshold):
    if np.linalg.norm(gravity) <= 1.0e-12:
        return None
    gt_h = horizontal(gt_velocity, gravity)
    est_h = horizontal(estimator_velocity, gravity)
    if np.linalg.norm(gt_h) <= threshold or np.linalg.norm(est_h) <= threshold:
        return None
    gt_h = gt_h / np.linalg.norm(gt_h)
    est_h = est_h / np.linalg.norm(est_h)
    return float(np.arctan2(np.dot(gravity / np.linalg.norm(gravity),
                                   np.cross(gt_h, est_h)),
                            np.dot(gt_h, est_h)))


def local_rotation_error(est_start, est_end, gt_start, gt_end):
    relative_error = (gt_start.T @ gt_end).T @ (est_start.T @ est_end)
    cosine = np.clip((np.trace(relative_error) - 1.0) * 0.5, -1.0, 1.0)
    return float(np.degrees(np.arccos(cosine)))


def interval(rows, start=None, end=None):
    if not rows:
        return []
    return [row for row in rows
            if (start is None or number(row, "timestamp") >= start - 1.0e-9) and
               (end is None or number(row, "timestamp") <= end + 1.0e-9)]


def axis_summary(rows):
    valid_rows = [row for row in rows if integer(row, "valid") == 1]
    result = {
        "rows": len(rows),
        "valid_rows": len(valid_rows),
        "valid_fraction": len(valid_rows) / len(rows) if rows else 0.0,
        "weak_rank_R": stats([number(row, "weak_rank_R") for row in valid_rows]),
    }
    for name in ("O_yaw", "O_long", "O_lat"):
        result[name] = stats([number(row, name) for row in valid_rows])
    counts = {}
    for row in valid_rows:
        label = row.get("offline_classification", row.get("classification", ""))
        counts[label] = counts.get(label, 0) + 1
    result["classification_counts"] = counts
    result["dominant_interpretation"] = (
        max(counts, key=counts.get) if counts else "NO_ROTATIONAL_WEAK_SUBSPACE")
    result["fraction_O_yaw_ge_0.8"] = (
        sum(number(row, "O_yaw") >= AXIS_GATE for row in valid_rows) /
        len(valid_rows) if valid_rows else 0.0)
    return result


def forcing_summary(rows):
    names = ("weak_chi_max_R", "weak_psi_max_R", "Psi_weak_R", "A_weak_R",
             "C_yaw_L", "C_yaw_F", "C_L", "G_per_used")
    return {name: stats([number(row, name) for row in rows]) for name in names}


def exact_runs(rows, predicate):
    longest = {"frames": 0, "start_frame": None, "end_frame": None,
               "start_time": None, "end_time": None, "duration_s": 0.0}
    current = []
    for row in rows:
        if predicate(row) and (not current or
                               integer(row, "frame") == integer(current[-1], "frame") + 1):
            current.append(row)
        else:
            if len(current) > longest["frames"]:
                longest = run_record(current)
            current = [row] if predicate(row) else []
    if len(current) > longest["frames"]:
        longest = run_record(current)
    return longest


def run_record(rows):
    return {
        "frames": len(rows),
        "start_frame": integer(rows[0], "frame"),
        "end_frame": integer(rows[-1], "frame"),
        "start_time": number(rows[0], "timestamp"),
        "end_time": number(rows[-1], "timestamp"),
        "duration_s": number(rows[-1], "timestamp") - number(rows[0], "timestamp"),
    }


def persistence(rows, baseline):
    values = finite([number(row, baseline[0]) for row in rows])
    threshold = baseline[1]
    valid = [row for row in rows if math.isfinite(number(row, baseline[0]))]
    above = [row for row in valid if number(row, baseline[0]) > threshold]
    run = exact_runs(valid, lambda row: number(row, baseline[0]) > threshold)
    return {
        "metric": baseline[0], "threshold": threshold,
        "valid_frames": len(valid), "above_frames": len(above),
        "above_fraction": len(above) / len(valid) if valid else 0.0,
        "longest_exact_run": run,
        "distribution": stats(values),
    }


def frame_metrics(rows):
    return {
        "O_yaw": [number(row, "O_yaw") for row in rows],
        "O_long": [number(row, "O_long") for row in rows],
        "O_lat": [number(row, "O_lat") for row in rows],
        "weak_chi_max_R": [number(row, "weak_chi_max_R") for row in rows],
        "weak_psi_max_R": [number(row, "weak_psi_max_R") for row in rows],
        "Psi_weak_R": [number(row, "Psi_weak_R") for row in rows],
        "A_weak_R": [number(row, "A_weak_R") for row in rows],
        "C_yaw_L": [number(row, "C_yaw_L") for row in rows],
        "C_yaw_F": [number(row, "C_yaw_F") for row in rows],
        "C_L": [number(row, "C_L") for row in rows],
        "G_per_used": [number(row, "G_per_used") for row in rows],
        "kappa_R": [number(row, "dcreg_schur_kappa_R") for row in rows],
    }


def course_records(axis_rows, est_time, est_pos, gt_time, gt_pos,
                   alignment_rotation, half_window, speed_threshold):
    records = []
    for row in axis_rows:
        timestamp = number(row, "timestamp")
        gt_fit = fit_velocity(gt_time, gt_pos, timestamp, half_window)
        est_fit = fit_velocity(est_time, est_pos, timestamp, half_window)
        gravity = vector3(row, "gravity_world")
        if gt_fit is None or est_fit is None or not np.isfinite(gravity).all() or \
                np.linalg.norm(gravity) <= 1.0e-12:
            continue
        gt_in_estimator = alignment_rotation.T @ gt_fit["velocity"]
        est_in_gt = alignment_rotation @ est_fit["velocity"]
        error = course_error(gt_in_estimator, est_fit["velocity"], gravity,
                             speed_threshold)
        if error is None:
            continue
        R = matrix3(row, "preupdate_R")
        ghat = gravity / np.linalg.norm(gravity)
        course = gt_in_estimator - ghat * ghat.dot(gt_in_estimator)
        course_norm = np.linalg.norm(course)
        if course_norm <= 1.0e-12 or not np.isfinite(R).all():
            continue
        course = course / course_norm
        u_yaw = vector3(row, "u_yaw")
        if np.linalg.norm(u_yaw) <= 1.0e-12:
            continue
        u_yaw = u_yaw / np.linalg.norm(u_yaw)
        u_long = R.T @ course
        u_long = u_long / np.linalg.norm(u_long)
        u_lat = np.cross(u_yaw, u_long)
        if np.linalg.norm(u_lat) <= 1.0e-12:
            continue
        u_lat = u_lat / np.linalg.norm(u_lat)
        projector = matrix3(row, "weak_projector_R")
        occupancy = np.asarray([u_yaw.dot(projector @ u_yaw),
                                u_long.dot(projector @ u_long),
                                u_lat.dot(projector @ u_lat)])
        rank = integer(row, "weak_rank_R")
        records.append({
            "frame": integer(row, "frame"), "timestamp": timestamp,
            "course_error_rad": error,
            "course_error_deg": math.degrees(error),
            "abs_course_error_deg": abs(math.degrees(error)),
            "u_yaw": u_yaw.tolist(), "u_long": u_long.tolist(),
            "u_lat": u_lat.tolist(), "occupancy": occupancy.tolist(),
            "occupancy_sum": float(np.sum(occupancy)),
            "weak_rank_R": rank,
            "invariant_error": float(abs(np.sum(occupancy) - rank)),
            "gt_sample_count": gt_fit["sample_count"],
            "est_sample_count": est_fit["sample_count"],
            "gt_speed_mps": gt_fit["speed_mps"],
            "est_speed_mps": est_fit["speed_mps"],
            "gt_fit_rmse_m": gt_fit["fit_rmse_m"],
            "est_fit_rmse_m": est_fit["fit_rmse_m"],
            "gt_velocity_estimator_frame": gt_in_estimator.tolist(),
            "est_velocity_estimator_frame": est_fit["velocity"].tolist(),
            "est_velocity_gt_frame": est_in_gt.tolist(),
        })
    return records


def add_offline_axis_fields(rows, records):
    by_frame = {record["frame"]: record for record in records}
    output = []
    for row in rows:
        record = by_frame.get(integer(row, "frame"))
        enriched = dict(row)
        if record is not None:
            enriched["O_yaw"] = record["occupancy"][0]
            enriched["O_long"] = record["occupancy"][1]
            enriched["O_lat"] = record["occupancy"][2]
            enriched["offline_classification"] = classify(
                record["occupancy"], record["weak_rank_R"])
        output.append(enriched)
    return output


def classify(occupancy, rank):
    if rank <= 0:
        return "NO_ROTATIONAL_WEAK_SUBSPACE"
    maximum = max(occupancy)
    if maximum < AXIS_GATE:
        return "MIXED"
    index = int(np.argmax(occupancy))
    return ("YAW_LIKE", "LONGITUDINAL_ROLL_LIKE", "LATERAL_TILT_LIKE")[index]


def local_errors(axis_rows, est_time, est_pos, gt_time, gt_pos,
                 est_quat, gt_quat, alignment_rotation, alignment_translation,
                 gt_attitude):
    aligned_est = (alignment_rotation @ est_pos.T).T + alignment_translation
    paired = associate(est_time, gt_time, MAX_DIFF)
    pair_est = {int(ei): int(gi) for ei, gi, _ in paired}
    # Position error is evaluated at the nearest globally associated sample.
    output = []
    for row in axis_rows:
        ei = nearest_index(est_time, number(row, "timestamp"))
        if ei is None or ei not in pair_est:
            continue
        gi = pair_est[ei]
        sample = {"frame": integer(row, "frame"),
                  "timestamp": number(row, "timestamp"),
                  "translation_error_m": float(np.linalg.norm(aligned_est[ei] - gt_pos[gi]))}
        if gt_attitude:
            for delta in (1.0, 5.0, 10.0):
                end = nearest_index(est_time, est_time[ei] + delta)
                if end is None or end not in pair_est:
                    sample[f"rotation_error_{int(delta)}s_deg"] = math.nan
                    continue
                sample[f"rotation_error_{int(delta)}s_deg"] = local_rotation_error(
                    rotation_matrix(est_quat[ei]), rotation_matrix(est_quat[end]),
                    rotation_matrix(gt_quat[pair_est[ei]]),
                    rotation_matrix(gt_quat[pair_est[end]]))
        output.append(sample)
    return output


def merge_row_metrics(rows, local):
    local_by_frame = {item["frame"]: item for item in local}
    output = []
    for row in rows:
        item = local_by_frame.get(integer(row, "frame"))
        merged = dict(row)
        if item is not None:
            merged.update(item)
        output.append(merged)
    return output


def physical_summary(rows):
    result = axis_summary(rows)
    result["forcing"] = forcing_summary(rows)
    for name in ("Psi_weak_R", "A_weak_R", "weak_chi_max_R", "weak_psi_max_R"):
        result["persistence_" + name] = persistence(
            rows, (name, pct([number(r, name) for r in rows], 0.95) or math.inf))
    return result


def onset_windows(rows):
    if not rows:
        return {}
    start, end = ONSET
    return {
        "pre5": interval(rows, start - 5.0, start),
        "pre3": interval(rows, start - 3.0, start),
        "pre1": interval(rows, start - 1.0, start),
        "onset": interval(rows, start, end),
        "full": rows,
    }


def yaw_gate(rows):
    start, end = ONSET
    onset = interval(rows, start, end)
    values = [number(row, "O_yaw") for row in onset if integer(row, "valid") == 1]
    median = statistics.median(values) if values else math.nan
    fraction = sum(value >= AXIS_GATE for value in values) / len(values) if values else 0.0
    passed = bool(values) and median >= AXIS_GATE and fraction >= 0.70
    return {"onset_start": start, "onset_end": end, "valid_frames": len(values),
            "median_O_yaw": median, "fraction_O_yaw_ge_0.8": fraction,
            "pass": passed,
            "marker": None if passed else "YAW_CAUSAL_CHAIN_NOT_AUTHORIZED"}


def course_report(records, axis_rows):
    def report(items, half, threshold):
        errors = [item["course_error_deg"] for item in items]
        return {
            "half_window_s": half, "window_duration_s": 2.0 * half,
            "speed_threshold_mps": threshold,
            "valid_records": len(items),
            "axis_rows": len(axis_rows),
            "valid_coverage": len(items) / len(axis_rows) if axis_rows else 0.0,
            "course_error_deg": stats(errors),
            "fit_sample_count_gt": stats([item["gt_sample_count"] for item in items]),
            "fit_sample_count_est": stats([item["est_sample_count"] for item in items]),
            "gt_speed_mps": stats([item["gt_speed_mps"] for item in items]),
            "est_speed_mps": stats([item["est_speed_mps"] for item in items]),
            "gt_fit_rmse_m": stats([item["gt_fit_rmse_m"] for item in items]),
            "est_fit_rmse_m": stats([item["est_fit_rmse_m"] for item in items]),
        }

    primary = [item for item in records if item["half_window_s"] == 1.0 and
               item["speed_threshold_mps"] == 0.10]
    return {
        "definition": "TRAJECTORY COURSE-DIRECTION ERROR; heading/course divergence only",
        "primary": report(primary, 1.0, 0.10),
        "robustness": [report([item for item in records
                                if item["half_window_s"] == half and
                                item["speed_threshold_mps"] == threshold], half, threshold)
                       for half in WINDOWS for threshold in SPEED_THRESHOLDS],
        "records": records,
        "gt_attitude_claimed": "MUST BE NO",
    }


def course_by_window(records, half=1.0, threshold=0.10):
    return [item for item in records if item["half_window_s"] == half and
            item["speed_threshold_mps"] == threshold]


def timeline(rows, course_records_primary, gate):
    if not gate["pass"]:
        return {"classification": "NOT_AUTHORIZED_BY_YAW_GATE", "authorized": False}
    start, end = ONSET
    course_by_frame = {item["frame"]: item for item in course_records_primary}
    enriched = []
    for row in rows:
        item = dict(row)
        course = course_by_frame.get(integer(row, "frame"))
        item["course_abs_error_deg"] = (course["abs_course_error_deg"]
                                         if course else math.nan)
        enriched.append(item)
    windows = onset_windows(enriched)
    metrics = ("O_yaw", "O_long", "O_lat", "dcreg_schur_kappa_R",
               "Psi_weak_R", "A_weak_R", "C_yaw_L", "C_yaw_F",
               "weak_chi_max_R", "weak_psi_max_R", "C_L", "G_per_used",
               "course_abs_error_deg", "translation_error_m")
    stages = {}
    for name in ("pre5", "pre3", "pre1", "onset"):
        stages[name] = {metric: stats([number(row, metric) for row in windows[name]])
                        for metric in metrics}
    # Resolution-aware ordering: report a causal order only if every event is
    # separated from the pre-5-s distribution and has a distinct stage.
    baseline = windows["pre5"]
    def first_rise(metric, threshold=None):
        base = finite([number(row, metric) for row in baseline])
        if not base:
            return None
        limit = pct(base, 0.95)
        if threshold is not None:
            limit = max(limit, threshold)
        for name in ("pre5", "pre3", "pre1", "onset"):
            values = finite([number(row, metric) for row in windows[name]])
            if values and statistics.median(values) > limit:
                return name
        return None
    events = {"forcing": first_rise("Psi_weak_R"),
              "course": first_rise("course_abs_error_deg", 5.0),
              "position": first_rise("translation_error_m")}
    order = [value for value in (events["forcing"], events["course"], events["position"])
             if value is not None]
    if not order:
        classification = "NO_EVENT"
    elif order == sorted(order, key=("pre5", "pre3", "pre1", "onset").index) and \
            len(set(order)) == 3:
        classification = "CORRECT_ORDER"
    elif len(set(order)) < len(order):
        classification = "SIMULTANEOUS_WITHIN_RESOLUTION"
    else:
        classification = "WRONG_ORDER"
    return {"classification": classification, "authorized": True,
            "stages": stages, "event_stage": events,
            "course_definition": "TRAJECTORY COURSE-DIRECTION ERROR only"}


def stairs_orientation(rows, local):
    by_frame = {item["frame"]: item for item in local}
    result = {}
    for delta in (1.0, 5.0, 10.0):
        target = [by_frame.get(integer(row, "frame"), {}).get(
            f"rotation_error_{int(delta)}s_deg", math.nan) for row in rows]
        result[f"{int(delta)}s"] = {
            "rotation_error_deg": stats(target),
            "by_O_yaw": stats([number(row, "O_yaw") for row, value in zip(rows, target)
                                if math.isfinite(value)]),
            "by_Psi_weak_R": stats([number(row, "Psi_weak_R") for row, value in zip(rows, target)
                                     if math.isfinite(value)]),
            "by_weak_chi": stats([number(row, "weak_chi_max_R") for row, value in zip(rows, target)
                                   if math.isfinite(value)]),
            "by_kappa_R": stats([number(row, "dcreg_schur_kappa_R") for row, value in zip(rows, target)
                                  if math.isfinite(value)]),
        }
    return {"same_orientation_semantics": True, "windows": result}


def analyze_scene(name, estimate_path, gt_path, axis_path):
    est_time, est_pos, est_quat = load_tum(estimate_path)
    gt_time, gt_pos, gt_quat = load_tum(gt_path, allow_unsorted=True)
    pairs = associate(est_time, gt_time, MAX_DIFF)
    if len(pairs) < 3:
        raise ValueError(f"{name}: insufficient trajectory matches: {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    alignment_rotation, alignment_translation = umeyama_se3(est_pos[ei], gt_pos[gi])
    axis_rows = load_axis(axis_path)
    gt_attitude = all(valid_quaternion(gt_quat[index]) for index in gi)
    local = local_errors(axis_rows, est_time, est_pos, gt_time, gt_pos, est_quat,
                         gt_quat, alignment_rotation, alignment_translation,
                         gt_attitude)
    records = []
    for half in WINDOWS:
        for threshold in SPEED_THRESHOLDS:
            for item in course_records(axis_rows, est_time, est_pos, gt_time, gt_pos,
                                       alignment_rotation, half, threshold):
                item["half_window_s"] = half
                item["speed_threshold_mps"] = threshold
                records.append(item)
    primary_records = course_by_window(records)
    invariant_errors = [item["invariant_error"] for item in records]
    max_invariant_error = max(invariant_errors, default=0.0)
    if max_invariant_error > 1.0e-5:
        raise ValueError("PROMPT06_AXIS_DECOMPOSITION_INVARIANT_FAILURE: "
                         f"max_error={max_invariant_error}")
    offline_axis_rows = add_offline_axis_fields(axis_rows, primary_records)
    merged = merge_row_metrics(offline_axis_rows, local)
    physical = {"full": physical_summary(merged)}
    if name == "tunnel2":
        windows = onset_windows(merged)
        physical.update({key: physical_summary(value) for key, value in windows.items()
                         if key != "full"})
    gate = yaw_gate(merged) if name == "tunnel2" else {
        "pass": False, "marker": "pre-declared onset is Tunnel2-only"}
    return {
        "sequence": name,
        "estimate": str(pathlib.Path(estimate_path).resolve()),
        "ground_truth": str(pathlib.Path(gt_path).resolve()),
        "axis_csv": str(pathlib.Path(axis_path).resolve()),
        "association": {"max_diff_s": MAX_DIFF, "matched": len(pairs),
                         "global_alignment": "single SE3, no per-window realignment",
                         "alignment_rotation": alignment_rotation.tolist(),
                         "alignment_translation": alignment_translation.tolist()},
        "ground_truth_attitude": {
            "available_for_all_matches": gt_attitude,
            "matched_with_valid_attitude": int(sum(valid_quaternion(gt_quat[index]) for index in gi)),
            "tunnel2_attitude_claim": "MUST BE NO" if name == "tunnel2" else "official reference available"},
        "frame_authority": {"schema_version": 5, "rows": len(axis_rows),
                             "valid_rows": sum(integer(row, "valid") == 1 for row in axis_rows),
                             "first_native_measurement_iteration_only": True,
                             "R_pred_and_gravity": "read-only before native UpdateObserve",
                             "course_axes": "offline only; not estimator CSV fields"},
        "physical_axis": physical,
        "yaw_gate": gate,
        "forcing_robustness": {
            "full": forcing_summary(merged),
            "pre5": forcing_summary(interval(merged, ONSET[0] - 5.0, ONSET[0])),
            "pre3": forcing_summary(interval(merged, ONSET[0] - 3.0, ONSET[0])),
            "pre1": forcing_summary(interval(merged, ONSET[0] - 1.0, ONSET[0])),
            "onset": forcing_summary(interval(merged, ONSET[0], ONSET[1])),
        },
        "course_proxy": course_report(records, axis_rows),
        "axis_decomposition_invariant": {
            "max_abs_sum_minus_rank": max_invariant_error,
            "records": len(invariant_errors), "tolerance": 1.0e-5,
            "marker_on_failure": "PROMPT06_AXIS_DECOMPOSITION_INVARIANT_FAILURE"},
        "causal_timeline": timeline(merged, primary_records, gate),
        "stairs_orientation_control": stairs_orientation(merged, local) if name == "stairs" else None,
        "rows": merged,
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
                        metavar=("NAME", "ESTIMATE", "GT", "AXIS_CSV"))
    parser.add_argument("--out-dir", type=pathlib.Path, required=True)
    parser.add_argument("--combined-out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    try:
        args.out_dir.mkdir(parents=True, exist_ok=True)
        reports = {}
        for name, estimate, gt, axis in args.scene:
            report = analyze_scene(name, pathlib.Path(estimate), pathlib.Path(gt),
                                   pathlib.Path(axis))
            reports[name] = report
            (args.out_dir / f"{name}.json").write_text(
                json.dumps(clean(report), indent=2, sort_keys=True) + "\n",
                encoding="utf-8")
        combined = {"analysis": "Prompt06 physical weak-axis and course-causality audit; evaluation only",
                    "onset": ONSET, "axis_gate": AXIS_GATE,
                    "scenes": {name: {"association": report["association"],
                                      "ground_truth_attitude": report["ground_truth_attitude"],
                                      "frame_authority": report["frame_authority"],
                                      "physical_axis": report["physical_axis"],
                                      "yaw_gate": report["yaw_gate"],
                                      "forcing_robustness": report["forcing_robustness"],
                                      "course_proxy": {key: value for key, value in report["course_proxy"].items()
                                                       if key != "records"},
                                      "causal_timeline": report["causal_timeline"],
                                      "stairs_orientation_control": report["stairs_orientation_control"]}
                             for name, report in reports.items()}}
        args.combined_out.parent.mkdir(parents=True, exist_ok=True)
        args.combined_out.write_text(json.dumps(clean(combined), indent=2, sort_keys=True) + "\n",
                                     encoding="utf-8")
        print(f"PROMPT06_ANALYSIS_PASS scenes={','.join(sorted(reports))}")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"PROMPT06_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
