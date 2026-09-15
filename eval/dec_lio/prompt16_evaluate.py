#!/usr/bin/env python3
"""Prompt17 corrected APE/RPE/segment evaluator.

APE is reported after an independent Umeyama alignment for each branch.  The
``common_frame_*`` fields explicitly apply the native branch's alignment to
both branches.  RPE and distance-segment errors use the translation component
of the relative SE(3) error, so a constant global rotation between trajectories
does not create a spurious translational error.
"""

import argparse
import csv
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


def quaternion_matrix(quaternion):
    """Return a rotation matrix for a TUM ``x y z w`` quaternion."""
    x, y, z, w = np.asarray(quaternion, dtype=float)
    norm = np.linalg.norm([x, y, z, w])
    if not np.isfinite(norm) or norm < 1.0e-12:
        raise ValueError("invalid quaternion")
    x, y, z, w = np.asarray([x, y, z, w], dtype=float) / norm
    return np.array([
        [1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - z * w),
         2.0 * (x * z + y * w)],
        [2.0 * (x * y + z * w), 1.0 - 2.0 * (x * x + z * z),
         2.0 * (y * z - x * w)],
        [2.0 * (x * z - y * w), 2.0 * (y * z + x * w),
         1.0 - 2.0 * (x * x + y * y)],
    ])


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
    if len(source) < 3:
        raise ValueError("at least three matched poses are required")
    source_mean = source.mean(axis=0)
    destination_mean = destination.mean(axis=0)
    covariance = (source - source_mean).T @ (destination - destination_mean)
    u, _, vt = np.linalg.svd(covariance)
    correction = np.diag([1.0, 1.0, np.sign(np.linalg.det(vt.T @ u.T))])
    rotation = vt.T @ correction @ u.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


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
    first_major = np.flatnonzero(errors > 1.0)
    return {
        "matches": len(matched),
        "ape_rmse_m": float(np.sqrt(np.mean(errors * errors))),
        "ape_median_m": float(np.median(errors)),
        "ape_p95_m": float(np.percentile(errors, 95)),
        "max_local_error_m": float(np.max(errors)),
        "endpoint_error_m": float(errors[-1]),
        "first_major_divergence_s": (
            float(est_times[matched[int(first_major[0])][0]])
            if len(first_major) else float("nan")
        ),
    }


def relative_translation_error(est_start_position, est_start_quaternion,
                               est_end_position, est_end_quaternion,
                               gt_start_position, gt_start_quaternion,
                               gt_end_position, gt_end_quaternion):
    """Translation norm of ``(T_e0^-1 T_e1)^-1(T_g0^-1 T_g1)``."""
    est_start_rotation = quaternion_matrix(est_start_quaternion)
    est_end_rotation = quaternion_matrix(est_end_quaternion)
    gt_start_rotation = quaternion_matrix(gt_start_quaternion)
    gt_end_rotation = quaternion_matrix(gt_end_quaternion)
    est_relative_rotation = est_start_rotation.T @ est_end_rotation
    est_relative_translation = est_start_rotation.T @ (
        np.asarray(est_end_position) - np.asarray(est_start_position))
    gt_relative_rotation = gt_start_rotation.T @ gt_end_rotation
    gt_relative_translation = gt_start_rotation.T @ (
        np.asarray(gt_end_position) - np.asarray(gt_start_position))
    error_translation = est_relative_rotation.T @ (
        gt_relative_translation - est_relative_translation)
    # Keep the full relative rotations in the implementation above: the
    # reported quantity is specifically the translation component of the
    # standard SE(3) relative-pose error.
    _ = gt_relative_rotation
    return float(np.linalg.norm(error_translation))


def _relative_error_for_indices(est_positions, est_quaternions, est_start,
                                est_end, gt_positions, gt_quaternions,
                                gt_start, gt_end):
    try:
        return relative_translation_error(
            est_positions[est_start], est_quaternions[est_start],
            est_positions[est_end], est_quaternions[est_end],
            gt_positions[gt_start], gt_quaternions[gt_start],
            gt_positions[gt_end], gt_quaternions[gt_end])
    except ValueError:
        return None


