#!/usr/bin/env python3
"""Prompt76 PS-T1..PS-T20 + AFC-T1..AFC-T20 — eval provenance final corrective.

Actual semantics come from the RUN (manifest + explicit semantic snapshot
file); the registry generator copies scorecard actual values (no stage
inference); the validator enforces §42/§43 hard conditions. Real-artifact
E2E uses ONLY the clean canonical pair (A2 052214Z / B0 052357Z).
"""
import hashlib
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
REG_GEN = ROOT / "scripts/super_livo/evaluation/visual_eval_registry.py"
GT = "/home/lc/super_livo/results/round13_visual_baseline/ntu_eee_01/eee_01_leica_pose.csv"
A2_RUN = "/home/lc/super_livo/results/round14_phaseA/a2_camera_epoch_shadow/20260829T052214Z"
B0_RUN = "/home/lc/super_livo/results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T052357Z"

sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
import visual_eval_score as V  # noqa: E402
import visual_eval_registry as R  # noqa: E402

SNAPSHOT = ROOT / "scripts/super_livo/evaluation/semantic_snapshot_v0.yaml"


def build_real(stage, run_dir):
    r = subprocess.run(
        [sys.executable, str(EVAL), "--stage", stage, "--run-dir",
         f"{run_dir}/out", "--manifest", f"{run_dir}/out/resolved_experiment_semantics.yaml",
         "--trajectory", f"{run_dir}/out/trajectory.tum", "--gt", GT,
         "--expected-rows", "3981", "--semantic-snapshot", str(SNAPSHOT)],
        capture_output=True, text=True, timeout=300)
    assert r.returncode == 0, r.stderr
    return json.loads((pathlib.Path(run_dir) / "out" / "visual_eval_score.json").read_text())


def fixture_manifest(profile="D_VISUAL_APPLY", apply=True):
    return {"dataset": "NTU", "sequence": "eee_01", "semantic_profile": profile,
            "visual_measurement_enabled": True,
            "visual_measurement_event": "CAMERA_EPOCH",
            "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
            "visual_measurement_exact_once": True,
            "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
            "visual_state_apply": apply,
            "visual_state_apply_connectivity": "ESTABLISHED" if apply else "NOT_ESTABLISHED",
            "config_provenance": {"dataset_calibration": "/cfg/x.yaml"},
            "production_revision": "a" * 40,
            "semantic_profile_revision": "b" * 64}


