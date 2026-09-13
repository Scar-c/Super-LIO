#!/usr/bin/env python3
"""Regression tests for Prompt04's evaluation-only causal helpers."""

import math
import unittest

from eval.dec_lio.prompt04_analysis import contiguous_runs, matched_bins, valid_quaternion


class Prompt04AnalysisTest(unittest.TestCase):
    def test_position_only_quaternion_is_not_attitude(self):
        self.assertFalse(valid_quaternion([0.0, 0.0, 0.0, 0.0]))
        self.assertTrue(valid_quaternion([0.0, 0.0, 0.0, 1.0]))

    def test_contiguous_runs(self):
        self.assertEqual(contiguous_runs([1, 2, 3, 7, 9, 10]), [(1, 3), (7, 7), (9, 10)])
        self.assertEqual(contiguous_runs([]), [])

    def test_matched_bins_keep_empty_scene_mode_explicit(self):
        sample = {
            "kappa_rot": 3.0, "kappa_trans": 7.0,
            "translation_error_m": 1.0, "rotation_error_deg": None,
            "lambda_min_rot": 2.0, "lambda_min_trans": 3.0,
            "lambda_per_used_rot": 0.2, "lambda_per_used_trans": 0.3,
            "xicp_lc_min_rot": 4.0, "xicp_lc_min_trans": 5.0,
            "xicp_ls_min_rot": 6.0, "xicp_ls_min_trans": 7.0,
            "xicp_nonfull_fraction_rot": 0.0, "xicp_nonfull_fraction_trans": 0.5,
            "xicp_partial_fraction_rot": 0.0, "xicp_partial_fraction_trans": 0.5,
            "xicp_none_fraction_rot": 0.0, "xicp_none_fraction_trans": 0.0,
            "mu_min_rot": 0.1, "mu_min_trans": 0.2,
            "eta_weak_min_rot": 0.3, "eta_weak_min_trans": math.nan,
            "zeta_weak_min_rot": 0.4, "zeta_weak_min_trans": math.nan,
            "median_used_residual_count": 8.0,
        }
        report = {"scene": {"windows": {"5.0": {"overlapping": {"window_samples": [sample]}}}}}
        result = matched_bins(report)
        self.assertEqual(result["kappa_bins"]["<=5"]["sequences"]["scene"]["rot"]["window_count"], 1)
        self.assertEqual(result["kappa_bins"]["5-10"]["sequences"]["scene"]["trans"]["window_count"], 1)
        self.assertEqual(result["kappa_bins"][">20"]["sequences"]["scene"]["trans"]["window_count"], 0)


if __name__ == "__main__":
    unittest.main()
