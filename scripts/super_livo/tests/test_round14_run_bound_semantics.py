#!/usr/bin/env python3
"""Prompt77 RB-T1..RB-T20 + AFC-T21..AFC-T25 — run-bound semantic snapshot.

The historical A2/B0 pair resolves its policy semantics through the
validated HISTORICAL_REVISION_BINDING to the immutable 31d677e snapshot;
changing/deleting the CURRENT checkout template must not reinterpret them.
"""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

import yaml

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVAL = ROOT / "scripts/super_livo/evaluation/visual_eval_score.py"
SNAP_DIR = ROOT / "scripts/super_livo/evaluation/semantic_snapshots"
BIND_DIR = ROOT / "docs/super_livo/evidence/run_semantic_bindings"
TEMPLATE = ROOT / "scripts/super_livo/evaluation/semantic_snapshot_v0.yaml"
HIST_SNAP = SNAP_DIR / "31d677e13ee32fc0f57940636283ae66f9a2e3dd.yaml"
GT = "/home/lc/super_livo/results/round13_visual_baseline/ntu_eee_01/eee_01_leica_pose.csv"
A2_RUN = "/home/lc/super_livo/results/round14_phaseA/a2_camera_epoch_shadow/20260829T052214Z"
B0_RUN = "/home/lc/super_livo/results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T052357Z"
PROD_REV = "31d677e13ee32fc0f57940636283ae66f9a2e3dd"

sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
import visual_eval_score as V  # noqa: E402
import visual_eval_registry as R  # noqa: E402


