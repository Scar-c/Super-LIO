#!/usr/bin/env python3
"""Prompt16 fixed-alignment APE/RPE/segment evaluator."""

import argparse
import csv
import math
import pathlib

import numpy as np


GT_TOLERANCE = 0.10
PAIR_TOLERANCE = 0.05
TIME_HORIZONS = (1.0, 5.0, 10.0, 20.0)
DISTANCE_HORIZONS = (5.0, 10.0, 20.0)


def load_tum(path):
    rows = []
    for number, line in enumerate(pathlib.Path(path).read_text().splitlines(), 1):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        values = line.split()
        if len(values) < 8:
            raise ValueError(f"{path}:{number}: expected 8 columns")
        rows.append([float(value) for value in values[:8]])
    if not rows:
        raise ValueError(f"{path}: no TUM rows")
    data = np.asarray(rows, dtype=float)
    order = np.argsort(data[:, 0], kind="stable")
    return data[order, 0], data[order, 1:4], data[order, 4:8]


def nearest_index(times, stamp, tolerance):
    index = int(np.searchsorted(times, stamp))
    candidates = [i for i in (index - 1, index) if 0 <= i < len(times)]
    if not candidates:
        return None
    best = min(candidates, key=lambda i: abs(float(times[i] - stamp)))
    return best if abs(float(times[best] - stamp)) <= tolerance else None


def pairs(est_times, gt_times, tolerance=GT_TOLERANCE):
    candidates = []
    for est_index, stamp in enumerate(est_times):
        lo = int(np.searchsorted(gt_times, stamp - tolerance, side="left"))
        hi = int(np.searchsorted(gt_times, stamp + tolerance, side="right"))
        candidates.extend(
            (abs(float(stamp - gt_times[gt_index])), est_index, gt_index)
            for gt_index in range(lo, hi)
        )
    used_est, used_gt, result = set(), set(), []
    for _, est_index, gt_index in sorted(candidates):
        if est_index in used_est or gt_index in used_gt:
            continue
        used_est.add(est_index)
        used_gt.add(gt_index)
        result.append((est_index, gt_index))
    return sorted(result)


def umeyama(source, destination):
    source_mean = source.mean(axis=0)
    destination_mean = destination.mean(axis=0)
    covariance = (source - source_mean).T @ (destination - destination_mean)
    u, _, vt = np.linalg.svd(covariance)
    correction = np.diag([1.0, 1.0, np.sign(np.linalg.det(vt.T @ u.T))])
    rotation = vt.T @ correction @ u.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


def interpolate(stamp, times, positions):
    if stamp < times[0] or stamp > times[-1]:
        return None
    index = int(np.searchsorted(times, stamp))
    if index == 0:
        return positions[0]
    if index >= len(times):
        return positions[-1]
    left, right = index - 1, index
    span = times[right] - times[left]
    if span <= 0.0:
        return positions[left]
    alpha = (stamp - times[left]) / span
    return (1.0 - alpha) * positions[left] + alpha * positions[right]


def aligned(positions, alignment):
    rotation, translation = alignment
    return (rotation @ positions.T).T + translation


def ape_metrics(est_times, est_positions, gt_times, gt_positions, alignment):
    matched = pairs(est_times, gt_times)
    if not matched:
        return {"matches": 0}
    source = np.asarray([est_positions[i] for i, _ in matched])
    destination = np.asarray([gt_positions[j] for _, j in matched])
    errors = np.linalg.norm(aligned(source, alignment) - destination, axis=1)
    return {
        "matches": len(matched),
        "ape_rmse_m": float(np.sqrt(np.mean(errors * errors))),
        "ape_median_m": float(np.median(errors)),
        "ape_p95_m": float(np.percentile(errors, 95)),
        "max_local_error_m": float(np.max(errors)),
        "endpoint_error_m": float(errors[-1]),
        "first_major_divergence_s": float(
            est_times[matched[next(i for i, error in enumerate(errors) if error > 1.0)][0]]
        ) if np.any(errors > 1.0) else float("nan"),
    }


