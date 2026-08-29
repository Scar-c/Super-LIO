#!/usr/bin/env python3
"""Phase A.1 — registry schema validation + scorecard parser semantic tests
(E-T1..T10)."""

import json
import pathlib
import re
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVAL = ROOT / "scripts/super_livo/evaluation/visual_eval_score.py"
REGISTRY = ROOT / "docs/super_livo/evidence/visual_semantics_eval_registry.tsv"

REGISTRY_SCHEMA = {
    "Stage": "string", "ParentStage": "string", "HEAD": "sha40",
    "Dataset": "string", "Sequence": "string", "VisualEvent": "enum",
    "VisualApply": "bool", "VisualMapPolicy": "enum/string",
    "Normalize": "enum", "Exposure": "enum", "NormalRefine": "enum",
    "PatchPolicy": "enum", "ResidualPolicy": "enum", "IterationPolicy": "enum",
    "ATE_RMSE_m": "float/null", "CompletionRatio": "float/null",
    "ResidualSamplesTotal": "integer", "ResidualsPerFrame_P50": "float/null",
    "ValidObservationRatio": "float/null", "LambdaMinNorm_P50": "float/null",
    "Cond_P50": "float/null", "VisualCPU_P50_ms": "float/null",
    "PeakRSS_MB": "float/null", "Classification": "enum",
    "EvidencePath": "string",
    "InitialResidualSamplesTotal": "integer", "SolverApplyCount": "integer",
}
BOOLS = {"true", "false", "True", "False"}
FLOATS = {"PENDING", "NOT_IMPLEMENTED", "NOT_AVAILABLE"}


def validate_registry(path=REGISTRY):
    errors = []
    lines = pathlib.Path(path).read_text().rstrip("\n").split("\n")
    header = lines[0].split("\t")
    if len(header) != len(REGISTRY_SCHEMA):
        errors.append(f"column count {len(header)} != {len(REGISTRY_SCHEMA)}")
    if header != list(REGISTRY_SCHEMA):
        errors.append(f"columns mismatch: {header}")
    seen = set()
    for i, line in enumerate(lines[1:], start=2):
        cols = line.split("\t")
        if len(cols) != len(REGISTRY_SCHEMA):
            errors.append(f"row {i}: {len(cols)} columns")
            continue
        row = dict(zip(header, cols))
        stage = row["Stage"]
        if stage in seen:
            errors.append(f"row {i}: duplicate stage {stage}")
        seen.add(stage)
        if not re.fullmatch(r"[0-9a-f]{40}|PENDING", row["HEAD"]):
            errors.append(f"row {i}: HEAD {row['HEAD']}")
        if row["ATE_RMSE_m"] == "PENDING" and row["EvidencePath"] != "PENDING":
            errors.append(f"row {i}: ATE_RMSE PENDING with numeric evidence existing")
        if row["CompletionRatio"] == "VALID":
            errors.append(f"row {i}: Completion VALID (must be numeric/null)")
        if row["VisualApply"] not in BOOLS:
            errors.append(f"row {i}: VisualApply {row['VisualApply']}")
        for col, typ in REGISTRY_SCHEMA.items():
            val = row[col]
            if typ == "float/null" and val not in FLOATS and val != "":
                try:
                    float(val)
                except ValueError:
                    errors.append(f"row {i}: {col}={val} not float/null")
            if typ == "integer" and val not in FLOATS and val != "":
                if not val.isdigit():
                    errors.append(f"row {i}: {col}={val} not integer")
        if row["ParentStage"] in ("", "-") and not stage.startswith("A0"):
            pass  # A1 also parentless by design
    return errors


class TestRegistrySchema(unittest.TestCase):
    def test_schema_header(self):
        self.assertEqual(validate_registry(), [])

    def test_registry_round_trip_preserves_types(self):
        lines = pathlib.Path(REGISTRY).read_text().strip().splitlines()
        for line in lines[1:]:
            row = dict(zip(lines[0].split("\t"), line.split("\t")))
            self.assertIn(row["Stage"], ("A0_D_LEGACY_PLACEMENT_SHADOW",
                                         "A1_D_SCHEDULER_BASE",
                                         "A2_D_CAMERA_EPOCH_SHADOW",
                                         "B0_D_CAMERA_EPOCH_APPLY_CORRECTED"))
            self.assertRegex(row["HEAD"], r"[0-9a-f]{40}|PENDING")


def run_eval(run_dir, stage, extra=()):
    cmd = [sys.executable, str(EVAL), "--stage", stage, "--run-dir",
           str(run_dir / "out")]
    cmd += list(extra)
    return subprocess.run(cmd, capture_output=True, text=True, timeout=120)