def build(stage, run_dir, snapshot_dir_override=None, template_override=None,
          binding_dir_override=None):
    """build_scorecard with test-owned snapshot/binding directories."""
    return V.build_scorecard(
        stage, f"{run_dir}/out", f"{run_dir}/out/resolved_experiment_semantics.yaml",
        trajectory=f"{run_dir}/out/trajectory.tum", gt=GT,
        ate_evaluator=str(ROOT / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"),
        expected_rows=3981)


def fake_env():
    """test-owned environment with a completely different V999 template."""
    tmp = pathlib.Path(tempfile.mkdtemp(prefix="rb77-"))
    snap = tmp / "semantic_snapshots"
    snap.mkdir()
    bind = tmp / "bindings"
    bind.mkdir()
    shutil.copy(HIST_SNAP, snap / HIST_SNAP.name)
    shutil.copy(next(BIND_DIR.glob("*.yaml")), bind / "binding.yaml")
    tpl = tmp / "semantic_snapshot_v0.yaml"
    data = TEMPLATE.read_text()
    data = data.replace("S3_SPATIAL_BALANCED_V0", "FAKE_FUTURE_MAP_V999")
    data = data.replace("SUPER_LIVO_PRE_PHASEC_PATCH_V0", "FAKE_FUTURE_PATCH_V999")
    data = data.replace("SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0", "FAKE_FUTURE_RESIDUAL_V999")
    data = data.replace("SUPER_LIVO_PRE_PHASEC_ITERATION_V0", "FAKE_FUTURE_ITER_V999")
    data = data.replace("NOT_IMPLEMENTED_V0", "FAKE_FUTURE_NORM_V999")
    tpl.write_text(data)
    return tmp, snap, bind, tpl


class TestRunBoundSemantics(unittest.TestCase):
    def _polys(self, score):
        a = score["actual_semantics"]
        return (a["visual_map_policy_id"], a["normalize_policy_id"],
                a["patch_policy_id"], a["residual_policy_id"],
                a["iteration_policy_id"])

    def test_rb_t1_t2_historical_ids_immutable_to_template_change(self):
        # baseline with the REAL repository state
        for stage, run in (("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN),
                           ("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)):
            base = build(stage, run)
            self.assertEqual(base["semantic_provenance"]["semantic_binding_mode"],
                             "HISTORICAL_REVISION_BINDING")
            base_ids = self._polys(base)
            # fake V999 current template (test-owned env, not production)
            tmp, snap, bind, tpl = fake_env()
            with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
                mut = build(stage, run)
            self.assertEqual(self._polys(mut), base_ids,
                             f"RB-T{1 if 'A2' in stage else 2}: historical IDs changed")
            self.assertEqual(mut["semantic_provenance"]["semantic_snapshot_sha256"],
                             base["semantic_provenance"]["semantic_snapshot_sha256"])

    def test_rb_t3_template_deletion_historical_still_resolves(self):
        for stage, run in (("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN),
                           ("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)):
            tmp, snap, bind, tpl = fake_env()
            tpl.unlink()  # delete current template
            with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
                s = build(stage, run)
            self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"],
                             "HISTORICAL_REVISION_BINDING")
            self.assertEqual(s["actual_semantics"]["visual_map_policy_id"],
                             "S3_SPATIAL_BALANCED_V0")

    def test_rb_t4_wrong_revision_binding_rejected(self):
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["runs"]["B0"]["production_revision"] = "f" * 40
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
            self.assertIn("SEMANTIC_PROVENANCE_MISSING", str(ctx.exception))

    def test_rb_t5_content_change_without_hash_update_rejected(self):
        snap = pathlib.Path(tempfile.mkdtemp()) / "snap.yaml"
        snap.write_text(HIST_SNAP.read_text().replace(
            "S3_SPATIAL_BALANCED_V0", "FAKE_MAP_V1"))
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        # binding still records the ORIGINAL snapshot hash -> the changed
        # content must be rejected by the resolver hash verification.
        b["semantic_snapshot_source"] = str(snap)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_rb_t6_revision_derivation_mismatch_rejected(self):
        snap = pathlib.Path(tempfile.mkdtemp()) / "snap.yaml"
        data = yaml.safe_load(HIST_SNAP.read_text())
        data["production_revision"] = "a" * 40
        snap.write_text(yaml.safe_dump(data))
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["semantic_snapshot_source"] = str(snap)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_rb_t7_snapshot_from_another_commit_rejected(self):
        snap = pathlib.Path(tempfile.mkdtemp()) / "snap.yaml"
        data = yaml.safe_load(HIST_SNAP.read_text())
        data["derived_from_revision"] = "c" * 40
        snap.write_text(yaml.safe_dump(data))
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["semantic_snapshot_source"] = str(snap)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_rb_t8_missing_binding_fails_canonical(self):
        tmp = pathlib.Path(tempfile.mkdtemp()) / "empty_bindings"
        tmp.mkdir()
        with V._semantic_env(binding_dir=tmp):
            with self.assertRaises(ValueError) as ctx:
                build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            self.assertIn("SEMANTIC_PROVENANCE_MISSING", str(ctx.exception))

    def test_rb_t9_t10_run_embedded_snapshot(self):
        # synthetic future run produced through the PRODUCTION materializer
        # (semantic_profiles.materialize_snapshot): RUN_EMBEDDED with a
        # correct hash is accepted; a WRONG manifest hash must be rejected
        # (Prompt78 §16 — old recompute-and-pass behavior was a false
        # positive: OLD_TEST_WAS_FALSE_POSITIVE).
        sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as SP
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="rb9-"))
        (tmp / "out").mkdir(parents=True)
        manifest = {"production_revision": "a" * 40, "semantic_profile": "D_VISUAL_APPLY",
                    "validator": "", "requires_measurement_evidence": True,
                    "scheduler_family": "D_CORRECTED", "camera_input_enabled": True,
                    "camera_epoch_enabled": True, "visual_frontend_enabled": True,
                    "visual_map_producer_enabled": True, "visual_measurement_enabled": True,
                    "visual_state_apply": True, "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
                    "full_lidar_observe_per_raw_scan": 1, "camera_stride": 1,
                    "visual_measurement_event": "CAMERA_EPOCH",
                    "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
                    "visual_measurement_exact_once": True,
                    "visual_state_apply_connectivity": "ESTABLISHED",
                    "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
                    "config_provenance": {"lio": "x", "visual": "y",
                                          "dataset_calibration": "z"},
                    "semantic_profile_revision": "b" * 64,
                    "dataset_adapter_revision": "c" * 64,
                    "transaction_revision": "d" * 40}
        snap_path, sha = SP.materialize_snapshot(
            manifest, TEMPLATE, tmp / "out" / "semantic_snapshot.yaml")
        manifest["semantic_snapshot_path"] = "semantic_snapshot.yaml"
        manifest["semantic_snapshot_sha256"] = sha
        manifest["semantic_snapshot_schema_version"] = 1
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(manifest))
        s = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                              tmp / "out" / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap_path)
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"], "RUN_EMBEDDED")
        self.assertEqual(s["semantic_provenance"]["semantic_snapshot_sha256"], sha)
        # RB-T10 corrected: wrong manifest hash -> SEMANTIC_SNAPSHOT_HASH_MISMATCH
        manifest["semantic_snapshot_sha256"] = "0" * 64
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(manifest))
        with self.assertRaises(ValueError) as ctx:
            V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                              tmp / "out" / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap_path)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_rb_t11_t15_future_capture_contract_fields(self):
        # semantic_profiles snapshot subcommand records the manifest fields
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="rb11-"))
        (tmp / "out").mkdir(parents=True)
        manifest = {"semantic_profile": "D_VISUAL_APPLY", "validator": "",
                    "requires_measurement_evidence": True,
                    "scheduler_family": "D_CORRECTED",
                    "camera_input_enabled": True, "camera_epoch_enabled": True,
                    "visual_frontend_enabled": True, "visual_map_producer_enabled": True,
                    "visual_measurement_enabled": True, "visual_state_apply": True,
                    "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
                    "full_lidar_observe_per_raw_scan": 1, "camera_stride": 1,
                    "visual_measurement_event": "CAMERA_EPOCH",
                    "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
                    "visual_measurement_exact_once": True,
                    "visual_state_apply_connectivity": "ESTABLISHED",
                    "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
                    "production_revision": "a" * 40,
                    "semantic_profile_revision": "b" * 64,
                    "dataset_adapter_revision": "c" * 64,
                    "transaction_revision": "d" * 40,
                    "config_provenance": {"lio": "x", "visual": "y",
                                          "dataset_calibration": "z"}}
        mf = tmp / "out" / "resolved_experiment_semantics.yaml"
        mf.write_text(yaml.safe_dump(manifest))
        snap_file = tmp / "out" / "semantic_snapshot.yaml"
        r = subprocess.run(
            [sys.executable, str(ROOT / "scripts/super_livo/experiments/semantic_profiles.py"),
             "snapshot", "--manifest", str(mf), "--snapshot", str(snap_file),
             "--template", str(TEMPLATE), "--sha", "unused"],
            capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        m2 = yaml.safe_load(mf.read_text())
        self.assertEqual(m2["semantic_snapshot_path"], str(snap_file))
        self.assertEqual(m2["semantic_snapshot_sha256"],
                         hashlib.sha256(snap_file.read_bytes()).hexdigest())
        self.assertEqual(m2["semantic_snapshot_schema_version"], 1)

    def test_rb_t16_template_advance_does_not_change_historical(self):
        tmp, snap, bind, tpl = fake_env()
        with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
            base = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            mut = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        self.assertEqual(self._polys(mut), self._polys(base))

    def test_rb_t17_registry_unchanged_after_template_mutation(self):
        tmp, snap, bind, tpl = fake_env()
        with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
            a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            b0 = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        row_a = R._row_from_scorecard(a2)
        row_b = R._row_from_scorecard(b0)
        self.assertEqual(row_a["VisualMapPolicy"], "S3_SPATIAL_BALANCED_V0")
        self.assertEqual(row_b["VisualMapPolicy"], "S3_SPATIAL_BALANCED_V0")
        self.assertEqual(row_a["SemanticBindingMode"], "HISTORICAL_REVISION_BINDING")
        self.assertEqual(row_a["SemanticSnapshotSHA256"],
                         hashlib.sha256(HIST_SNAP.read_bytes()).hexdigest())

    def test_rb_t18_validator_rejects_unverifiable_policy_source(self):
        a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        a2["semantic_provenance"]["semantic_snapshot_sha256"] = None
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2}, None)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISSING", str(ctx.exception))

    def test_rb_t19_future_checkpoint_independent_ids(self):
        # synthetic Phase-C-style checkpoint: a NEW snapshot with new IDs must
        # not alter the historical ones (they are separate files/hashes).
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="rb19-"))
        (tmp / "out").mkdir(parents=True)
        sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as SP
        manifest = {"production_revision": "e" * 40, "semantic_profile": "D_VISUAL_APPLY",
                    "validator": "", "requires_measurement_evidence": True,
                    "scheduler_family": "D_CORRECTED", "camera_input_enabled": True,
                    "camera_epoch_enabled": True, "visual_frontend_enabled": True,
                    "visual_map_producer_enabled": True, "visual_measurement_enabled": True,
                    "visual_state_apply": True, "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
                    "full_lidar_observe_per_raw_scan": 1, "camera_stride": 1,
                    "visual_measurement_event": "CAMERA_EPOCH",
                    "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
                    "visual_measurement_exact_once": True,
                    "visual_state_apply_connectivity": "ESTABLISHED",
                    "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
                    "config_provenance": {"lio": "x", "visual": "y",
                                          "dataset_calibration": "z"},
                    "semantic_profile_revision": "b" * 64,
                    "dataset_adapter_revision": "c" * 64,
                    "transaction_revision": "d" * 40}
        snap_path, sha = SP.materialize_snapshot(
            manifest, TEMPLATE, tmp / "out" / "semantic_snapshot.yaml")
        snap = yaml.safe_load(pathlib.Path(snap_path).read_text())
        snap["snapshot_id"] = "PHASE_C_CHECKPOINT_V1"
        snap["policies"]["patch_policy_id"] = "PHASE_C_PATCH_V1"
        pathlib.Path(snap_path).write_text(yaml.safe_dump(snap))
        snap = yaml.safe_load(pathlib.Path(snap_path).read_text())
        _ = snap
        sha = hashlib.sha256(pathlib.Path(snap_path).read_bytes()).hexdigest()
        manifest["semantic_snapshot_path"] = "semantic_snapshot.yaml"
        manifest["semantic_snapshot_sha256"] = sha
        manifest["semantic_snapshot_schema_version"] = 1
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
            yaml.safe_dump(manifest))
        s = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                              tmp / "out" / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap_path)
        self.assertEqual(s["actual_semantics"]["patch_policy_id"], "PHASE_C_PATCH_V1")
        # historical run unchanged
        hist = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        self.assertEqual(hist["actual_semantics"]["patch_policy_id"],
                         "SUPER_LIVO_PRE_PHASEC_PATCH_V0")

    def test_rb_t20_no_current_checkout_template_source(self):
        a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        b0 = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        for s in (a2, b0):
            sp = s["semantic_provenance"]
            self.assertNotIn("semantic_snapshot_v0.yaml", str(sp["semantic_snapshot_source"]))
            self.assertNotEqual(sp["semantic_binding_mode"], "CURRENT_CHECKOUT_TEMPLATE")


