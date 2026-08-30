#!/usr/bin/env python3
# Provenance: ported from origin/super-livo:scripts/super_livo/evaluation/eval_tum_translation.py.
# Legacy source audited at the immutable old-branch reference; semantics retained:
# translation APE after rigid SE(3) Umeyama alignment, explicitly no scale.
"""Translation APE for TUM trajectories using rigid SE(3), never scale."""
import argparse
import pathlib
import shlex
import subprocess
import sys

import numpy as np


def quat_to_rot(q):
    x, y, z, w = q
    return np.array([
        [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
        [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
        [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
    ])


def load_tum(path):
    timestamps, positions, quaternions = [], [], []
    with open(path, encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            values = line.split()
            if len(values) < 8:
                raise ValueError(f"{path}:{line_number}: expected at least 8 columns")
            row = [float(value) for value in values[:8]]
            timestamps.append(row[0])
            positions.append(row[1:4])
            quaternions.append(row[4:8])
    if not timestamps:
        raise ValueError(f"{path}: no TUM rows")
    timestamps = np.asarray(timestamps, dtype=float)
    if np.any(np.diff(timestamps) <= 0):
        raise ValueError(f"{path}: timestamps are not strictly increasing")
    return timestamps, np.asarray(positions), np.asarray(quaternions)


def associate_unique(est_times, gt_times, max_diff):
    """Return one-to-one pairs, prioritising the smallest absolute time delta."""
    candidates = []
    for est_index, timestamp in enumerate(est_times):
        lo = int(np.searchsorted(gt_times, timestamp - max_diff, side="left"))
        hi = int(np.searchsorted(gt_times, timestamp + max_diff, side="right"))
        for gt_index in range(lo, hi):
            candidates.append(
                (abs(float(timestamp - gt_times[gt_index])), est_index, gt_index)
            )
    used_est, used_gt, pairs = set(), set(), []
    for delta, est_index, gt_index in sorted(candidates):
        if est_index in used_est or gt_index in used_gt:
            continue
        used_est.add(est_index)
        used_gt.add(gt_index)
        pairs.append((est_index, gt_index, delta))
    pairs.sort()
    return pairs


def umeyama_se3(source, destination):
    if source.shape != destination.shape or source.shape[0] < 3:
        raise ValueError("SE(3) alignment requires at least three paired 3D points")
    source_mean = source.mean(axis=0)
    destination_mean = destination.mean(axis=0)
    covariance = (
        (source - source_mean).T @ (destination - destination_mean)
        / source.shape[0]
    )
    u_matrix, _, vt_matrix = np.linalg.svd(covariance)
    sign = np.sign(np.linalg.det(vt_matrix.T @ u_matrix.T))
    correction = np.diag([1.0, 1.0, sign])
    rotation = vt_matrix.T @ correction @ u_matrix.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


def git_head(repo_root):
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=str(repo_root),
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "UNKNOWN"


def parse_vector(text):
    values = [float(value) for value in text.split(",")]
    if len(values) != 3:
        raise argparse.ArgumentTypeError("--prism requires x,y,z")
    return np.asarray(values, dtype=float)


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="TUM translation APE (SE(3) alignment, no scale)"
    )
    parser.add_argument("estimate")
    parser.add_argument("ground_truth")
    parser.add_argument("--prism", type=parse_vector, default=np.zeros(3),
                        help="body-frame offset of the compared GT point: x,y,z")
    parser.add_argument("--frame", default="body",
                        help="physical comparison frame")
    parser.add_argument("--max-diff", type=float, default=0.05,
                        help="maximum absolute association delta in seconds")
    parser.add_argument("--min-matches", type=int, default=3)
    parser.add_argument("--out", help="optional persistent text metrics output")
    return parser.parse_args(argv)


def evaluate(args):
    estimate_time, estimate_position, estimate_quaternion = load_tum(args.estimate)
    gt_time, gt_position, _ = load_tum(args.ground_truth)

    compared_position = np.empty_like(estimate_position)
    for index, (position, quaternion) in enumerate(
            zip(estimate_position, estimate_quaternion)):
        norm = np.linalg.norm(quaternion)
        if not np.isfinite(norm) or norm <= 0:
            raise ValueError(f"estimate quaternion {index} is invalid")
        compared_position[index] = position + quat_to_rot(quaternion / norm) @ args.prism

    pairs = associate_unique(estimate_time, gt_time, args.max_diff)
    if len(pairs) < args.min_matches:
        raise ValueError(
            f"insufficient matches: {len(pairs)} < {args.min_matches} "
            f"within max_diff={args.max_diff:.6f}s"
        )
    source = np.asarray([compared_position[est] for est, _, _ in pairs])
    destination = np.asarray([gt_position[gt] for _, gt, _ in pairs])
    rotation, translation = umeyama_se3(source, destination)
    aligned = (rotation @ source.T).T + translation
    errors = np.linalg.norm(aligned - destination, axis=1)
    mt = [estimate_time[est] for est, _, _ in pairs]
    assoc_dt = np.abs([gt_time[gt] - estimate_time[est] for est, gt, _ in pairs])
    return {
        "matched": len(pairs),
        "duration": float(
            min(estimate_time[-1], gt_time[-1])
            - max(estimate_time[0], gt_time[0])
        ),
        "matched_duration": float(mt[-1] - mt[0]) if len(mt) > 1 else 0.0,
        "assoc_dt_p50": float(np.percentile(assoc_dt, 50)) if len(assoc_dt) else 0.0,
        "assoc_dt_p90": float(np.percentile(assoc_dt, 90)) if len(assoc_dt) else 0.0,
        "assoc_dt_p95": float(np.percentile(assoc_dt, 95)) if len(assoc_dt) else 0.0,
        "assoc_dt_p99": float(np.percentile(assoc_dt, 99)) if len(assoc_dt) else 0.0,
        "assoc_dt_max": float(np.max(assoc_dt)) if len(assoc_dt) else 0.0,
        "rmse": float(np.sqrt(np.mean(errors ** 2))),
        "mean": float(np.mean(errors)),
        "median": float(np.median(errors)),
        "max": float(np.max(errors)),
        "p90": float(np.percentile(errors, 90)),
        "p95": float(np.percentile(errors, 95)),
    }


def render(args, metrics, argv):
    script = pathlib.Path(__file__).resolve()
    repo_root = script.parents[3]
    lines = [
        f"git HEAD: {git_head(repo_root)}",
        f"script path: {script}",
        f"arguments: {shlex.join(argv)}",
        f"comparison frame: {args.frame}",
        "alignment type: SE(3), no scale",
        f"association max_diff: {args.max_diff:.6f} s",
        f"matched count: {metrics['matched']}",
        f"trajectory_overlap_duration: {metrics['duration']:.3f} s",
        f"matched_duration: {metrics['matched_duration']:.3f} s",
        (
            "association |dt| s: "
            f"P50={metrics['assoc_dt_p50']:.4f} "
            f"P90={metrics['assoc_dt_p90']:.4f} "
            f"P95={metrics['assoc_dt_p95']:.4f} "
            f"P99={metrics['assoc_dt_p99']:.4f} "
            f"max={metrics['assoc_dt_max']:.4f}"
        ),
        (
            "translation APE (m): "
            f"RMSE={metrics['rmse']:.4f} "
            f"mean={metrics['mean']:.4f} "
            f"median={metrics['median']:.4f} "
            f"max={metrics['max']:.4f} "
            f"P90={metrics['p90']:.4f} "
            f"P95={metrics['p95']:.4f}"
        ),
    ]
    return "\n".join(lines) + "\n"


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        metrics = evaluate(args)
        output = render(args, metrics, raw_argv)
        sys.stdout.write(output)
        if args.out:
            pathlib.Path(args.out).write_text(output, encoding="utf-8")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())


