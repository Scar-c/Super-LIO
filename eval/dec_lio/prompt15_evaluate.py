#!/usr/bin/env python3
"""Evaluate one-event Prompt15 branch replays under one fixed SE(3) alignment."""

import argparse
import csv
import math
import pathlib
import sys

import numpy as np


TIME_HORIZONS = (1.0, 5.0, 10.0, 20.0)
DISTANCE_HORIZONS = (5.0, 10.0, 20.0)
EPS = 1.0e-3
GT_TOLERANCE = 0.10


def load_tum(path, allow_unsorted=False):
    rows = []
    with pathlib.Path(path).open(encoding="utf-8") as stream:
        for number, line in enumerate(stream, 1):
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            values = line.split()
            if len(values) < 8:
                raise ValueError(f"{path}:{number}: expected 8 columns")
            rows.append([float(value) for value in values[:8]])
    if not rows:
        raise ValueError(f"{path}: no TUM rows")
    data = np.asarray(rows, dtype=float)
    if np.any(np.diff(data[:, 0]) <= 0.0) and not allow_unsorted:
        raise ValueError(f"{path}: timestamps are not strictly increasing")
    if allow_unsorted:
        order = np.argsort(data[:, 0], kind="stable")
        data = data[order]
    return data[:, 0], data[:, 1:4], data[:, 4:8]


def load_gt(path):
    times, positions, quaternions = load_tum(path, allow_unsorted=True)
    # Prompt12 established that bridge01 is the only non-monotonic supplied GT.
    # The primary Prompt15 GT files are monotonic, but stable ordering is cheap.
    order = np.argsort(times, kind="stable")
    return times[order], positions[order], quaternions[order]


def nearest_index(times, stamp, tolerance=0.05):
    index = int(np.searchsorted(times, stamp))
    candidates = [i for i in (index - 1, index) if 0 <= i < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda i: abs(float(times[i] - stamp)))
    return best if abs(float(times[best] - stamp)) <= tolerance else None


def associate_estimate_to_gt(est_times, gt_times, tolerance=GT_TOLERANCE):
    pairs = []
    used = set()
    for ei, stamp in enumerate(est_times):
        gi = nearest_index(gt_times, stamp, tolerance)
        if gi is not None and gi not in used:
            pairs.append((ei, gi))
            used.add(gi)
    return pairs


def umeyama_se3(source, destination):
    source_mean = source.mean(axis=0)
    destination_mean = destination.mean(axis=0)
    covariance = (source - source_mean).T @ (destination - destination_mean)
    u, _, vt = np.linalg.svd(covariance)
    correction = np.diag([1.0, 1.0, np.sign(np.linalg.det(vt.T @ u.T))])
    rotation = vt.T @ correction @ u.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


def quaternion_normalize(q):
    norm = np.linalg.norm(q)
    if not np.isfinite(norm) or norm <= 1.0e-12:
        return None
    return q / norm


def quaternion_multiply(lhs, rhs):
    x1, y1, z1, w1 = lhs
    x2, y2, z2, w2 = rhs
    return np.asarray([
        w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
        w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2,
        w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2,
        w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2,
    ])


def quaternion_inverse(q):
    return np.asarray([-q[0], -q[1], -q[2], q[3]]) / np.dot(q, q)


def quaternion_angle(lhs, rhs):
    lhs = quaternion_normalize(lhs)
    rhs = quaternion_normalize(rhs)
    if lhs is None or rhs is None:
        return float("nan")
    dot = abs(float(np.dot(lhs, rhs)))
    return 2.0 * math.acos(min(1.0, max(-1.0, dot)))


def relative_rotation_error(q_start, q_end, gt_start, gt_end):
    q_est = quaternion_multiply(quaternion_inverse(q_start), q_end)
    q_gt = quaternion_multiply(quaternion_inverse(gt_start), gt_end)
    if quaternion_normalize(gt_start) is None or quaternion_normalize(gt_end) is None:
        return float("nan")
    return quaternion_angle(q_est, q_gt)


def read_observation_stage(path):
    if not pathlib.Path(path).is_file():
        return None
    result = []
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            try:
                result.append((float(row["timestamp"]), int(row["N_used"])))
            except (KeyError, ValueError):
                continue
    return result


def nearest_stage(stage, stamp):
    if not stage:
        return "nan"
    timestamp, used = min(stage, key=lambda item: abs(item[0] - stamp))
    return str(used) if abs(timestamp - stamp) <= 0.05 else "nan"


