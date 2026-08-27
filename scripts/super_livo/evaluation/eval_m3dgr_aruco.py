#!/usr/bin/env python3
"""Deterministic wrapper for M3DGR's official ArUco sequence semantics.

The primary value is the first-to-last relative *translation* error.  The
upstream script also prints a rotation-matrix Frobenius norm and a combined
quantity called RMSE; the latter mixes metres with a dimensionless norm and is
retained only for byte-independent semantic parity, never as our primary score.
"""
import argparse
import hashlib
import pathlib
import sys
from typing import Optional, Sequence

import numpy as np
import yaml


UPSTREAM_REPOSITORY = "https://github.com/sjtuyinjie/M3DGR"
UPSTREAM_REVISION = "e0cf7d59c9a5a3df515624034698d976abc26549"
UPSTREAM_PATH = "ArUco_evaluate.py"
UPSTREAM_SHA256 = "ab01db4b27a98027b804dc5f691dcbac22ab0cf8f46cb26502339c1a995b86ab"
PRIMARY_METRIC = "M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M"


def sha256_file(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_reference(path):
    """Read the same fixed-line M3DGR reference representation as upstream."""
    lines = pathlib.Path(path).read_text(encoding="utf-8").splitlines()
    if len(lines) < 11:
        raise ValueError(f"{path}: incomplete M3DGR reference transform")
    try:
        rotation = np.asarray(
            [[float(value) for value in lines[index].split()] for index in (3, 4, 5)],
            dtype=float,
        )
        translation = np.asarray(
            [float(lines[index].strip()) for index in (7, 8, 9)], dtype=float
        )
        total_time = float(lines[-1].split(":", 1)[1].replace("s", "").strip())
    except (IndexError, ValueError) as error:
        raise ValueError(f"{path}: invalid M3DGR reference transform: {error}") from error
    if rotation.shape != (3, 3) or translation.shape != (3,):
        raise ValueError(f"{path}: invalid M3DGR transform dimensions")
    if not np.all(np.isfinite(rotation)) or not np.all(np.isfinite(translation)):
        raise ValueError(f"{path}: non-finite M3DGR transform")
    if not np.isfinite(total_time) or total_time <= 0:
        raise ValueError(f"{path}: reference duration must be positive")
    return rotation, translation, total_time


def quaternion_to_rotation(quaternion):
    x, y, z, w = quaternion
    norm = float(np.linalg.norm(quaternion))
    if not np.isfinite(norm) or norm <= 0:
        raise ValueError("invalid zero/non-finite TUM quaternion")
    x, y, z, w = quaternion / norm
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ],
        dtype=float,
    )


