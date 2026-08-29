#!/usr/bin/env python3
"""Prompt75 FS-T1..FS-T30 — canonical eval final-seal tests.

BEHAVIORAL / GENERATOR_LEVEL / PRODUCTION_HELPER_UNIT / REAL_E2E only:
- C++ behavior tests run through the built round14_final_seal_test binary
- evaluator tests call build_scorecard(...) on fixture producers
- registry tests go through visual_eval_registry.generate_registry(...)
- round-trip tests consume REAL run artifacts (results/round14_phaseA)
"""
import json
import pathlib
import re
import subprocess
import sys
import tempfile
import unittest

import yaml

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVAL = ROOT / "scripts/super_livo/evaluation/visual_eval_score.py"
REGISTRY_GEN = ROOT / "scripts/super_livo/evaluation/visual_eval_registry.py"
GT_EVAL = ROOT / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"

sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
import visual_eval_score as v  # noqa: E402
import visual_eval_registry as R  # noqa: E402


def find_cpp_test_binary():
    for base in ("/home/lc/super_livo/devel/.private/super_lio/lib/super_lio",
                 "/home/lc/super_livo/devel/lib/super_lio"):
        p = pathlib.Path(base) / "round14_final_seal_test"
        if p.exists():
            return p
    return None


def run_cpp():
    bin_path = find_cpp_test_binary()
    if bin_path is None:
        raise AssertionError("round14_final_seal_test binary not built")
    return subprocess.run([str(bin_path)], capture_output=True, text=True,
                          timeout=120)


def fixture_run(tmp, lines, manifest=None, provenance=None, config_hash=None,
                trajectory_rows=3981, state='{"state":"SUCCESS","cleanup_verified":true}'):
    d = pathlib.Path(tmp) / "run"
    (d / "out").mkdir(parents=True)
    (d / "out" / "node_stdout.log").write_text("\n".join(lines))
    (d / "state.json").write_text(state)
    if manifest:
        (d / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(manifest))
    if provenance:
        (d / "out" / "run_provenance.yaml").write_text(
            yaml.safe_dump({"run": provenance}))
    if config_hash:
        (d / "out" / "effective_config.post_resolve.yaml.sha256").write_text(
            config_hash + "\n")
    if trajectory_rows:
        times = [1.0 + i * 1e-3 for i in range(trajectory_rows)]
        (d / "out" / "trajectory.tum").write_text(
            "\n".join(f"{t:.9f} 0 0 0 0 0 0 1" for t in times))
        (d / "gt.csv").write_text(
            "# time, seq, stamp, x, y, z, ...\n" + "\n".join(
                f"{t * 1e9:.0f} 0 0 0.0 0.0 0.0 0 0 0 0 0 0 0 0" for t in times))
    return d


def run_eval(run_dir, stage, extra=()):
    cmd = [sys.executable, str(EVAL), "--stage", stage, "--run-dir",
           str(run_dir / "out")]
    if (run_dir / "out" / "trajectory.tum").exists():
        cmd += ["--trajectory", str(run_dir / "out" / "trajectory.tum")]
    if (run_dir / "gt.csv").exists() and not any(a == "--gt" for a in extra):
        cmd += ["--gt", str(run_dir / "gt.csv")]
    cmd += list(extra)
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
    return r, json.loads((run_dir / "out" / "visual_eval_score.json").read_text())


MANIFEST = {"dataset": "NTU", "sequence": "eee_01",
            "semantic_profile": "D_VISUAL_APPLY",
            "config_provenance": {"dataset_calibration": "/cfg/eee_01.yaml"},
            "production_revision": "a" * 40}
PROVENANCE = {"git_head": "a" * 40, "git_dirty": False}
OWNERSHIP = ("Round11X fullscan ownership: raw_input_points=1000 "
             "pre_observe_excluded_scans=1 pre_observe_excluded_points=10 "
             "eligible_geometry_points=990 used_once=980 duplicate_use=0 "
             "never_used=10 imu_only_segments=5 "
             "raw_input_scans=100 eligible_scans=99 unique_used_scans=98 "
             "duplicate_scan_use_events=0 eligible_never_used_scans=1")