def parse_assignments(values, label):
    result = {}
    for value in values:
        parts = value.split("=", 1)
        if len(parts) != 2:
            raise ValueError(f"{label} must be NAME=PATH: {value}")
        result[parts[0]] = pathlib.Path(parts[1])
    return result


def parse_interventions(values):
    result = {}
    for value in values:
        parts = value.split("=", 2)
        if len(parts) != 3:
            raise ValueError("--intervention must be SEQUENCE=RANK=PATH")
        result[(parts[0], int(parts[1]))] = pathlib.Path(parts[2])
    return result


def fixed_alignment(native, gt):
    pairs = associate_estimate_to_gt(native[0], gt[0])
    if len(pairs) < 3:
        raise ValueError(f"insufficient full-trajectory GT matches: {len(pairs)}")
    source = np.asarray([native[1][ei] for ei, _ in pairs])
    destination = np.asarray([gt[1][gi] for _, gi in pairs])
    return umeyama_se3(source, destination), pairs


def gt_at(stamp, gt, tolerance=GT_TOLERANCE):
    index = nearest_index(gt[0], stamp, tolerance)
    return None if index is None else index


def interpolate_gt_position(stamp, gt):
    if stamp < gt[0][0] or stamp > gt[0][-1]:
        return None
    index = int(np.searchsorted(gt[0], stamp))
    if index == 0:
        return gt[1][0]
    if index >= len(gt[0]):
        return gt[1][-1]
    left, right = index - 1, index
    span = gt[0][right] - gt[0][left]
    if span <= 0.0:
        return gt[1][left]
    alpha = (stamp - gt[0][left]) / span
    return (1.0 - alpha) * gt[1][left] + alpha * gt[1][right]


def make_pairs(native, intervention):
    pairs = []
    for ni, stamp in enumerate(intervention[0]):
        nj = nearest_index(native[0], stamp)
        if nj is not None:
            pairs.append((nj, ni))
    return pairs


def aligned(position, alignment):
    rotation, translation = alignment
    return rotation @ position + translation


def orientation_valid(quaternions):
    return np.all(np.linalg.norm(quaternions, axis=1) > 1.0e-12)


def horizon_row(sequence, rank, event_time, horizon, native, intervention, gt,
                alignment, native_start, intervention_start):
    target_time = event_time + horizon
    native_end = nearest_index(native[0], target_time)
    intervention_end = nearest_index(intervention[0], target_time)
    if native_end is None or intervention_end is None:
        return {"available": 0, "sequence": sequence, "selection_rank": rank,
                "horizon_s": horizon}
    # The diagnostic timestamp is LiDAR scan-end time and tunnel1 GT is sampled
    # on a slightly different clock. Use one declared 100 ms GT contract for
    # the event anchor and every future sample; branch time pairing stays 50 ms.
    gt_start = gt_at(event_time, gt, GT_TOLERANCE)
    gt_end = gt_at(float(intervention[0][intervention_end]), gt)
    gt_start_position = interpolate_gt_position(event_time, gt)
    gt_end_position = interpolate_gt_position(
        float(intervention[0][intervention_end]), gt)
    if gt_start is None or gt_start_position is None or gt_end_position is None:
        return {"available": 0, "sequence": sequence, "selection_rank": rank,
                "horizon_s": horizon}
    gt_segment = gt_end_position - gt_start_position
    native_segment = aligned(native[1][native_end], alignment) - aligned(
        native[1][native_start], alignment)
    intervention_segment = aligned(intervention[1][intervention_end], alignment) - aligned(
        intervention[1][intervention_start], alignment)
    gt_position = gt_end_position
    native_endpoint_error = np.linalg.norm(
        aligned(native[1][native_end], alignment) - gt_position)
    intervention_endpoint_error = np.linalg.norm(
        aligned(intervention[1][intervention_end], alignment) - gt_position)
    native_interval = (native[0] >= event_time) & (native[0] <= target_time)
    intervention_interval = (intervention[0] >= event_time) & (
        intervention[0] <= target_time)
    native_errors = []
    intervention_errors = []
    for stamp, position in zip(native[0][native_interval], native[1][native_interval]):
        gi = gt_at(float(stamp), gt)
        if gi is not None:
            native_errors.append(np.linalg.norm(aligned(position, alignment) - gt[1][gi]))
    for stamp, position in zip(intervention[0][intervention_interval],
                               intervention[1][intervention_interval]):
        gi = gt_at(float(stamp), gt)
        if gi is not None:
            intervention_errors.append(np.linalg.norm(aligned(position, alignment) - gt[1][gi]))
    row = {
        "sequence": sequence,
        "selection_rank": rank,
        "horizon_s": horizon,
        "available": 1,
        "native_rpe_m": np.linalg.norm(native_segment - gt_segment),
        "intervention_rpe_m": np.linalg.norm(intervention_segment - gt_segment),
        "native_endpoint_error_m": native_endpoint_error,
        "intervention_endpoint_error_m": intervention_endpoint_error,
        "native_suffix_ate_m": np.mean(native_errors) if native_errors else float("nan"),
        "intervention_suffix_ate_m": (np.mean(intervention_errors)
                                      if intervention_errors else float("nan")),
        "native_end_time": native[0][native_end],
        "intervention_end_time": intervention[0][intervention_end],
        "native_rot_rpe_rad": float("nan"),
        "intervention_rot_rpe_rad": float("nan"),
    }
    if gt_end is not None and orientation_valid(gt[2][gt_start:gt_end + 1]) and orientation_valid(
            native[2][native_start:native_end + 1]):
        row["native_rot_rpe_rad"] = relative_rotation_error(
            native[2][native_start], native[2][native_end],
            gt[2][gt_start], gt[2][gt_end])
    if gt_end is not None and orientation_valid(gt[2][gt_start:gt_end + 1]) and orientation_valid(
            intervention[2][intervention_start:intervention_end + 1]):
        row["intervention_rot_rpe_rad"] = relative_rotation_error(
            intervention[2][intervention_start], intervention[2][intervention_end],
            gt[2][gt_start], gt[2][gt_end])
    row["delta_suffix_ate_m"] = (row["intervention_suffix_ate_m"] -
                                  row["native_suffix_ate_m"])
    row["delta_rpe_m"] = row["intervention_rpe_m"] - row["native_rpe_m"]
    return row