def fixture_scorecard(stage, apply=None, event=None, ts=None, conn=None,
                      payload=None, map_id=None, patch_id=None, residual_id=None,
                      iteration_id=None, extra_actual=None, drop_manifest=False,
                      provenance_missing=False):
    """Synthetic scorecard via the real build path on a fixture run."""
    tmp = pathlib.Path(tempfile.mkdtemp(prefix="ps-"))
    (tmp / "out").mkdir(parents=True)
    lines = [
        "Round11X cadence: policy=imu_fullscan raw_scans=100 geometry_updates=98 "
        "updates_per_raw_scan=0.98 map_updates=98 imu_segments=5",
        "Round11X fullscan ownership: raw_input_points=1000 "
        "pre_observe_excluded_scans=1 pre_observe_excluded_points=10 "
        "eligible_geometry_points=990 used_once=980 duplicate_use=0 "
        "never_used=10 imu_only_segments=5 raw_input_scans=100 eligible_scans=99 "
        "unique_used_scans=98 duplicate_scan_use_events=0 eligible_never_used_scans=1",
        "VISUAL_MEASUREMENT observation: frames=50 candidates=950 valid=930 "
        "rejected=20 residual_samples=10000 conservation=OK",
        "R14 initial measurement query: attempts=40 hits=38",
        "R14 initial measurement observation: frames=20 candidates=400 valid=390 "
        "rejected=10 residual_samples=4000",
        "R14 residuals_per_frame P10=180.0 P50=200.0 P90=220.0 P99=240.0 n=20",
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
        "R14 visual cpu P10=5.0 P50=10.0 P90=15.0",
        "R14 Apply attempts=20 success=20 fail=0",
        "R14 Apply skip zero_candidate=1 zero_valid_residual=0",
        "R14 solver callbacks=80 completed_iterations=80",
        "R14 solver iterations_per_apply mean=4.000 P10=4.0 P50=4.0 P90=4.0 "
        "P99=4.0 max=4.0 n=20",
    ]
    (tmp / "out" / "node_stdout.log").write_text("\n".join(lines))
    (tmp / "state.json").write_text('{"state":"SUCCESS","cleanup_verified":true}')
    times = [1.0 + i * 1e-3 for i in range(100)]
    (tmp / "out" / "trajectory.tum").write_text(
        "\n".join(f"{t:.9f} 0 0 0 0 0 0 1" for t in times))
    (tmp / "gt.csv").write_text("# t\n" + "\n".join(
        f"{t * 1e9:.0f} 0 0 0.0 0.0 0.0 0 0 0 0 0 0 0 0" for t in times))
    m = fixture_manifest(apply=True if apply is None else apply)
    if drop_manifest:
        pass
    else:
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(m))
        # Prompt77: run-bound snapshot for synthetic fixtures (RUN_EMBEDDED)
        snap = yaml.safe_load(SNAPSHOT.read_text())
        snap["production_revision"] = m.get("production_revision", "a" * 40)
        snap_file = tmp / "out" / "semantic_snapshot.yaml"
        snap_file.write_text(yaml.safe_dump(snap))
        m["semantic_snapshot_path"] = "semantic_snapshot.yaml"
        m["semantic_snapshot_sha256"] = hashlib.sha256(snap_file.read_bytes()).hexdigest()
        m["semantic_snapshot_schema_version"] = "1"
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(m))
    (tmp / "out" / "run_provenance.yaml").write_text(
        yaml.safe_dump({"run": {"git_head": "a" * 40, "git_dirty": False}}))
    (tmp / "out" / "effective_config.post_resolve.yaml.sha256").write_text("c" * 64 + "\n")
    cmd = [sys.executable, str(EVAL), "--stage", stage, "--run-dir", str(tmp / "out"),
           "--trajectory", str(tmp / "out" / "trajectory.tum"), "--gt", str(tmp / "gt.csv"),
           "--expected-rows", "100", "--semantic-snapshot", str(SNAPSHOT)]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    assert r.returncode == 0, r.stderr
    score = json.loads((tmp / "out" / "visual_eval_score.json").read_text())
    a = score["actual_semantics"]
    if event is not None: a["visual_measurement_event"] = event
    if ts is not None: a["visual_measurement_timestamp_semantics"] = ts
    if apply is not None: a["visual_state_apply"] = apply
    if conn is not None: a["visual_state_apply_connectivity"] = conn
    if payload is not None: a["camera_payload_ownership_mode"] = payload
    if map_id is not None: a["visual_map_policy_id"] = map_id
    if patch_id is not None: a["patch_policy_id"] = patch_id
    if residual_id is not None: a["residual_policy_id"] = residual_id
    if iteration_id is not None: a["iteration_policy_id"] = iteration_id
    if extra_actual: a.update(extra_actual)
    if provenance_missing:
        score["semantic_provenance"]["complete"] = False
    return tmp, score


