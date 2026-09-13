#!/usr/bin/env python3
"""Translation APE with rigid SE(3) alignment and no scale or crop."""

import argparse
import pathlib
import sys

import numpy as np


def load_tum(path, allow_unsorted=False):
    times, positions, quaternions = [], [], []
    with open(path, encoding="utf-8") as stream:
        for number, line in enumerate(stream, 1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            values = line.split()
            if len(values) < 8:
                raise ValueError(f"{path}:{number}: expected 8 columns")
            row = [float(value) for value in values[:8]]
            times.append(row[0])
            positions.append(row[1:4])
            quaternions.append(row[4:8])
    if not times:
        raise ValueError(f"{path}: no TUM rows")
    times = np.asarray(times)
    if np.any(np.diff(times) <= 0) and not allow_unsorted:
        raise ValueError(f"{path}: timestamps are not strictly increasing")
    if allow_unsorted:
        order = np.argsort(times, kind="stable")
        times = times[order]
        positions = np.asarray(positions)[order]
        quaternions = np.asarray(quaternions)[order]
    return times, np.asarray(positions), np.asarray(quaternions)


def associate(est, gt, max_diff):
    candidates = []
    for ei, stamp in enumerate(est):
        lo = int(np.searchsorted(gt, stamp - max_diff, side="left"))
        hi = int(np.searchsorted(gt, stamp + max_diff, side="right"))
        candidates.extend((abs(float(stamp - gt[gi])), ei, gi)
                          for gi in range(lo, hi))
    used_est, used_gt, pairs = set(), set(), []
    for delta, ei, gi in sorted(candidates):
        if ei in used_est or gi in used_gt:
            continue
        used_est.add(ei)
        used_gt.add(gi)
        pairs.append((ei, gi, delta))
    return sorted(pairs)


def umeyama_se3(source, destination):
    if source.shape != destination.shape or len(source) < 3:
        raise ValueError("SE(3) alignment requires three paired points")
    source_mean, destination_mean = source.mean(0), destination.mean(0)
    covariance = (source - source_mean).T @ (destination - destination_mean) / len(source)
    u, _, vt = np.linalg.svd(covariance)
    correction = np.diag([1.0, 1.0, np.sign(np.linalg.det(vt.T @ u.T))])
    rotation = vt.T @ correction @ u.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("estimate")
    parser.add_argument("ground_truth")
    parser.add_argument("--max-diff", type=float, default=0.05)
    parser.add_argument("--min-matches", type=int, default=3)
    parser.add_argument("--out")
    args = parser.parse_args(argv)
    try:
        et, ep, eq = load_tum(args.estimate)
        # The supplied GEODE text has unique but non-monotonic records. Keep
        # the estimate strict and apply only a stable timestamp order to the
        # reference for association; no estimate rows are cropped or sorted.
        gt, gp, _ = load_tum(args.ground_truth, allow_unsorted=True)
        pairs = associate(et, gt, args.max_diff)
        if len(pairs) < args.min_matches:
            raise ValueError(f"insufficient matches: {len(pairs)}")
        source, destination = [], []
        for ei, gi, _ in pairs:
            norm = np.linalg.norm(eq[ei])
            if not np.isfinite(norm) or norm <= 0:
                raise ValueError(f"invalid estimate quaternion at row {ei}")
            source.append(ep[ei])
            destination.append(gp[gi])
        rotation, translation = umeyama_se3(np.asarray(source), np.asarray(destination))
        error = np.linalg.norm((rotation @ np.asarray(source).T).T + translation - destination, axis=1)
        metrics = {
            "matched": len(pairs),
            "overlap_s": min(et[-1], gt[-1]) - max(et[0], gt[0]),
            "rmse_m": np.sqrt(np.mean(error ** 2)),
            "mean_m": np.mean(error),
            "median_m": np.median(error),
            "p90_m": np.percentile(error, 90),
            "p95_m": np.percentile(error, 95),
            "max_m": np.max(error),
        }
        text = "\n".join(["alignment: SE(3), no scale, no crop",
                           "ground_truth_order: stable timestamp sort of supplied reference",
                           "estimate_order: native output order, not sorted"] +
                           [f"{key}: {value:.9g}" if isinstance(value, float)
                            else f"{key}: {value}" for key, value in metrics.items()]) + "\n"
        print(text, end="")
        if args.out:
            pathlib.Path(args.out).write_text(text, encoding="utf-8")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"EVALUATION_FAIL: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