CADENCE = ("Round11X cadence: policy=imu_fullscan raw_scans=100 "
           "geometry_updates=98 updates_per_raw_scan=0.98 map_updates=98 "
           "imu_segments=5")
BASE_LINES = [
    CADENCE, OWNERSHIP,
    "VISUAL_MEASUREMENT query: attempts=100 hits=95 misses=5 rejected_explicit=0 conservation=OK",
    "VISUAL_MEASUREMENT observation: frames=50 candidates=950 valid=930 rejected=20 residual_samples=10000 conservation=OK",
    "VISUAL_MEASUREMENT H: accumulations=50 nonzero=50 zero=0 nonfinite=0 norm_count=50 P50=1 P95=2 P99=3 max=4",
    "VISUAL_MEASUREMENT b: accumulations=50 nonzero=50 zero=0 nonfinite=0 norm_count=50 P50=1 P95=2 P99=3 max=4",
    "R14 initial measurement query: attempts=40 hits=38",
    "R14 initial measurement observation: frames=20 candidates=400 valid=390 rejected=10 residual_samples=4000",
    "R14 solver measurement query: attempts=60 hits=57",
    "R14 solver measurement observation: frames=30 candidates=550 valid=540 rejected=10 residual_samples=6000",
    "R14 I_norm lambda_min P10=10.0 P50=20.0 P90=30.0",
    "R14 I_norm lambda_max P10=1000.0 P50=2000.0 P90=3000.0",
    "R14 I_norm trace P10=1000.0 P50=2000.0 P90=3000.0",
    "R14 I cond P10=50.0 P50=100.0 P90=150.0",
    "R14 I degenerate_frames=0 invalid_frames=0",
    "R14 camera-epoch Visual: executions=20",
    "R14 LiDAR-callback Visual: executions=0 skipped=80",
    "R14 duplicate Visual: count=0",
    "R14 payload missing: 0",
    "R14 payload released-before: 0",
    "R14 payload release-after: 20",
    "R14 residuals_per_frame P10=180.0 P50=200.0 P90=220.0 P99=240.0 n=20",
    "R14 Apply attempts=20 success=20 fail=0",
    "R14 Apply skip zero_candidate=1 zero_valid_residual=0",
    "R14 solver callbacks=80 completed_iterations=80",
    "R14 solver callbacks_per_apply P10=4.0 P50=4.0 P90=4.0 max=4.0 n=20",
    "R14 solver iterations_per_apply mean=4.000 P10=4.0 P50=4.0 P90=4.0 P99=4.0 max=4.0 n=20",
    "R14 initial_residual_samples_total=4000 initial_frames=20",
    "R14 visual cpu P10=5.0 P50=10.0 P90=15.0",
    "R14 Apply delta_pos_m P50=0.01 P90=0.02 delta_rot_rad P50=0.001",
    "R14 cov_trace before_P50=1.0 after_P50=0.9 delta_P50=0.1",
]