class TestProvenanceSemantics(TestProvenanceSemantics if False else unittest.TestCase):
    """PS-T1..PS-T20 — actual-vs-expected semantics from the run."""

    def test_ps_t1_t2_actual_semantics_from_artifact(self):
        for stage, run in (("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN),
                           ("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)):
            s = build_real(stage, run)
            a = s["actual_semantics"]
            self.assertEqual(a["semantic_profile"],
                             "D_VISUAL_SHADOW" if "A2" in stage else "D_VISUAL_APPLY")
            self.assertEqual(a["visual_measurement_event"], "CAMERA_EPOCH")
            self.assertEqual(a["visual_measurement_timestamp_semantics"],
                             "CAMERA_EPOCH_PROPAGATED_STATE")
            self.assertEqual(a["visual_state_apply"], "A2" not in stage)
            self.assertEqual(a["camera_payload_ownership_mode"],
                             "RETAIN_THROUGH_MEASUREMENT")
            self.assertEqual(a["visual_map_policy_id"], "S3_SPATIAL_BALANCED_V0")
            self.assertTrue(s["semantic_provenance"]["complete"])

    def test_ps_t3_t4_scorecard_preserves_apply(self):
        self.assertIs(build_real("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
                      ["actual_semantics"]["visual_state_apply"], False)
        self.assertIs(build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
                      ["actual_semantics"]["visual_state_apply"], True)

    def test_ps_t5_event_from_run_provenance(self):
        s = build_real("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        manifest = yaml.safe_load(pathlib.Path(
            A2_RUN + "/out/resolved_experiment_semantics.yaml").read_text())
        self.assertEqual(s["actual_semantics"]["visual_measurement_event"],
                         manifest["visual_measurement_event"])
        self.assertEqual(s["provenance"]["stage_id"], "A2_D_CAMERA_EPOCH_SHADOW")

    def test_ps_t6_t7_policy_ids_from_snapshot_file(self):
        # the canonical pair binds HISTORICALLY to the immutable 31d677e
        # snapshot (Prompt77), not to today's template.
        HIST = (ROOT / "scripts/super_livo/evaluation/semantic_snapshots"
                / "31d677e13ee32fc0f57940636283ae66f9a2e3dd.yaml")
        snap = yaml.safe_load(HIST.read_text())
        s = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        a = s["actual_semantics"]
        for key in ("visual_map_policy_id", "normalize_policy_id",
                    "exposure_policy_id", "normal_policy_id",
                    "patch_policy_id", "residual_policy_id",
                    "iteration_policy_id"):
            self.assertEqual(a[key], snap["policies"][key])
        self.assertEqual(s["semantic_provenance"]["semantic_snapshot_sha256"],
                         hashlib.sha256(HIST.read_bytes()).hexdigest())
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"],
                         "HISTORICAL_REVISION_BINDING")

    def test_ps_t8_registry_semantics_equal_scorecard(self):
        a2 = build_real("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        for score in (a2, b0):
            a = score["actual_semantics"]
            row = R._row_from_scorecard(score)
            self.assertEqual(row["VisualEvent"], a["visual_measurement_event"])
            self.assertEqual(row["TimestampSemantics"],
                             a["visual_measurement_timestamp_semantics"])
            self.assertEqual(row["VisualApply"], a["visual_state_apply"])
            self.assertEqual(row["ApplyConnectivity"],
                             a["visual_state_apply_connectivity"])
            self.assertEqual(row["PayloadOwnershipMode"],
                             a["camera_payload_ownership_mode"])
            self.assertEqual(row["VisualMapPolicy"], a["visual_map_policy_id"])
            self.assertEqual(row["PatchPolicy"], a["patch_policy_id"])
            self.assertEqual(row["ResidualPolicy"], a["residual_policy_id"])
            self.assertEqual(row["IterationPolicy"], a["iteration_policy_id"])

    def test_ps_t9_t10_no_stage_inference_in_generator(self):
        src = (ROOT / "scripts/super_livo/evaluation/visual_eval_registry.py").read_text()
        # semantic columns must be read from actual_semantics; no
        # stage-conditional semantic assignment, no stage->event map, and no
        # policy-ID constants in the generator.
        self.assertNotIn('"VisualApply": stage ==', src)
        self.assertNotIn('"VisualEvent": {', src)
        self.assertNotIn('"S3_SPATIAL_BALANCED_V0"', src)
        self.assertNotIn('"SUPER_LIVO_PRE_PHASEC_PATCH_V0"', src)
        self.assertNotIn('"CAMERA_EPOCH"', src)
        # the apply_stage variable is used ONLY for the solver NOT_APPLICABLE
        # accounting, never for semantics: verify the semantic assignments
        # come from actual_semantics.
        self.assertIn('"VisualApply": a.get("visual_state_apply"', src)
        self.assertIn('"VisualEvent": a.get("visual_measurement_event"', src)
        self.assertIn('"VisualMapPolicy": a.get("visual_map_policy_id"', src)

    def test_ps_t11_t12_validator_rejects_wrong_apply(self):
        _, a2 = fixture_scorecard("A2_D_CAMERA_EPOCH_SHADOW", apply=True)
        _, b0 = fixture_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", apply=False)
        self.assertTrue(R.validate_canonical_scorecard(a2))
        self.assertTrue(R.validate_canonical_scorecard(b0))

    def test_ps_t13_t14_validator_rejects_lidar_callback_and_duplicate(self):
        from test_round14_final_seal import BASE_LINES as BL
        _, s = fixture_scorecard("A2_D_CAMERA_EPOCH_SHADOW")
        s["event_placement"]["lidar_callback_visual_count"] = 1
        self.assertTrue(R.validate_canonical_scorecard(s))
        s["event_placement"]["lidar_callback_visual_count"] = 0
        s["event_placement"]["duplicate_visual_event_count"] = 1
        self.assertTrue(R.validate_canonical_scorecard(s))

    def test_ps_t15_t16_validator_rejects_payload_missing_and_early_release(self):
        _, s = fixture_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        s["event_placement"]["payload_missing_at_measurement"] = 1
        self.assertTrue(R.validate_canonical_scorecard(s))
        _, s2 = fixture_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED")
        s2["event_placement"]["payload_released_before_measurement"] = 1
        self.assertTrue(R.validate_canonical_scorecard(s2))

    def test_ps_t17_t18_clean_canonical_e2e_passes(self):
        for stage, run in (("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN),
                           ("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)):
            score = build_real(stage, run)
            self.assertEqual(R.validate_canonical_scorecard(score), [])
            tmp = pathlib.Path(tempfile.mkdtemp()) / "r.tsv"
            R.generate_registry({stage: score}, tmp)
            self.assertEqual(R.validate_registry(tmp), [])

    def test_ps_t19_no_stale_positive_fixture(self):
        # historical pre-final-seal runs must NOT appear as positive fixtures
        # in CLOSE-gate tests (this audit test itself only names them in the
        # forbidden list, which is not a positive fixture).
        for test_src in (ROOT / "scripts/super_livo/tests").glob("test_round14*.py"):
            if test_src.name == "test_round14_provenance_seal.py":
                continue
            text = test_src.read_text()
            for stale in ("040348Z", "031213Z", "033913Z", "021933Z",
                          "012548Z", "015720Z"):
                self.assertNotIn(stale, text,
                                 f"{test_src.name}: stale artifact {stale}")

    def test_ps_t20_current_iteration_semantics(self):
        s = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        m = s["measurement_counts"]
        self.assertEqual(m["solver_apply_count"], 1965)
        self.assertEqual(m["solver_iteration_count"], 7758)
        self.assertEqual(m["solver_callback_invocations"], 7758)
        self.assertNotEqual(m["solver_iteration_count"], m["solver_apply_count"])
        self.assertEqual(m["solver_iteration_count"], m["solver_callback_invocations"])


class TestAdversarialProvenance(unittest.TestCase):
    """AFC-T1..AFC-T20 — every mutated fixture must be rejected."""

    def _reject_tsv(self, mutate, stage="B0_D_CAMERA_EPOCH_APPLY_CORRECTED"):
        a2 = build_real("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="afc-")) / "r.tsv"
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2, stage: b0}, tmp)
        lines = tmp.read_text().split("\n")
        header = lines[0].split("\t")
        out = [lines[0]]
        for line in lines[1:]:
            cols = line.split("\t")
            if len(cols) == len(header) and cols[0] == stage:
                mutate(dict(zip(header, cols)), cols, header)
            out.append("\t".join(cols))
        tmp.write_text("\n".join(out))
        return R.validate_registry(tmp)

    def _set(self, row, cols, header, key, value):
        cols[header.index(key)] = value

    def test_afc_t1_wrong_parent(self):
        e = self._reject_tsv(lambda r, c, h: self._set(r, c, h, "ParentStage",
                                                        "A1_D_SCHEDULER_BASE"))
        self.assertTrue(any("parent" in x for x in e), e)

    def test_afc_t2_wrong_head(self):
        e = self._reject_tsv(lambda r, c, h: self._set(r, c, h, "HEAD", "z" * 40))
        self.assertTrue(any("HEAD" in x for x in e), e)

    def test_afc_t3_dirty(self):
        e = self._reject_tsv(lambda r, c, h: self._set(r, c, h, "GitDirty", "1"))
        self.assertTrue(any("dirty" in x for x in e), e)

    def test_afc_t4_wrong_config_hash(self):
        e = self._reject_tsv(lambda r, c, h: self._set(r, c, h, "ConfigHash", "d" * 64))
        self.assertTrue(any("config hash" in x for x in e), e)

    def test_afc_t5_wrong_visual_apply(self):
        e = self._reject_tsv(lambda r, c, h: self._set(r, c, h, "VisualApply", "false"))
        self.assertTrue(any("semantic contract" in x for x in e), e)

    def test_afc_t6_wrong_visual_event(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "VisualEvent", "FULL_LIDAR_OBSERVE_CALLBACK"))
        self.assertTrue(any("semantic contract" in x for x in e), e)

    def test_afc_t7_wrong_timestamp_semantics(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "TimestampSemantics", "LEGACY"))
        self.assertTrue(any("semantic contract" in x for x in e), e)

    def test_afc_t8_wrong_map_policy(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "VisualMapPolicy", "S3_SPATIAL_BALANCED_V9"))
        self.assertTrue(any(e), e)

    def test_afc_t9_wrong_patch_policy(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "PatchPolicy", "CURRENT"))
        self.assertTrue(any("CURRENT" in x for x in e), e)

    def test_afc_t10_wrong_residual_policy(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "ResidualPolicy", "DEFAULT"))
        self.assertTrue(any(e), e)

    def test_afc_t11_lidar_callback_gt0(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "LidarCallbackVisualCount", "1"))
        self.assertTrue(any(e), e)

    def test_afc_t12_duplicate_event_gt0(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "DuplicateVisualEventCount", "1"))
        self.assertTrue(any(e), e)

    def test_afc_t13_payload_missing_gt0(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "PayloadMissing", "1"))
        self.assertTrue(any(e), e)

    def test_afc_t14_payload_early_release_gt0(self):
        e = self._reject_tsv(lambda r, c, h: self._set(
            r, c, h, "PayloadReleasedBeforeMeasurement", "1"))
        self.assertTrue(any(e), e)

    def test_afc_t15_a2_apply_gt0(self):
        a2 = build_real("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        a2["measurement_counts"]["solver_apply_count"] = 1
        a2["measurement_counts"]["solver_apply_success"] = 1
        a2["measurement_counts"]["solver_apply_fail"] = 0
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2}, None)
        self.assertIn("A2_APPLY_ATTEMPTS_MUST_BE_ZERO", str(ctx.exception))

    def test_afc_t16_b0_apply_zero(self):
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        b0["measurement_counts"]["solver_apply_count"] = 0
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("B0_APPLY_ATTEMPTS_REQUIRED", str(ctx.exception))

    def test_afc_t17_apply_identity_fail(self):
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        b0["measurement_counts"]["solver_apply_success"] = 1900
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("B0_APPLY_IDENTITY_FAIL", str(ctx.exception))

    def test_afc_t18_missing_gt_stat(self):
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        b0["accuracy"]["ape_translation_mean_m"] = "NOT_AVAILABLE"
        tmp = pathlib.Path(tempfile.mkdtemp()) / "r.tsv"
        R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        e = R.validate_registry(tmp)
        self.assertTrue(any("APE_Mean_m" in x for x in e), e)

    def test_afc_t19_apply_as_iteration(self):
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        b0["measurement_counts"]["solver_iteration_count"] = 1965
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, None)
        self.assertIn("SOLVER_ITERATION_CALLBACK_IDENTITY_FAIL", str(ctx.exception))

    def test_afc_t20_historical_invalid_b0_not_canonical(self):
        b0 = build_real("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        b0["provenance"]["stage_id"] = "B0_D_CAMERA_EPOCH_APPLY"
        b0["provenance"]["parent_stage"] = "UNREGISTERED_STAGE"
        b0["provenance"]["git_dirty"] = 0
        tmp = pathlib.Path(tempfile.mkdtemp()) / "r.tsv"
        R.generate_registry({"B0_D_CAMERA_EPOCH_APPLY": b0}, tmp)
        e = R.validate_registry(tmp)
        self.assertTrue(any("VALID" in x and "non-canonical" in x for x in e), e)


if __name__ == "__main__":
    unittest.main()