def rpe_metrics(est_times, est_positions, est_quaternions, gt_times,
                gt_positions, gt_quaternions):
    result = {}
    for horizon in TIME_HORIZONS:
        errors = []
        for start_index, start_time in enumerate(est_times):
            end_index = nearest_index(est_times, start_time + horizon,
                                      PAIR_TOLERANCE)
            if end_index is None or end_index <= start_index:
                continue
            gt_start = nearest_index(gt_times, float(start_time),
                                     GT_TOLERANCE)
            gt_end = nearest_index(gt_times, float(est_times[end_index]),
                                   GT_TOLERANCE)
            if gt_start is None or gt_end is None:
                continue
            error = _relative_error_for_indices(
                est_positions, est_quaternions, start_index, end_index,
                gt_positions, gt_quaternions, gt_start, gt_end)
            if error is not None:
                errors.append(error)
        result[f"rpe_{int(horizon)}s_m"] = (
            float(np.median(errors)) if errors else float("nan"))
        result[f"rpe_{int(horizon)}s_p95_m"] = (
            float(np.percentile(errors, 95)) if errors else float("nan"))
        result[f"rpe_{int(horizon)}s_matches"] = len(errors)
    return result


def distance_metrics(est_times, est_positions, est_quaternions, gt_times,
                      gt_positions, gt_quaternions):
    """Evaluate relative errors at GT distance horizons, not estimate distance."""
    result = {}
    gt_cumulative = np.concatenate((
        [0.0], np.cumsum(np.linalg.norm(np.diff(gt_positions, axis=0), axis=1))))
    for horizon in DISTANCE_HORIZONS:
        errors = []
        for gt_start, start_distance in enumerate(gt_cumulative):
            gt_end = int(np.searchsorted(gt_cumulative,
                                         start_distance + horizon))
            if gt_end >= len(gt_times):
                continue
            est_start = nearest_index(est_times, float(gt_times[gt_start]),
                                      GT_TOLERANCE)
            est_end = nearest_index(est_times, float(gt_times[gt_end]),
                                    GT_TOLERANCE)
            if (est_start is None or est_end is None or
                    est_end <= est_start):
                continue
            error = _relative_error_for_indices(
                est_positions, est_quaternions, est_start, est_end,
                gt_positions, gt_quaternions, gt_start, gt_end)
            if error is not None:
                errors.append(error)
        result[f"segment_{int(horizon)}m_m"] = (
            float(np.median(errors)) if errors else float("nan"))
        result[f"segment_{int(horizon)}m_p95_m"] = (
            float(np.percentile(errors, 95)) if errors else float("nan"))
        result[f"segment_{int(horizon)}m_matches"] = len(errors)
    return result


def evaluate(sequence, branch, estimate_path, gt_path, common_alignment=None):
    est_times, est_positions, est_quaternions = load_tum(estimate_path)
    gt_times, gt_positions, gt_quaternions = load_tum(gt_path)
    matched = pairs(est_times, gt_times)
    if len(matched) < 3:
        raise ValueError(f"{branch} trajectory has fewer than three GT matches")
    independent_alignment = umeyama(
        np.asarray([est_positions[i] for i, _ in matched]),
        np.asarray([gt_positions[j] for _, j in matched]),
    )
    row = {"sequence": sequence, "branch": branch,
           "rows": len(est_times), "matches": len(matched),
           "estimate_path": str(estimate_path)}
    overlap_start = max(est_times[0], gt_times[0])
    overlap_end = min(est_times[-1], gt_times[-1])
    row["completion"] = max(0.0, overlap_end - overlap_start) / max(
        1.0e-12, gt_times[-1] - gt_times[0])
    row.update(ape_metrics(est_times, est_positions, gt_times, gt_positions,
                           independent_alignment))
    row["alignment_frame"] = "independent_branch_umeyama"
    if common_alignment is not None:
        common = ape_metrics(est_times, est_positions, gt_times, gt_positions,
                             common_alignment)
        for key, value in common.items():
            row[f"common_frame_{key}"] = value
    row.update(rpe_metrics(est_times, est_positions, est_quaternions,
                           gt_times, gt_positions, gt_quaternions))
    row.update(distance_metrics(est_times, est_positions, est_quaternions,
                                gt_times, gt_positions, gt_quaternions))
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
    common_alignment = umeyama(
        np.asarray([native_positions[i] for i, _ in native_pairs]),
        np.asarray([gt_positions[j] for _, j in native_pairs]),
    )
    rows = [
        evaluate(args.sequence, "native", args.native, args.ground_truth,
                 common_alignment),
        evaluate(args.sequence, "asymmetric", args.asymmetric,
                 args.ground_truth, common_alignment),
    ]
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=sorted(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"sequence={args.sequence} native_matches={len(native_pairs)} "
          f"output={args.out}")
    for row in rows:
        print(row["branch"], "independent_ape_rmse_m=", row["ape_rmse_m"],
              "common_frame_ape_rmse_m=",
              row["common_frame_ape_rmse_m"],
              "rpe_10s_m=", row.get("rpe_10s_m"))


if __name__ == "__main__":
    main()
