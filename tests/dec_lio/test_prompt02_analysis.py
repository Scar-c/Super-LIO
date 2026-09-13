#!/usr/bin/env python3
"""Prompt02 synthetic local-error, projector, and ranking tests."""

import csv
import math
import pathlib
import tempfile
import unittest

import numpy as np

from eval.dec_lio.local_error_analysis import (
    analyze_sequence,
    projector_is_valid,
    quat_to_matrix,
    spearman,
)


class Prompt02AnalysisTest(unittest.TestCase):
    def test_spearman_and_projector_coordinate_transform(self):
        self.assertAlmostEqual(spearman([1, 2, 3], [10, 20, 30])["rho"], 1.0)
        weak = np.diag([1.0, 0.0, 0.0])
        self.assertTrue(projector_is_valid(weak, 1))
        rotation = np.asarray([[0.0, -1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]])
        transformed = rotation @ weak @ rotation.T
        self.assertTrue(projector_is_valid(transformed, 1))
        self.assertFalse(projector_is_valid(weak + 0.1, 1))
        self.assertTrue(np.allclose(quat_to_matrix([0.0, 0.0, 0.0, 1.0]), np.eye(3)))

    def test_synthetic_local_error_windows(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            gt_path = root / "gt.tum"
            est_path = root / "estimate.tum"
            summary_path = root / "summary.csv"
            with gt_path.open("w", encoding="utf-8") as gt, est_path.open(
                "w", encoding="utf-8"
            ) as est:
                for index in range(21):
                    time = float(index)
                    gt.write(f"{time} {time} 0 0 0 0 0 1\n")
                    # Curvature cannot be removed by one global rigid alignment.
                    position = time + 0.01 * time * time
                    est.write(f"{time} {position} 0 0 0 0 0 1\n")
            fields = [
                "schema_version", "frame", "iteration", "timestamp", "need_converge",
                "candidate_count", "used_residual_count", "used_residual_ratio", "valid",
                "factorization_ok", "eigensolver_ok", "cond_full", "cond_rot", "cond_trans",
                "weak_rank_rot", "weak_rank_trans",
            ]
            fields += [f"lambda_{mode}_{index}" for mode in ("rot", "trans") for index in range(3)]
            fields += [f"P_weak_{mode}_{index}" for mode in ("rot", "trans") for index in range(9)]
            with summary_path.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                for index in range(21):
                    row = {name: 0 for name in fields}
                    row.update({
                        "schema_version": 2, "frame": index, "iteration": 0,
                        "timestamp": float(index), "need_converge": 0,
                        "candidate_count": 100, "used_residual_count": 80,
                        "used_residual_ratio": 0.8, "valid": 1,
                        "factorization_ok": 1, "eigensolver_ok": 1,
                        "cond_full": 2 + index, "cond_rot": 2 + index,
                        "cond_trans": 2 + index, "weak_rank_rot": 0,
                        "weak_rank_trans": 1,
                    })
                    for mode in ("rot", "trans"):
                        for component, value in enumerate((1.0, 2.0, 3.0)):
                            row[f"lambda_{mode}_{component}"] = value
                    for mode in ("rot", "trans"):
                        matrix = np.diag([1.0, 0.0, 0.0]) if mode == "trans" else np.zeros((3, 3))
                        for component, value in enumerate(matrix.reshape(-1)):
                            row[f"P_weak_{mode}_{component}"] = value
                    writer.writerow(row)
            result = analyze_sequence(est_path, gt_path, summary_path)
            self.assertEqual(result["association"]["matched"], 21)
            self.assertEqual(result["windows"]["1.0"]["count"], 20)
            self.assertIsNotNone(
                result["windows"]["1.0"]["correlations"]["translation_error_m"]
                ["max_log_kappa_trans"]["rho"]
            )
            self.assertEqual(
                result["windows"]["1.0"]["error_stats"]["weak_projected_drift_m"]["count"],
                20,
            )


if __name__ == "__main__":
    unittest.main()