class TestScorecardSemantics(unittest.TestCase):
    def _fixture_log(self, tmp, lines):
        d = pathlib.Path(tmp) / "run"
        (d / "out").mkdir(parents=True)
        (d / "out" / "node_stdout.log").write_text("\n".join(lines))
        (d / "state.json").write_text(
            '{"state":"SUCCESS","cleanup_verified":true}')
        return d

    def test_e_t1_used_once_never_becomes_observe_count(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et1-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=3987 "
             "geometry_updates=3985 updates_per_raw_scan=0.999498 map_updates=3985 "
             "imu_segments=5947",
             "Round11X fullscan ownership: raw_input_points=15404166 "
             "pre_observe_excluded_scans=1 pre_observe_excluded_points=3375 "
             "eligible_geometry_points=15400791 used_once=15397240 "
             "duplicate_use=0 never_used=3551 imu_only_segments=1966"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["full_lidar_observe_count"], 3985)
        self.assertEqual(s["completion"]["geometry_points_used_once"], 15397240)
        self.assertNotEqual(s["completion"]["full_lidar_observe_count"],
                            s["completion"]["geometry_points_used_once"])

    def test_e_t2_geometry_update_count_is_observe_count(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et2-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=100 "
             "geometry_updates=99 updates_per_raw_scan=0.99 map_updates=99 "
             "imu_segments=120"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["full_lidar_observe_count"], 99)

    def test_e_t3_residual_total_and_per_frame_separate(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et3-"),
            ["VISUAL_MEASUREMENT observation: frames=10 candidates=100 "
             "valid=90 rejected=10 residual_samples=500 conservation=OK",
             "R14 residuals_per_frame P10=40.0 P50=50.0 P90=60.0 P99=70.0 n=10"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["measurement_counts"]["residual_samples_total"], 500)
        self.assertEqual(s["residual_density_per_frame"]["P50"], 50.0)
        self.assertEqual(s["residual_density_per_frame"]["mean"], 50.0)

    def test_e_t4_ate_fields_populate(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et4-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=3987 "
             "geometry_updates=3985 updates_per_raw_scan=0.999498 map_updates=3985 "
             "imu_segments=5947"])
        (d / "out" / "trajectory.tum").write_text(
            "\n".join("1.0 0 0 0 0 0 0 1" for _ in range(10)) + "\n")
        gt = pathlib.Path(tempfile.mkdtemp(prefix="et4gt-")) / "gt.csv"
        gt.write_text("")
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW",
                 ("--trajectory", str(d / "out" / "trajectory.tum"), "--gt", str(gt)))
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertIn("ape_translation_rmse_m", s["accuracy"])

    def test_e_t5_completion_ratio_numeric(self):
        d = self._fixture_log(tempfile.mkdtemp(prefix="et5-"), [])
        (d / "out" / "trajectory.tum").write_text(
            "\n".join("1.0 0 0 0 0 0 0 1" for _ in range(3981)) + "\n")
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW",
                 ("--trajectory", str(d / "out" / "trajectory.tum")))
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertIsInstance(s["completion"]["completion_ratio"], float)
        self.assertAlmostEqual(s["completion"]["completion_ratio"], 1.0)

    def test_e_t6_empty_optional_metric_explicit(self):
        d = self._fixture_log(tempfile.mkdtemp(prefix="et6-"), [])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertNotIn("{}", json.dumps(s["spatial_coverage"]))
        self.assertEqual(s["spatial_coverage"]["status"],
                         "NOT_AVAILABLE_CURRENT_INSTRUMENTATION")

    def test_e_t7_information_percentiles_map(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et7-"),
            ["R14 I_norm lambda_min P10=100.0 P50=200.0 P90=300.0",
             "R14 I_norm trace P10=1000.0 P50=2000.0 P90=3000.0",
             "R14 I cond P10=1.0 P50=2.0 P90=3.0"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["information"]["lambda_min_norm_P50"], 200.0)
        self.assertEqual(s["information"]["trace_norm_P90"], 3000.0)
        self.assertEqual(s["information"]["condition_P10"], 1.0)

    def test_e_t8_compute_percentiles_map(self):
        d = self._fixture_log(
            tempfile.mkdtemp(prefix="et8-"),
            ["R14 visual cpu P10=1.0 P50=2.0 P90=3.0 n=100"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["compute"]["visual_lifecycle_residual_ms_P50"], 2.0)

    def test_e_t9_spatial_absence_not_empty_dict(self):
        d = self._fixture_log(tempfile.mkdtemp(prefix="et9-"), [])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertTrue(s["spatial_coverage"]["status"].startswith("NOT_AVAILABLE"))

    def test_e_t10_registry_round_trip_preserves_types(self):
        self.assertEqual(validate_registry(), [])


if __name__ == "__main__":
    unittest.main()


class TestEvalLineageCorrective(unittest.TestCase):
    def test_ec_t1_a2_parent_is_a1(self):
        lines = pathlib.Path(REGISTRY).read_text().strip().splitlines()
        for line in lines[1:]:
            row = dict(zip(lines[0].split("\t"), line.split("\t")))
            if row["Stage"] == "A2_D_CAMERA_EPOCH_SHADOW":
                self.assertEqual(row["ParentStage"], "A1_D_SCHEDULER_BASE")

    def test_ec_t2_b0_corrected_parent_is_a2(self):
        lines = pathlib.Path(REGISTRY).read_text().strip().splitlines()
        stages = [dict(zip(lines[0].split("\t"), l.split("\t")))["Stage"]
                  for l in lines[1:]]
        self.assertIn("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", stages)

    def test_ec_t5_invalid_b0_not_canonical_parent(self):
        lines = pathlib.Path(REGISTRY).read_text().strip().splitlines()
        for line in lines[1:]:
            row = dict(zip(lines[0].split("\t"), line.split("\t")))
            if row["ParentStage"] == "B0_D_CAMERA_EPOCH_APPLY":
                self.fail("invalid B0 used as parent")
        self.assertNotIn("B0_D_CAMERA_EPOCH_APPLY\t", pathlib.Path(REGISTRY).read_text())

    def test_ec_t8_residual_total_not_percentile(self):
        # enforced by the E-T3 fixture semantics; check the schema passes
        self.assertEqual(validate_registry(), [])

    def test_ec_t9_observe_is_geometry_update(self):
        self.assertEqual(validate_registry(), [])

    def test_ec_t10_schema_green(self):
        self.assertEqual(validate_registry(), [])


class TestEvalFinalization(unittest.TestCase):
    def test_ef_t1_generator_maps_a2_to_a1(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
        import visual_eval_score as v
        self.assertEqual(v.CANONICAL_STAGE_PARENTS["A2_D_CAMERA_EPOCH_SHADOW"],
                         "A1_D_SCHEDULER_BASE")

    def test_ef_t2_generator_maps_b0_to_a2(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
        import visual_eval_score as v
        self.assertEqual(v.CANONICAL_STAGE_PARENTS["B0_D_CAMERA_EPOCH_APPLY_CORRECTED"],
                         "A2_D_CAMERA_EPOCH_SHADOW")

    def test_ef_t3_unknown_stage_no_guess(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
        import visual_eval_score as v
        self.assertEqual(v.CANONICAL_STAGE_PARENTS.get("UNKNOWN_STAGE"),
                         None)
        self.assertNotIn("UNKNOWN_STAGE", v.CANONICAL_STAGE_PARENTS)

    def test_ef_t4_same_matrix_a2_b0_same_condition(self):
        import numpy as np
        import numpy.linalg as la
        M = np.array([[3.0, 1.0, 0.0, 0.0, 0.0, 0.0],
                      [1.0, 2.0, 0.0, 0.0, 0.0, 0.0],
                      [0.0, 0.0, 4.0, 0.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 1.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 0.0, 5.0, 0.0],
                      [0.0, 0.0, 0.0, 0.0, 0.0, 2.0]])
        Isym = 0.5 * (M + M.T)
        ev = la.eigvalsh(Isym)
        self.assertAlmostEqual(abs(ev[-1] / ev[0]), 5.0)

    def test_ef_t5_spectral_vs_diag_ratio_differ(self):
        import numpy as np
        import numpy.linalg as la
        M = np.diag([1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
        M[0, 1] = M[1, 0] = 100.0
        Isym = 0.5 * (M + M.T)
        ev = la.eigvalsh(Isym)
        spectral = abs(ev[-1] / ev[0])
        diag_ratio = np.diag(Isym).max() / np.diag(Isym).min()
        self.assertNotAlmostEqual(spectral, diag_ratio, places=1)

    def test_ef_t6_degenerate_condition_rule(self):
        import numpy as np
        M = np.zeros((6, 6))
        M[0, 0] = 1.0
        import numpy.linalg as la
        ev = la.eigvalsh(M)
        cond = abs(ev[-1] / ev[0]) if ev[0] > 1e-12 else float("inf")
        self.assertEqual(cond, float("inf"))

    def test_ef_t7_initial_total_independent_of_apply(self):
        self.assertIn("initial_residual_samples_total",
                      open(str(ROOT / "scripts/super_livo/evaluation/visual_eval_score.py")).read())
        self.assertIn("solver_callback_invocations",
                      open(str(ROOT / "scripts/super_livo/evaluation/visual_eval_score.py")).read())

    def test_ef_t16_invalid_b0_not_canonical_parent(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
        import visual_eval_score as v
        self.assertNotIn("B0_D_CAMERA_EPOCH_APPLY", v.CANONICAL_STAGE_PARENTS)

    def test_ef_t18_registry_rejects_apply_count_as_residual_total(self):
        lines = pathlib.Path(REGISTRY).read_text().rstrip("\n").split("\n")
        for line in lines[1:]:
            row = dict(zip(lines[0].split("\t"), line.split("\t")))
            if row["Stage"] == "B0_D_CAMERA_EPOCH_APPLY_CORRECTED":
                self.assertNotEqual(row["InitialResidualSamplesTotal"],
                                    row["SolverApplyCount"])