def rpe_metrics(est_times, est_positions, gt_times, gt_positions):
    result = {}
    for horizon in TIME_HORIZONS:
        errors = []
        for start_index, start_time in enumerate(est_times):
            end_index = nearest_index(est_times, start_time + horizon, PAIR_TOLERANCE)
            if end_index is None or end_index <= start_index:
                continue
            gt_start = interpolate(start_time, gt_times, gt_positions)
            gt_end = interpolate(float(est_times[end_index]), gt_times, gt_positions)
            if gt_start is None or gt_end is None:
                continue
            errors.append(np.linalg.norm(
                (est_positions[end_index] - est_positions[start_index]) -
                (gt_end - gt_start)))
        result[f"rpe_{int(horizon)}s_m"] = float(np.median(errors)) if errors else float("nan")
        result[f"rpe_{int(horizon)}s_p95_m"] = float(np.percentile(errors, 95)) if errors else float("nan")
    return result


def distance_metrics(est_times, est_positions, gt_times, gt_positions):
    result = {}
    cumulative = np.concatenate(([0.0], np.cumsum(np.linalg.norm(np.diff(est_positions, axis=0), axis=1))))
    for horizon in DISTANCE_HORIZONS:
        errors = []
        for start_index, start_distance in enumerate(cumulative):
            target = start_distance + horizon
            end_index = int(np.searchsorted(cumulative, target))
            if end_index >= len(est_times):
                continue
            gt_start = interpolate(float(est_times[start_index]), gt_times, gt_positions)
            gt_end = interpolate(float(est_times[end_index]), gt_times, gt_positions)
            if gt_start is None or gt_end is None:
                continue
            errors.append(np.linalg.norm(
                (est_positions[end_index] - est_positions[start_index]) -
                (gt_end - gt_start)))
        result[f"segment_{int(horizon)}m_m"] = float(np.median(errors)) if errors else float("nan")
        result[f"segment_{int(horizon)}m_p95_m"] = float(np.percentile(errors, 95)) if errors else float("nan")
    return result


def evaluate(sequence, branch, estimate_path, gt_path, alignment):
    est_times, est_positions, _ = load_tum(estimate_path)
    gt_times, gt_positions, _ = load_tum(gt_path)
    overlap_start = max(est_times[0], gt_times[0])
    overlap_end = min(est_times[-1], gt_times[-1])
    completion = max(0.0, overlap_end - overlap_start) / max(1.0e-12, gt_times[-1] - gt_times[0])
    row = {"sequence": sequence, "branch": branch, "rows": len(est_times),
           "completion": completion, "estimate_path": str(estimate_path)}
    row.update(ape_metrics(est_times, est_positions, gt_times, gt_positions, alignment))
    row.update(rpe_metrics(est_times, est_positions, gt_times, gt_positions))
    row.update(distance_metrics(est_times, est_positions, gt_times, gt_positions))
    return row


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--ground-truth", required=True, type=pathlib.Path)
    parser.add_argument("--native", required=True, type=pathlib.Path)
    parser.add_argument("--asymmetric", required=True, type=pathlib.Path)
    parser.add_argument("--out", required=True, type=pathlib.Path)
    args = parser.parse_args(argv)

    native_times, native_positions, _ = load_tum(args.native)
    gt_times, gt_positions, _ = load_tum(args.ground_truth)
    native_pairs = pairs(native_times, gt_times)
    if len(native_pairs) < 3:
        raise ValueError("native trajectory has fewer than three GT matches")
    alignment = umeyama(
        np.asarray([native_positions[i] for i, _ in native_pairs]),
        np.asarray([gt_positions[j] for _, j in native_pairs]),
    )
    rows = [
        evaluate(args.sequence, "native", args.native, args.ground_truth, alignment),
        evaluate(args.sequence, "asymmetric", args.asymmetric, args.ground_truth, alignment),
    ]
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=sorted(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"sequence={args.sequence} native_matches={len(native_pairs)} output={args.out}")
    for row in rows:
        print(row["branch"], "ape_rmse_m=", row.get("ape_rmse_m"),
              "ape_median_m=", row.get("ape_median_m"),
              "rpe_10s_m=", row.get("rpe_10s_m"))


if __name__ == "__main__":
    main()
