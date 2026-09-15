import csv
import importlib.util
import math
import pathlib
import tempfile
import unittest

import numpy as np


EVALUATOR = pathlib.Path(__file__).parents[2] / "eval/dec_lio/prompt16_evaluate.py"
SPEC = importlib.util.spec_from_file_location("prompt17_evaluator", EVALUATOR)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class Prompt17EvaluatorTest(unittest.TestCase):
    def test_relative_translation_is_global_rotation_invariant(self):
        q_global = np.array([0.0, 0.0, math.sin(math.pi / 4.0),
                             math.cos(math.pi / 4.0)])
        q_identity = np.array([0.0, 0.0, 0.0, 1.0])
        est_start = np.array([0.0, 0.0, 0.0])
        est_end = np.array([1.0, 0.0, 0.0])
        gt_rotation = MODULE.quaternion_matrix(q_global)
        gt_start = gt_rotation @ est_start
        gt_end = gt_rotation @ est_end
        error = MODULE.relative_translation_error(
            est_start, q_identity, est_end, q_identity,
            gt_start, q_global, gt_end, q_global)
        self.assertLess(error, 1.0e-12)

    def test_corrected_rpe_and_segment_are_rotation_invariant(self):
        q_global = np.array([0.0, 0.0, math.sin(math.pi / 4.0),
                             math.cos(math.pi / 4.0)])
        q_identity = np.array([0.0, 0.0, 0.0, 1.0])
        times = np.arange(7, dtype=float)
        est_positions = np.column_stack((times, np.zeros(7), np.zeros(7)))
        gt_positions = (MODULE.quaternion_matrix(q_global) @
                        est_positions.T).T
        est_quaternions = np.tile(q_identity, (7, 1))
        gt_quaternions = np.tile(q_global, (7, 1))
        rpe = MODULE.rpe_metrics(
            times, est_positions, est_quaternions,
            times, gt_positions, gt_quaternions)
        segments = MODULE.distance_metrics(
            times, est_positions, est_quaternions,
            times, gt_positions, gt_quaternions)
        self.assertEqual(rpe["rpe_1s_matches"], 6)
        self.assertLess(rpe["rpe_1s_m"], 1.0e-12)
        self.assertGreater(segments["segment_5m_matches"], 0)
        self.assertLess(segments["segment_5m_m"], 1.0e-12)

    def test_output_labels_independent_and_common_ape(self):
        q_global = np.array([0.0, 0.0, math.sin(math.pi / 4.0),
                             math.cos(math.pi / 4.0)])
        q_identity = np.array([0.0, 0.0, 0.0, 1.0])
        est_positions = np.array([
            [0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [1.0, 2.0, 0.0],
            [2.0, 2.0, 1.0],
        ])
        gt_positions = (MODULE.quaternion_matrix(q_global) @
                        est_positions.T).T

        def write_tum(path, positions, quaternion):
            with path.open("w", encoding="utf-8") as stream:
                for index, position in enumerate(positions):
                    values = [float(index), *position, *quaternion]
                    stream.write(" ".join(f"{value:.17g}" for value in values)
                                 + "\n")

        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            native = root / "native.tum"
            asymmetric = root / "asymmetric.tum"
            ground_truth = root / "gt.tum"
            output = root / "evaluation.csv"
            write_tum(native, est_positions, q_identity)
            write_tum(asymmetric, est_positions, q_identity)
            write_tum(ground_truth, gt_positions, q_global)
            MODULE.main([
                "--sequence", "synthetic",
                "--ground-truth", str(ground_truth),
                "--native", str(native),
                "--asymmetric", str(asymmetric),
                "--out", str(output),
            ])
            with output.open(newline="", encoding="utf-8") as stream:
                rows = list(csv.DictReader(stream))
            self.assertEqual(len(rows), 2)
            self.assertEqual(rows[0]["alignment_frame"],
                             "independent_branch_umeyama")
            for row in rows:
                self.assertIn("common_frame_ape_rmse_m", row)
                self.assertLess(float(row["ape_rmse_m"]), 1.0e-10)
                self.assertLess(float(row["common_frame_ape_rmse_m"]),
                                1.0e-10)


if __name__ == "__main__":
    unittest.main()