def distance_rows(sequence, rank, event_time, native, intervention, gt, alignment,
                  native_start, intervention_start):
    rows = []
    gt_start = gt_at(event_time, gt, GT_TOLERANCE)
    if gt_start is None:
        return rows
    for distance in DISTANCE_HORIZONS:
        cumulative = 0.0
        previous = gt_start
        gt_end = None
        for index in range(gt_start + 1, len(gt[0])):
            cumulative += np.linalg.norm(gt[1][index] - gt[1][previous])
            previous = index
            if cumulative >= distance:
                gt_end = index
                break
        if gt_end is None:
            continue
        target_time = gt[0][gt_end]
        ni = nearest_index(native[0], target_time)
        ii = nearest_index(intervention[0], target_time)
        if ni is None or ii is None:
            continue
        native_segment = aligned(native[1][ni], alignment) - aligned(
            native[1][native_start], alignment)
        intervention_segment = aligned(intervention[1][ii], alignment) - aligned(
            intervention[1][intervention_start], alignment)
        gt_segment = gt[1][gt_end] - gt[1][gt_start]
        rows.append({
            "sequence": sequence,
            "selection_rank": rank,
            "horizon_m": distance,
            "gt_arc_m": cumulative,
            "native_relative_error_m": np.linalg.norm(native_segment - gt_segment),
            "intervention_relative_error_m": np.linalg.norm(intervention_segment - gt_segment),
            "delta_relative_error_m": np.linalg.norm(intervention_segment - gt_segment) -
            np.linalg.norm(native_segment - gt_segment),
            "native_endpoint_error_m": np.linalg.norm(aligned(native[1][ni], alignment) - gt[1][gt_end]),
            "intervention_endpoint_error_m": np.linalg.norm(aligned(intervention[1][ii], alignment) - gt[1][gt_end]),
        })
    return rows


def classify(curve, horizons):
    available = [row for row in horizons if int(row.get("available", 0)) and
                 math.isfinite(float(row.get("delta_suffix_ate_m", "nan")))]
    long_rows = [row for row in available if float(row["horizon_s"]) >= 5.0]
    if len(long_rows) < 2:
        return "UNAVAILABLE"
    deltas = [float(row["delta_suffix_ate_m"]) for row in long_rows]
    if len(deltas) >= 2 and all(delta <= -EPS for delta in deltas):
        return "PERSISTENT_BENEFIT"
    if len(deltas) >= 2 and all(delta >= EPS for delta in deltas):
        return "PERSISTENT_HARM"
    if len(curve) >= 2:
        first = curve[0]["separation_translation_m"]
        last = curve[-1]["separation_translation_m"]
        peak = max(row["separation_translation_m"] for row in curve)
        if math.isfinite(first) and math.isfinite(last) and last <= 0.5 * max(first, EPS):
            return "RECOVERS"
        if math.isfinite(last) and math.isfinite(peak) and last >= 0.75 * peak and last > EPS:
            return "AMPLIFIES"
    return "MIXED"