class TestFinalSealBehavior(unittest.TestCase):
    def test_fs_t1_cpp_helper_behavior(self):
        r = run_cpp()
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("FS-T1 helper valid: PASS", r.stdout)
        self.assertIn("FS-T1 sorted lambda_min == 1/2: PASS", r.stdout)

    def test_fs_t2_single_canonical_implementation(self):
        helper_h = (ROOT / "src/super_lio/include/lio/VisualInformationMetrics.h").read_text()
        self.assertEqual(helper_h.count("computeVisualInformationMetrics"), 1)
        cpp = (ROOT / "src/super_lio/src/lio/super_lio.cpp").read_text()
        self.assertEqual(cpp.count("computeVisualInformationMetrics("), 2)
        self.assertNotIn("Hn.eigenvalues()", cpp)
        # single degeneracy rule: no residual 1e-12 threshold outside the
        # helper (the unrelated 1e-30 division guards in the FD/photo paths
        # are not information metrics).
        self.assertNotIn("lam_min > 1e-12", cpp)
        self.assertNotIn("lam_min <= 1e-12", cpp)

    def test_fs_t3_same_helper_identical_metric(self):
        r = run_cpp()
        for line in ("FS-T3 A2/B0 caller identical lambda_min: PASS",
                     "FS-T3 A2/B0 caller identical lambda_max: PASS",
                     "FS-T3 A2/B0 caller identical condition: PASS"):
            self.assertIn(line, r.stdout)

    def test_fs_t4_degenerate_single_rule(self):
        r = run_cpp()
        for line in ("FS-T4 A2/B0 identical degeneracy: PASS",
                     "FS-T4 degenerate flagged: PASS",
                     "FS-T4 condition == +inf: PASS",
                     "FS-T4 epsilon single source: PASS"):
            self.assertIn(line, r.stdout)

    def test_fs_t5_t6_eskf_loop_producer(self):
        r = run_cpp()
        self.assertIn("FS-T5 loop source increments once per iteration: PASS", r.stdout)
        self.assertIn("FS-T5 early-break counter matches executed iterations: PASS", r.stdout)
        m = re.search(r"FS_ESKF_LOOP apply=(\d+) iterations=(\d+) callbacks=(\d+)", r.stdout)
        self.assertIsNotNone(m)
        self.assertEqual((m.group(1), m.group(2), m.group(3)), ("1", "4", "4"))

    def test_fs_t7_t9_context_accounting(self):
        r = run_cpp()
        for line in ("FS-T7 solver residual total sums callbacks: PASS",
                     "FS-T8 initial attempts 3 (2 hit + 1 miss): PASS",
                     "FS-T9 solver counters untouched: PASS",
                     "FS-T8 initial unchanged after solver: PASS"):
            self.assertIn(line, r.stdout)

    def test_fs_t21_duplicate_scan_use_events(self):
        r = run_cpp()
        self.assertIn("FS-T21 duplicate scan-use event counted: PASS", r.stdout)
        self.assertIn("FS-T21 aggregate updates==unique+dup: PASS", r.stdout)