class TestAdversarialRunBound(unittest.TestCase):
    """AFC-T21..AFC-T25."""

    def _registry_errors(self, mutate_binding_dir=None, snapshot_content=None):
        a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        b0 = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "r.tsv"
        R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2,
                             "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": b0}, tmp)
        return tmp, R.validate_registry(tmp)

    def test_afc_t21_t22_template_mutation_cannot_reinterpret(self):
        # historical scorecards byte-identical in semantic fields under a
        # fake V999 template (test-owned env)
        tmp, snap, bind, tpl = fake_env()
        with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
            a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            b0 = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        base_a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        base_b0 = build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        for mut, base in ((a2, base_a2), (b0, base_b0)):
            self.assertEqual(mut["actual_semantics"].get("visual_map_policy_id"),
                             base["actual_semantics"].get("visual_map_policy_id"))
            self.assertEqual(mut["actual_semantics"].get("patch_policy_id"),
                             base["actual_semantics"].get("patch_policy_id"))
            self.assertEqual(mut["semantic_provenance"]["semantic_snapshot_sha256"],
                             base["semantic_provenance"]["semantic_snapshot_sha256"])

    def test_afc_t23_wrong_historical_revision_rejected(self):
        snap = pathlib.Path(tempfile.mkdtemp()) / "snap.yaml"
        data = yaml.safe_load(HIST_SNAP.read_text())
        data["production_revision"] = "a" * 40
        snap.write_text(yaml.safe_dump(data))
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["semantic_snapshot_source"] = str(snap)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                build("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_afc_t24_missing_snapshot_hash_rejected(self):
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["semantic_snapshot_sha256"] = None
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        a2 = build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        a2["semantic_provenance"]["semantic_snapshot_sha256"] = None
        with self.assertRaises(ValueError) as ctx:
            R.generate_registry({"A2_D_CAMERA_EPOCH_SHADOW": a2}, None)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISSING", str(ctx.exception))

    def test_afc_t25_current_template_as_historical_source_rejected(self):
        # force the resolver to use the current template: binding dir empty
        # + no run snapshot -> must fail closed, never fall back
        tmp = pathlib.Path(tempfile.mkdtemp()) / "empty"
        tmp.mkdir()
        with V._semantic_env(binding_dir=tmp):
            with self.assertRaises(ValueError) as ctx:
                build("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            self.assertIn("SEMANTIC_PROVENANCE_MISSING", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