def write_csv(path, rows, fields):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, extrasaction="ignore",
                                lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--selection", required=True)
    parser.add_argument("--native", action="append", required=True,
                        metavar="SEQUENCE=PATH")
    parser.add_argument("--native-replay-b", action="append", required=True,
                        metavar="SEQUENCE=PATH")
    parser.add_argument("--intervention", action="append", required=True,
                        metavar="SEQUENCE=RANK=PATH")
    parser.add_argument("--ground-truth", action="append", required=True,
                        metavar="SEQUENCE=PATH")
    parser.add_argument("--output-dir", required=True)
    args = parser.parse_args(argv)
    try:
        native_paths = parse_assignments(args.native, "--native")
        native_b_paths = parse_assignments(args.native_replay_b, "--native-replay-b")
        intervention_paths = parse_interventions(args.intervention)
        gt_paths = parse_assignments(args.ground_truth, "--ground-truth")
        selection_rows = list(csv.DictReader(pathlib.Path(args.selection).open(
            newline="", encoding="utf-8")))
        if not selection_rows:
            raise ValueError("selection CSV is empty")
        out = pathlib.Path(args.output_dir)
        native = {}
        native_b = {}
        native_lines = {}
        native_b_lines = {}
        gt = {}
        alignment = {}
        native_stage = {}
        for sequence, path in native_paths.items():
            native[sequence] = load_tum(path / "trajectory.tum")
            native_b[sequence] = load_tum(native_b_paths[sequence] / "trajectory.tum")
            native_lines[sequence] = pathlib.Path(path / "trajectory.tum").read_bytes().splitlines()
            native_b_lines[sequence] = pathlib.Path(
                native_b_paths[sequence] / "trajectory.tum").read_bytes().splitlines()
            if native[sequence][0].shape != native_b[sequence][0].shape or \
                    native_lines[sequence] != native_b_lines[sequence]:
                raise ValueError(f"native replay mismatch: {sequence}")
            gt[sequence] = load_gt(gt_paths[sequence])
            alignment[sequence], _ = fixed_alignment(native[sequence], gt[sequence])
            native_stage[sequence] = read_observation_stage(path / "observation_stage.csv")

        event_rows = []
        horizon_rows = []
        distance_rows_all = []
        curve_rows = []
        aggregate = []
        for selection in selection_rows:
            sequence = selection["sequence"]
            rank = int(selection["selection_rank"])
            event_time = float(selection["timestamp"])
            branch_path = intervention_paths[(sequence, rank)]
            intervention = load_tum(branch_path / "trajectory.tum")
            pair = make_pairs(native[sequence], intervention)
            native_start = nearest_index(native[sequence][0], event_time)
            intervention_start = nearest_index(intervention[0], event_time)
            if native_start is None or intervention_start is None:
                raise ValueError(f"event timestamp not in trajectories: {sequence} rank {rank}")
            intervention_lines = pathlib.Path(
                branch_path / "trajectory.tum").read_bytes().splitlines()
            if native_start != intervention_start or \
                    native_lines[sequence][:native_start] != intervention_lines[:intervention_start]:
                raise ValueError(
                    f"counterfactual prefix mismatch: {sequence} rank {rank}")
            event_curve = []
            stage_i = read_observation_stage(branch_path / "observation_stage.csv")
            for ni, ii in pair:
                stamp = float(intervention[0][ii])
                if stamp < event_time or stamp > event_time + max(TIME_HORIZONS):
                    continue
                gt_index = gt_at(stamp, gt[sequence])
                native_position = aligned(native[sequence][1][ni], alignment[sequence])
                intervention_position = aligned(intervention[1][ii], alignment[sequence])
                native_q = native[sequence][2][ni]
                intervention_q = intervention[2][ii]
                event_curve.append({
                    "sequence": sequence,
                    "selection_rank": rank,
                    "event_timestamp": event_time,
                    "timestamp": stamp,
                    "relative_time_s": stamp - event_time,
                    "native_x": native_position[0], "native_y": native_position[1],
                    "native_z": native_position[2],
                    "intervention_x": intervention_position[0],
                    "intervention_y": intervention_position[1],
                    "intervention_z": intervention_position[2],
                    "separation_translation_m": np.linalg.norm(
                        native_position - intervention_position),
                    "separation_rotation_rad": quaternion_angle(native_q, intervention_q),
                    "native_error_m": (np.linalg.norm(native_position - gt[sequence][1][gt_index])
                                       if gt_index is not None else float("nan")),
                    "intervention_error_m": (np.linalg.norm(intervention_position - gt[sequence][1][gt_index])
                                             if gt_index is not None else float("nan")),
                    "native_used": nearest_stage(native_stage[sequence], stamp),
                    "intervention_used": nearest_stage(stage_i, stamp),
                })
            curve_rows.extend(event_curve)
            event_horizons = [horizon_row(
                sequence, rank, event_time, horizon, native[sequence], intervention,
                gt[sequence], alignment[sequence], native_start, intervention_start)
                for horizon in TIME_HORIZONS]
            horizon_rows.extend(event_horizons)
            event_distances = distance_rows(
                sequence, rank, event_time, native[sequence], intervention,
                gt[sequence], alignment[sequence], native_start, intervention_start)
            distance_rows_all.extend(event_distances)
            classification = classify(event_curve, event_horizons)
            event_csv = branch_path / "prompt15_event.csv"
            event_meta = {}
            if event_csv.is_file():
                with event_csv.open(newline="", encoding="utf-8") as stream:
                    rows = list(csv.DictReader(stream))
                if rows:
                    event_meta = rows[-1]
            event_rows.append({
                **selection,
                "event_csv_applied": event_meta.get("applied", "0"),
                "event_csv_reason": event_meta.get("selection_reason", "MISSING"),
                "prefix_rows_byte_identical": native_start,
                "classification": classification,
                "curve_samples": len(event_curve),
                "max_separation_translation_m": max(
                    (row["separation_translation_m"] for row in event_curve), default=float("nan")),
                "final_separation_translation_m": (event_curve[-1]["separation_translation_m"]
                                                    if event_curve else float("nan")),
            })
            aggregate.append({"classification": classification})

        event_fields = list(selection_rows[0].keys()) + [
            "event_csv_applied", "event_csv_reason", "prefix_rows_byte_identical",
            "classification", "curve_samples",
            "max_separation_translation_m", "final_separation_translation_m"]
        horizon_fields = ["sequence", "selection_rank", "horizon_s", "available",
                          "native_rpe_m", "intervention_rpe_m", "delta_rpe_m",
                          "native_endpoint_error_m", "intervention_endpoint_error_m",
                          "native_suffix_ate_m", "intervention_suffix_ate_m",
                          "delta_suffix_ate_m", "native_rot_rpe_rad",
                          "intervention_rot_rpe_rad", "native_end_time",
                          "intervention_end_time"]
        distance_fields = ["sequence", "selection_rank", "horizon_m", "gt_arc_m",
                           "native_relative_error_m", "intervention_relative_error_m",
                           "delta_relative_error_m", "native_endpoint_error_m",
                           "intervention_endpoint_error_m"]
        curve_fields = ["sequence", "selection_rank", "event_timestamp", "timestamp",
                        "relative_time_s", "native_x", "native_y", "native_z",
                        "intervention_x", "intervention_y", "intervention_z",
                        "separation_translation_m", "separation_rotation_rad",
                        "native_error_m", "intervention_error_m", "native_used",
                        "intervention_used"]
        write_csv(out / "EVENT_RESULTS.csv", event_rows, event_fields)
        write_csv(out / "FUTURE_HORIZON_RESULTS.csv", horizon_rows, horizon_fields)
        write_csv(out / "DISTANCE_HORIZON_RESULTS.csv", distance_rows_all, distance_fields)
        write_csv(out / "BRANCH_SEPARATION_CURVES.csv", curve_rows, curve_fields)
        counts = {name: sum(row["classification"] == name for row in event_rows)
                  for name in ("PERSISTENT_BENEFIT", "PERSISTENT_HARM", "RECOVERS",
                               "AMPLIFIES", "MIXED", "UNAVAILABLE")}
        write_csv(out / "AGGREGATE.csv", [counts], list(counts.keys()))
        print(f"native_native_parity=PASS events={len(event_rows)} output={out}")
        print("aggregate=" + ",".join(f"{key}:{value}" for key, value in counts.items()))
    except (OSError, ValueError, KeyError, csv.Error, np.linalg.LinAlgError) as error:
        print(f"PROMPT15_EVALUATION_FAIL: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
