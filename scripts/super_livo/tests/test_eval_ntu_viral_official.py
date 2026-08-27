#!/usr/bin/env python3
"""Round11AB semantic tests for the NTU VIRAL dataset-author evaluator."""
import math
import pathlib
import sys
import tempfile

import numpy as np


ROOT = pathlib.Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))

from eval_ntu_viral_official import evaluate, load_tum, PRISM_B


def quat_z(angle):
    return np.array([0.0, 0.0, math.sin(angle / 2.0), math.cos(angle / 2.0)])


def write_tum(path, rows):
    path.write_text(
        "".join(
            " ".join(f"{value:.12f}" for value in row) + "\n" for row in rows
        ),
        encoding="utf-8",
    )


def main():
    with tempfile.TemporaryDirectory(prefix="round11ab-ntu-eval-") as tmp_name:
        tmp = pathlib.Path(tmp_name)
        estimate = tmp / "estimate.tum"
        leica = tmp / "leica.tum"

        times = np.array([0.025, 0.075, 0.125, 0.175])
        positions = np.array(
            [[0.0, 0.0, 0.0], [1.0, 0.2, 0.0], [2.0, 1.0, 0.3], [3.0, 1.5, 1.0]]
        )
        quaternions = [quat_z(angle) for angle in (0.0, 0.3, 0.7, 1.1)]
        prism_positions = []
        for position, quaternion in zip(positions, quaternions):
            x, y, z, w = quaternion
            rotation = np.array(
                [
                    [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
                    [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
                    [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
                ]
            )
            prism_positions.append(position + rotation @ PRISM_B)

        write_tum(
            estimate,
            [[timestamp, *position, *quaternion] for timestamp, position, quaternion in zip(times, positions, quaternions)],
        )
        # Every estimate lands on the lower interpolation bracket; the extra
        # final sample closes the final bracket. A repeated final position must
        # be removed before interpolation, as in the dataset MATLAB code.
        gt_rows = [
            [timestamp, *prism, 0, 0, 0, 1]
            for timestamp, prism in zip(times, prism_positions)
        ]
        gt_rows.append([0.225, *(prism_positions[-1] + [0.1, 0.1, 0.1]), 0, 0, 0, 1])
        gt_rows.append(gt_rows[-1].copy())
        gt_rows[-1][0] += 0.01
        write_tum(leica, gt_rows)

        est_t, est_p, est_q = load_tum(estimate)
        gt_t, gt_p, _ = load_tum(leica)
        metrics = evaluate(est_t, est_p, est_q, gt_t, gt_p, PRISM_B)
        assert metrics["matched"] == 4, metrics
        assert metrics["translation_ate_rmse_m"] < 1e-9, metrics
        assert metrics["lever_arm_applied"] is True, metrics
        assert metrics["alignment"] == "SE3_UMEYAMA_NO_SCALE", metrics
        assert metrics["interpolation_max_bracket_s"] == 0.1, metrics

        no_prism = evaluate(est_t, est_p, est_q, gt_t, gt_p, np.zeros(3))
        assert no_prism["translation_ate_rmse_m"] > 0.01, no_prism

        # Official combteeth uses a strict bracket interval < 0.1 s.
        sparse_gt_t = np.array([0.0, 0.1, 0.2])
        sparse_gt_p = np.zeros((3, 3))
        try:
            evaluate(est_t, est_p, est_q, sparse_gt_t, sparse_gt_p, PRISM_B)
        except ValueError as error:
            assert "insufficient interpolated matches" in str(error)
        else:
            raise AssertionError("0.1-second bracket was incorrectly accepted")

    print("NTU VIRAL OFFICIAL EVALUATOR TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