class TestFinalSealEvaluator(unittest.TestCase):
    def _card(self, stage="B0_D_CAMERA_EPOCH_APPLY_CORRECTED", lines=None, **kw):
        d = fixture_run(tempfile.mkdtemp(prefix="fs-"),
                        lines or BASE_LINES,
                        manifest=dict(MANIFEST, production_revision=PROVENANCE["git_head"]),
                        provenance=PROVENANCE, config_hash="c" * 64, **kw)
        r, s = run_eval(d, stage)
        self.assertEqual(r.returncode, 0, r.stderr)
        return d, s

    def test_fs_t10_initial_mean_initial_total_frames(self):
        _, s = self._card()
        self.assertEqual(s["residual_density_per_frame"]["mean"], 4000.0 / 20)

    def test_fs_t11_global_cannot_populate_initial(self):
        _, s = self._card()
        self.assertEqual(s["measurement_counts"]["initial_residual_samples_total"], 4000)
        self.assertNotEqual(
            s["measurement_counts"]["initial_residual_samples_total"],
            s["measurement_counts"]["residual_samples_total"])

    def test_fs_t13_scorecard_parses_gt_4stat(self):
        d = fixture_run(tempfile.mkdtemp(prefix="fs13-"), BASE_LINES,
                        manifest=dict(MANIFEST, production_revision=PROVENANCE["git_head"]),
                        provenance=PROVENANCE, config_hash="c" * 64)
        r, s = run_eval(d, "B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        self.assertEqual(r.returncode, 0, r.stderr)
        for k in ("ape_translation_rmse_m", "ape_translation_mean_m",
                  "ape_translation_median_m", "ape_translation_max_m"):
            self.assertIsInstance(s["accuracy"][k], float, k)

    def test_fs_t20_scan_fields_distinct(self):
        _, s = self._card()
        c = s["completion"]
        self.assertEqual(c["raw_lidar_input_scans"], 100)
        self.assertEqual(c["preobserve_excluded_scans"], 1)
        self.assertEqual(c["eligible_raw_scans"], 99)
        self.assertEqual(c["unique_geometry_used_scans"], 98)
        self.assertEqual(c["geometry_update_events"], 98)
        self.assertEqual(c["duplicate_geometry_use_events"], 0)
        self.assertEqual(c["eligible_never_used_scans"], 1)
        self.assertNotIn("processable_raw_lidar_scans", c)

    def test_fs_t22_missing_event_evidence_no_invention(self):
        lines = [l for l in BASE_LINES
                 if not l.startswith("R14 camera-epoch Visual")
                 and not l.startswith("R14 LiDAR-callback Visual")]
        _, s = self._card(lines=lines)
        self.assertEqual(s["event_placement"]["camera_event_visual_count"], "EVIDENCE_MISSING")
        self.assertEqual(s["event_placement"]["lidar_callback_visual_count"], "EVIDENCE_MISSING")

    def test_fs_t23_completion_reference_explicit(self):
        d, s = self._card()
        self.assertEqual(s["completion"]["expected_or_reference_rows"], None)
        d2 = fixture_run(tempfile.mkdtemp(prefix="fs23-"), BASE_LINES,
                         manifest=dict(MANIFEST, production_revision=PROVENANCE["git_head"]),
                         provenance=PROVENANCE, config_hash="c" * 64)
        r, s2 = run_eval(d2, "B0_D_CAMERA_EPOCH_APPLY_CORRECTED", ["--expected-rows", "3981"])
        self.assertEqual(s2["completion"]["expected_or_reference_rows"], 3981)
        self.assertAlmostEqual(s2["completion"]["completion_ratio"], 1.0, places=6)

    def test_fs_t24_semantic_labels_from_snapshot(self):
        _, s = self._card()
        p = s["provenance"]
        self.assertEqual(p["patch_semantics"], "SUPER_LIVO_PRE_PHASEC_PATCH_V0")
        self.assertEqual(p["iteration_semantics"], "SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0")
        self.assertNotIn("CURRENT", json.dumps(p))

    def test_fs_t25_config_sha_non_null(self):
        _, s = self._card()
        self.assertEqual(s["provenance"]["config_hash"], "c" * 64)

    def test_fs_t18_19_build_scorecard_parent(self):
        _, s = self._card(stage="A2_D_CAMERA_EPOCH_SHADOW")
        self.assertEqual(s["provenance"]["parent_stage"], "A1_D_SCHEDULER_BASE")
        _, s = self._card(stage="B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        self.assertEqual(s["provenance"]["parent_stage"], "A2_D_CAMERA_EPOCH_SHADOW")

    def test_fs_t12_gt_evaluator_same_sample_set(self):
        # synthetic fixture: estimate exactly on GT grid, prism offset zeroed
        # by construction (q = identity, body offset constant, Umeyama removes
        # the translation -> errors are the constant offset residuals).
        est = pathlib.Path(tempfile.mkdtemp(prefix="fs12-")) / "est.tum"
        lines = []
        for t in range(0, 20):
            lines.append(f"{1.0 + t * 1e-3:.9f} 0.5 0.0 0.0 0 0 0 1")
        est.write_text("\n".join(lines))
        gt = pathlib.Path(tempfile.mkdtemp(prefix="fs12gt-")) / "gt.csv"
        gt.write_text("# time, seq, stamp, x, y, z, ...\n" + "\n".join(
            f"{(1.0 + t * 1e-3) * 1e9:.0f} 0 0 0.0 0.0 0.0 0 0 0 0 0 0 0 0"
            for t in range(0, 21)))
        r = subprocess.run([sys.executable, str(GT_EVAL), "--est", str(est),
                            "--gt", str(gt)], capture_output=True, text=True,
                           timeout=120)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        # prism compensation moves the estimate by a constant body offset;
        # Umeyama removes the constant -> per-sample errors are constant;
        # RMSE/mean/median/max must be consistent with a constant vector.
        vals = {}
        for lab in (r"ATE \(m\)", "mean", "median", "max"):
            m = re.search(lab + r":\s*([-0-9.e+]+)", r.stdout)
            self.assertIsNotNone(m, r.stdout)
            vals[lab] = float(m.group(1))
        self.assertAlmostEqual(vals["mean"], vals[r"ATE \(m\)"], places=6)
        self.assertAlmostEqual(vals["median"], vals[r"ATE \(m\)"], places=6)
        self.assertAlmostEqual(vals["max"], vals[r"ATE \(m\)"], places=6)
        m = re.search(r"associated_samples:\s*(\d+)", r.stdout)
        # the official evaluator drops the first estimate when it coincides
        # with the GT start (searchsorted j == 0); 20-1 = 19 associated.
        self.assertEqual(m.group(1), "19")


class TestFinalSealRegistry(unittest.TestCase):
    def _card_file(self, stage, lines=None, provenance=None, config_hash="c" * 64,
                   dirty=False, expected_rows=None):
        prov = dict(PROVENANCE)
        if dirty:
            prov["git_dirty"] = True
        d = fixture_run(tempfile.mkdtemp(prefix="fsr-"), lines or BASE_LINES,
                        manifest=dict(MANIFEST, production_revision=prov["git_head"]),
                        provenance=prov, config_hash=config_hash)
        extra = ["--expected-rows", str(expected_rows)] if expected_rows else []
        r, s = run_eval(d, stage, extra)
        self.assertEqual(r.returncode, 0, r.stderr)
        p = pathlib.Path(tempfile.mkdtemp(prefix="fsc-")) / f"{stage}.json"
        p.write_text(json.dumps(s))
        return p

    def test_fs_t14_15_16_generated_parents(self):
        a1 = self._card_file("A1_D_SCHEDULER_BASE", dirty=True)
        a2 = self._card_file("A2_D_CAMERA_EPOCH_SHADOW")
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="fsrgen-")) / "reg.tsv"
        R.generate_registry({"A1_D_SCHEDULER_BASE": a1, "A2_D_CAMERA_EPOCH_SHADOW": a2,
                             "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        rows = {r2["Stage"]: r2 for r2 in R.generate_registry(
            {"A1_D_SCHEDULER_BASE": a1, "A2_D_CAMERA_EPOCH_SHADOW": a2,
             "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)[1]}
        self.assertEqual(rows["A1_D_SCHEDULER_BASE"]["ParentStage"], "A0_D_LEGACY_PLACEMENT_SHADOW")
        self.assertEqual(rows["A2_D_CAMERA_EPOCH_SHADOW"]["ParentStage"], "A1_D_SCHEDULER_BASE")
        self.assertEqual(rows["B0_D_CAMERA_EPOCH_APPLY_CORRECTED"]["ParentStage"],
                         "A2_D_CAMERA_EPOCH_SHADOW")
        self.assertEqual(rows["A1_D_SCHEDULER_BASE"]["Classification"],
                         "MIGRATED_HISTORICAL")
        self.assertEqual(rows["A2_D_CAMERA_EPOCH_SHADOW"]["Classification"], "VALID")
        self.assertEqual(R.validate_registry(tmp), [])

    def test_fs_t17_validator_rejects_parent_mismatch(self):
        a2 = self._card_file("A2_D_CAMERA_EPOCH_SHADOW")
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="fsr-")) / "reg.tsv"
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2,
                             "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        text = tmp.read_text().replace("A2_D_CAMERA_EPOCH_SHADOW\tA1_D_SCHEDULER_BASE",
                                       "A2_D_CAMERA_EPOCH_SHADOW\tA0_D_LEGACY_PLACEMENT_SHADOW")
        tmp.write_text(text)
        errors = R.validate_registry(tmp)
        self.assertTrue(any("parent" in e for e in errors), errors)

    def test_fs_t26_dirty_run_rejected(self):
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", dirty=True)
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("CANONICAL_RUN_DIRTY_SOURCE", str(ctx.exception))

    def test_fs_t27_row_generated_not_manual(self):
        a2 = self._card_file("A2_D_CAMERA_EPOCH_SHADOW")
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="fsr-")) / "reg.tsv"
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2}, tmp)
        baseline = tmp.read_text()
        tmp.write_text(baseline.replace("2212.82424", "999.0"))
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2}, tmp)
        self.assertEqual(tmp.read_text(), baseline)

    def test_fs_t28_real_round_trip_preserves_numerics(self):
        real = ROOT / "/home/lc/super_livo/results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T040348Z"
        if not real.exists():
            self.skipTest("real B0 run artifact absent")
        score_path = real / "out" / "visual_eval_score.json"
        gt = pathlib.Path(
            "/home/lc/super_livo/results/round13_visual_baseline/"
            "ntu_eee_01/eee_01_leica_pose.csv")
        r, s = run_eval(real, "B0_D_CAMERA_EPOCH_APPLY_CORRECTED",
                        ["--gt", str(gt), "--expected-rows", "3981"])
        self.assertEqual(r.returncode, 0, r.stderr)
        score = s
        row = R._row_from_scorecard(score)
        checks = [
            ("InitialResidualSamplesTotal", "393229"),
            ("SolverApplyCount", "1965"),
            ("SolverIterationCount", "1965"),
            ("SolverCallbackInvocations", "7758"),
            ("APE_RMSE_m", "0.133707"),
        ]
        for col, expected in checks:
            self.assertEqual(R._norm(row[col]), expected, col)
        self.assertIn(R._norm(row["InitialValidObservationRatio"]), ("0.9949", "NOT_AVAILABLE"))
        # the 040348Z artifact predates the duplicate-scan-use producer token;
        # the hard exact-once gate is exercised by fixtures and the new clean
        # runs (FS-T21 + canonical runs), not by the stale artifact.
        self.assertIn(R._norm(row["DuplicateGeometryUseEvents"]),
                      ("0", "NOT_AVAILABLE"))

    def test_fs_t29_invalid_b0_not_canonical_parent(self):
        self.assertNotIn("B0_D_CAMERA_EPOCH_APPLY_INVALID", v.CANONICAL_STAGE_PARENTS.values())
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="fsr-")) / "reg.tsv"
        header = list(R.REGISTRY_SCHEMA.keys())
        row = {h: "NOT_AVAILABLE" for h in header}
        row.update({"Stage": "B0_D_CAMERA_EPOCH_APPLY", "Classification": "VALID",
                    "ParentStage": "A2_D_CAMERA_EPOCH_SHADOW"})
        tmp.write_text("\t".join(header) + "\n" + "\t".join(row[h] for h in header) + "\n")
        errors = R.validate_registry(tmp)
        self.assertTrue(any("VALID" in e and "non-canonical" in e for e in errors), errors)


