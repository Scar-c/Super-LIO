#!/usr/bin/env python3
"""Prompt03 frame authority, corrective statistics, and robust-window tests."""

import math
import unittest

from eval.dec_lio.prompt03_analysis import (
    bootstrap_correlations,
    corrected_prompt02,
    empirical_thresholds,
    make_windows,
    spearman,
)


class Prompt03AnalysisTest(unittest.TestCase):
    def test_corrective_statistics_separate_rank_zero(self):
        def row(iteration, rank, projector):
            result = {
                "frame": "7", "iteration": str(iteration), "valid": "1",
                "weak_rank_rot": str(rank), "weak_rank_trans": "0",
            }
            result.update({f"P_weak_rot_{i}": str(value)
                           for i, value in enumerate(projector)})
            result.update({f"P_weak_trans_{i}": "0" for i in range(9)})
            result.update({f"normalized_lambda_rot_{i}": "0.01" if i < rank else "1"
                           for i in range(3)})
            result.update({f"raw_rot_basis_{i}": str(1.0 if i in (0, 4, 8) else 0.0)
                           for i in range(9)})
            return result

        identity = [1, 0, 0, 0, 0, 0, 0, 0, 0]
        rows = [row(0, 0, [0] * 9), row(1, 0, [0] * 9),
                row(2, 1, identity), row(3, 1, identity),
                row(4, 2, identity)]
        result = corrected_prompt02(rows)["modes"]["rot"]
        self.assertEqual(result["rank0_to_rank0_separate"], 1)
        self.assertEqual(result["rank_change_numerator"], 2)
        self.assertEqual(result["valid_adjacent_transition_denominator"], 4)
        self.assertEqual(result["same_nonzero_rank"]["rank1_to_rank1"]["count"], 1)
        self.assertEqual(result["same_nonzero_rank"]["rank2_to_rank2"]["count"], 0)

    def test_nonoverlap_windows_and_block_bootstrap(self):
        # Directly exercise the temporal block sampler with monotonic synthetic
        # windows.  Ten-second blocks are six windows for Delta=~2 seconds.
        windows = [{"geometry": {"signal": float(i)},
                    "translation_error_m": float(i)} for i in range(30)]
        report = bootstrap_correlations(windows, "translation_error_m", ["signal"], 2.0)
        self.assertEqual(report["seed"], 20260913)
        self.assertEqual(report["block_windows"], 5)
        self.assertEqual(report["replicates"], 1000)
        self.assertAlmostEqual(report["metrics"]["signal"]["median"], 1.0, places=12)

    def test_spearman_constant_series_is_undefined(self):
        self.assertIsNone(spearman([1, 1, 1], [1, 2, 3])["rho"])

    def test_empirical_report_contains_both_xicp_targets(self):
        rows = []
        for index, label in enumerate(("FULL", "PARTIAL", "NONE")):
            row = {"dcreg_schur_kappa_rot": str(index + 1)}
            row.update({f"xicp_class_rot_{mode}": label if mode == 0 else "FULL"
                        for mode in range(3)})
            rows.append(row)
        targets = empirical_thresholds(rows, "rot")["schur"]
        self.assertEqual(set(targets), {"nonfull", "none"})
        raw_rows = [{**row, "raw_block_kappa_rot": row["dcreg_schur_kappa_rot"]}
                    for row in rows]
        self.assertEqual(set(empirical_thresholds(raw_rows, "rot", raw=True)), {"raw"})


if __name__ == "__main__":
    unittest.main()
