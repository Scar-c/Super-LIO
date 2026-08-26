#!/usr/bin/env python3
"""Compare unaligned estimator state at identical raw LiDAR scan ends."""
import argparse
import csv
import math
import pathlib
import sys

import numpy as np


def load(path):
    rows = {}
    with pathlib.Path(path).open(encoding="utf-8", newline="") as stream:
        for row in csv.DictReader(stream):
            timestamp = float(row["scan_end"])
            if timestamp in rows:
                raise ValueError(f"{path}: duplicate scan_end {timestamp:.9f}")
            rows[timestamp] = row
    if not rows:
        raise ValueError(f"{path}: no raw scan-end rows")
    return rows


def vec(row, names):
    return np.asarray([float(row[name]) for name in names], dtype=float)


def percentile_text(values):
    return "/".join(f"{np.percentile(values, p):.6g}"
                    for p in (50, 90, 99, 100))


def compare(baseline, candidate):
    base = load(baseline)
    cand = load(candidate)
    if set(base) != set(cand):
        missing = len(set(base) - set(cand))
        extra = len(set(cand) - set(base))
        raise ValueError(
            f"raw scan-end key mismatch: missing={missing} extra={extra}")

    position, rotation, velocity, covariance = [], [], [], []
    p_fields = [f"P{r}_{c}" for r in range(18) for c in range(18)]
    for timestamp in sorted(base):
        left, right = base[timestamp], cand[timestamp]
        position.append(np.linalg.norm(
            vec(right, ("px", "py", "pz")) -
            vec(left, ("px", "py", "pz"))))
        velocity.append(np.linalg.norm(
            vec(right, ("vx", "vy", "vz")) -
            vec(left, ("vx", "vy", "vz"))))
        q_left = vec(left, ("qx", "qy", "qz", "qw"))
        q_right = vec(right, ("qx", "qy", "qz", "qw"))
        q_left /= np.linalg.norm(q_left)
        q_right /= np.linalg.norm(q_right)
        dot = float(np.clip(abs(np.dot(q_left, q_right)), 0.0, 1.0))
        rotation.append(2.0 * math.acos(dot))
        covariance.append(np.linalg.norm(
            vec(right, p_fields) - vec(left, p_fields)))

    return {
        "matched": len(base),
        "position": np.asarray(position),
        "rotation": np.asarray(rotation),
        "velocity": np.asarray(velocity),
        "covariance": np.asarray(covariance),
    }


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Raw-scan-end B0 versus candidate state comparison")
    parser.add_argument("baseline")
    parser.add_argument("candidate")
    parser.add_argument("--out")
    args = parser.parse_args(argv)
    try:
        metrics = compare(args.baseline, args.candidate)
        output = "\n".join((
            f"matched raw scan ends: {metrics['matched']}",
            "alignment: NONE",
            "position diff (m) P50/P90/P99/max: " +
            percentile_text(metrics["position"]),
            "rotation diff (rad) P50/P90/P99/max: " +
            percentile_text(metrics["rotation"]),
            "velocity diff (m/s) P50/P90/P99/max: " +
            percentile_text(metrics["velocity"]),
            "covariance Frobenius diff P50/P90/P99/max: " +
            percentile_text(metrics["covariance"]),
        )) + "\n"
        sys.stdout.write(output)
        if args.out:
            pathlib.Path(args.out).write_text(output, encoding="utf-8")
        return 0
    except (OSError, ValueError, KeyError, np.linalg.LinAlgError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
