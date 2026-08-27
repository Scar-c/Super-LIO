#!/usr/bin/env python3
"""Round11AB tests for the pinned M3DGR ArUco evaluator semantics."""
import math
import pathlib
import sys
import tempfile

import numpy as np


ROOT = pathlib.Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))

from eval_m3dgr_aruco import evaluate, load_reference, load_tum_strict


def rot_x(angle):
    c, s = math.cos(angle), math.sin(angle)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]], dtype=float)


def rot_z(angle):
    c, s = math.cos(angle), math.sin(angle)
    return np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]], dtype=float)


def matrix_to_quaternion(rotation):
    # Stable enough for the deliberately non-singular synthetic rotations.
    trace = float(np.trace(rotation))
    w = math.sqrt(1.0 + trace) / 2.0
    x = (rotation[2, 1] - rotation[1, 2]) / (4.0 * w)
    y = (rotation[0, 2] - rotation[2, 0]) / (4.0 * w)
    z = (rotation[1, 0] - rotation[0, 1]) / (4.0 * w)
    return np.array([x, y, z, w])


def pose(rotation, translation):
    result = np.eye(4)
    result[:3, :3] = rotation
    result[:3, 3] = translation
    return result


def write_reference(path, transform, duration=20.0):
    rotation = transform[:3, :3]
    translation = transform[:3, 3]
    path.write_text(
        "\n\n\n"
        + "\n".join(" ".join(f"{value:.12f}" for value in row) for row in rotation)
        + "\n\n"
        + "\n".join(f"{value:.12f}" for value in translation)
        + f"\n\nbag_time: {duration:.9f}s\n",
        encoding="utf-8",
    )


def write_tum(path, timestamped_poses):
    lines = []
    for timestamp, transform in timestamped_poses:
        translation = transform[:3, 3]
        quaternion = matrix_to_quaternion(transform[:3, :3])
        values = [timestamp, *translation, *quaternion]
        lines.append(" ".join(f"{value:.12f}" for value in values))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    with tempfile.TemporaryDirectory(prefix="round11ab-m3-eval-") as tmp_name:
        tmp = pathlib.Path(tmp_name)
        reference_path = tmp / "reference.txt"
        trajectory_path = tmp / "trajectory.tum"

        # Non-commuting rotations and a non-zero initial translation make the
        # inverse/reference-direction mistake observably wrong.
        first = pose(rot_x(0.37), np.array([4.0, -2.0, 1.5]))
        reference = pose(rot_z(-0.42), np.array([1.2, -0.7, 0.4]))
        last = first @ reference
        write_reference(reference_path, reference)
        write_tum(
            trajectory_path,
            [(100.0, first), (108.0, last), (108.5, last), (109.0, last)],
        )

        ref_rotation, ref_translation, ref_time = load_reference(reference_path)
        timestamps, poses = load_tum_strict(trajectory_path)
        metrics = evaluate(ref_rotation, ref_translation, ref_time, timestamps, poses)
        assert metrics["last_pose_index"] == 1, metrics
        assert abs(metrics["tracked_duration_s"] - 8.0) < 1e-9, metrics
        assert abs(metrics["tracking_rate_percent"] - 40.0) < 1e-9, metrics
        assert metrics["translation_error_m"] < 1e-10, metrics
        assert metrics["rotation_frobenius"] < 1e-10, metrics
        assert metrics["primary_metric"] == (
            "M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M"
        )
        assert metrics["combined_rmse_status"] == "NON_PRIMARY_MIXED_UNITS"

        inverse_metrics = evaluate(
            reference[:3, :3].T,
            -(reference[:3, :3].T @ reference[:3, 3]),
            ref_time,
            timestamps,
            poses,
        )
        assert inverse_metrics["translation_error_m"] > 1.0, inverse_metrics

        malformed = tmp / "malformed.tum"
        malformed.write_text("0 0 0 0 0 0 1\n", encoding="utf-8")
        try:
            load_tum_strict(malformed)
        except ValueError as error:
            assert "exactly 8 columns" in str(error)
        else:
            raise AssertionError("malformed TUM row was accepted")

    print("M3DGR ARUCO EVALUATOR TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