class TestAdversarialFalseClose(unittest.TestCase):
    def _reg(self, col=None, value=None, stage="B0_D_CAMERA_EPOCH_APPLY_CORRECTED"):
        a2 = self._card_file("A2_D_CAMERA_EPOCH_SHADOW")
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="adv-")) / "reg.tsv"
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2,
                             "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        if col is not None:
            lines = tmp.read_text().split("\n")
            header = lines[0].split("\t")
            ci = header.index(col)
            out = [lines[0]]
            for line in lines[1:]:
                cols = line.split("\t")
                if len(cols) == len(header) and cols[0] == stage:
                    cols[ci] = value
                out.append("\t".join(cols))
            tmp.write_text("\n".join(out))
        return R.validate_registry(tmp)

    def _card_file(self, stage, dirty=False, lines=None):
        prov = dict(PROVENANCE)
        if dirty:
            prov["git_dirty"] = True
        d = fixture_run(tempfile.mkdtemp(prefix="adv-"), lines or BASE_LINES,
                        manifest=dict(MANIFEST, production_revision=prov["git_head"]),
                        provenance=prov, config_hash="c" * 64)
        r, s = run_eval(d, stage)
        self.assertEqual(r.returncode, 0, r.stderr)
        p = pathlib.Path(tempfile.mkdtemp(prefix="advc-")) / f"{stage}.json"
        p.write_text(json.dumps(s))
        return p

    def _reject(self, name, errors, needle=None):
        if needle is None:
            needle = name
        self.assertTrue(any(needle in e for e in errors), f"{name}: {errors}")

    def test_wrong_parent_rejected(self):
        self._reject("wrong parent", self._reg("ParentStage", "A1_D_SCHEDULER_BASE"),
                     "parent")

    def test_wrong_head_rejected(self):
        self._reject("wrong HEAD", self._reg("HEAD", "zz" * 20), "HEAD")

    def test_dirty_source_rejected(self):
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", dirty=True)
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("CANONICAL_RUN_DIRTY_SOURCE", str(ctx.exception))

    def test_wrong_config_hash_rejected(self):
        a2 = self._card_file("A2_D_CAMERA_EPOCH_SHADOW")
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        score = json.loads(b0.read_text())
        score["provenance"]["config_hash"] = "d" * 64
        b0.write_text(json.dumps(score))
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2,
                                 "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("CANONICAL_CONFIG_HASH_MISMATCH", str(ctx.exception))

    def test_apply_as_residual_rejected(self):
        self._reject("Apply-as-residual", self._reg("InitialResidualSamplesTotal", "20"),
                     "initial residual total equals Apply count")

    def test_apply_as_iteration_rejected(self):
        self._reject("Apply-as-iteration", self._reg("SolverIterationCount", "20"),
                     "iteration 20 != callbacks")

    def test_missing_gt_stat_rejected(self):
        self._reject("missing GT stat", self._reg("APE_RMSE_m", "NOT_AVAILABLE"),
                     "APE_RMSE_m")

    def test_missing_event_evidence_rejected(self):
        lines = [l for l in BASE_LINES if "R14 camera-epoch Visual" not in l]
        b0 = self._card_file("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", lines=lines)
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="adv-")) / "reg.tsv"
        R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        self._reject("missing event evidence", R.validate_registry(tmp), "camera-event")

    def test_duplicate_geometry_use_rejected(self):
        self._reject("duplicate geometry use", self._reg("DuplicateGeometryUseEvents", "1"),
                     "duplicate geometry use")

    def test_current_label_rejected(self):
        self._reject("CURRENT label", self._reg("PatchPolicy", "CURRENT"), "CURRENT")

    def test_wrong_completion_reference_rejected(self):
        d = fixture_run(tempfile.mkdtemp(prefix="adv-"), BASE_LINES,
                        manifest=dict(MANIFEST, production_revision=PROVENANCE["git_head"]),
                        provenance=PROVENANCE, config_hash="c" * 64)
        r, s = run_eval(d, "B0_D_CAMERA_EPOCH_APPLY_CORRECTED", ["--expected-rows", "5000"])
        self.assertEqual(s["completion"]["completion_ratio"] is not None, True)
        self.assertAlmostEqual(s["completion"]["completion_ratio"], 3981 / 5000.0, places=6)


if __name__ == "__main__":
    unittest.main()