def load_tum_strict(path):
    """Load strict eight-column TUM, matching upstream format validation."""
    timestamps, poses = [], []
    with pathlib.Path(path).open(encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            fields = line.strip().split()
            if len(fields) != 8:
                raise ValueError(f"{path}:{line_number}: expected exactly 8 columns")
            try:
                values = np.asarray([float(field) for field in fields], dtype=float)
            except ValueError as error:
                raise ValueError(f"{path}:{line_number}: non-numeric TUM row") from error
            if not np.all(np.isfinite(values)):
                raise ValueError(f"{path}:{line_number}: non-finite TUM row")
            transform = np.eye(4)
            transform[:3, :3] = quaternion_to_rotation(values[4:8])
            transform[:3, 3] = values[1:4]
            timestamps.append(values[0])
            poses.append(transform)
    if not timestamps:
        raise ValueError(f"{path}: no TUM rows")
    timestamps = np.asarray(timestamps, dtype=float)
    if np.any(np.diff(timestamps) < 0):
        raise ValueError(f"{path}: timestamps decrease")
    return timestamps, np.asarray(poses)


def evaluate(ref_rotation, ref_translation, ref_total_time, timestamps, poses):
    if len(poses) != len(timestamps) or len(poses) < 1:
        raise ValueError("trajectory must contain at least one timestamped pose")
    last_pose_index = len(poses) - 1
    for index in range(len(poses) - 2, -1, -1):
        if not np.allclose(poses[index], poses[last_pose_index]):
            break
        last_pose_index = index

    relative = np.linalg.inv(poses[0]) @ poses[last_pose_index]
    rotation_error = float(np.linalg.norm(ref_rotation - relative[:3, :3]))
    translation_error = float(np.linalg.norm(ref_translation - relative[:3, 3]))
    combined = float(np.sqrt((rotation_error ** 2 + translation_error ** 2) / 2.0))
    tracked_duration = float(timestamps[last_pose_index] - timestamps[0])
    tracking_rate = min(100.0, tracked_duration / float(ref_total_time) * 100.0)
    return {
        "primary_metric": PRIMARY_METRIC,
        "translation_error_m": translation_error,
        "rotation_frobenius": rotation_error,
        "rotation_unit": "DIMENSIONLESS_MATRIX_FROBENIUS_NORM",
        "combined_rmse": combined,
        "combined_rmse_status": "NON_PRIMARY_MIXED_UNITS",
        "first_pose_index": 0,
        "last_pose_index": int(last_pose_index),
        "trajectory_pose_count": int(len(poses)),
        "tracked_duration_s": tracked_duration,
        "reference_duration_s": float(ref_total_time),
        "tracking_rate_percent": tracking_rate,
        "relative_transform_convention": "T_B0_Bend = inverse(T_W_B0) @ T_W_Bend",
        "reference_transform_convention": "T_B0_Bend (end-frame coordinates into initial body frame)",
    }


def _identity(path):
    path = pathlib.Path(path).resolve()
    return {"path": str(path), "size_bytes": path.stat().st_size, "sha256": sha256_file(path)}


def attach_provenance(path, result_path, metrics):
    path = pathlib.Path(path)
    data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
    data["evaluator"] = {
        "status": "ATTACHED",
        "name": PRIMARY_METRIC,
        "provenance_tier": "DATASET_AUTHOR_BENCHMARK",
        "upstream_repository": UPSTREAM_REPOSITORY,
        "upstream_revision": UPSTREAM_REVISION,
        "upstream_path": UPSTREAM_PATH,
        "upstream_sha256": UPSTREAM_SHA256,
        "wrapper": _identity(pathlib.Path(__file__)),
        "result": _identity(result_path),
        "primary_value": metrics["translation_error_m"],
        "unit": "m",
    }
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(yaml.safe_dump(data, sort_keys=False), encoding="utf-8")
    temporary.replace(path)


def parse_args(argv: Optional[Sequence[str]] = None):
    parser = argparse.ArgumentParser(description="M3DGR ArUco relative-pose evaluator")
    parser.add_argument("reference", type=pathlib.Path)
    parser.add_argument("trajectory", type=pathlib.Path)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    parser.add_argument("--provenance", type=pathlib.Path)
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    try:
        ref_rotation, ref_translation, ref_total_time = load_reference(args.reference)
        timestamps, poses = load_tum_strict(args.trajectory)
        metrics = evaluate(
            ref_rotation, ref_translation, ref_total_time, timestamps, poses
        )
        document = {
            "schema_version": 1,
            "evaluator": {
                "name": PRIMARY_METRIC,
                "provenance_tier": "DATASET_AUTHOR_BENCHMARK",
                "upstream_repository": UPSTREAM_REPOSITORY,
                "upstream_revision": UPSTREAM_REVISION,
                "upstream_path": UPSTREAM_PATH,
                "upstream_sha256": UPSTREAM_SHA256,
                "wrapper_path": str(pathlib.Path(__file__).resolve()),
                "wrapper_sha256": sha256_file(pathlib.Path(__file__)),
            },
            "inputs": {
                "reference": _identity(args.reference),
                "trajectory": _identity(args.trajectory),
            },
            "result": metrics,
        }
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(yaml.safe_dump(document, sort_keys=False), encoding="utf-8")
        if args.provenance is not None:
            attach_provenance(args.provenance, args.out, metrics)
        print(f"primary_metric: {PRIMARY_METRIC}")
        print(f"translation_error_m: {metrics['translation_error_m']:.9f}")
        print(f"rotation_frobenius: {metrics['rotation_frobenius']:.9f}")
        print(f"tracking_rate_percent: {metrics['tracking_rate_percent']:.6f}")
        print("combined_rmse_status: NON_PRIMARY_MIXED_UNITS")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError, yaml.YAMLError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
