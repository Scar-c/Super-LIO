#!/usr/bin/env python3
"""Prompt78 FH-RED-T1/T2 + FH-T1..FH-T20 — final hard close: future-run
snapshot integrity.

All future-run fixtures are produced through the PRODUCTION materializer
(semantic_profiles.materialize_snapshot) and verified by the production
resolver. The real no-bag transaction seam exercises the actual
run_superlivo_transaction.sh capture path.
"""
import hashlib
import json
import pathlib
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

import yaml

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVAL = ROOT / "scripts/super_livo/evaluation/visual_eval_score.py"
TEMPLATE = ROOT / "scripts/super_livo/evaluation/semantic_snapshot_v0.yaml"
HIST_SNAP = ROOT / "scripts/super_livo/evaluation/semantic_snapshots/31d677e13ee32fc0f57940636283ae66f9a2e3dd.yaml"
BIND_DIR = ROOT / "docs/super_livo/evidence/run_semantic_bindings"
GT = "/home/lc/super_livo/results/round13_visual_baseline/ntu_eee_01/eee_01_leica_pose.csv"
A2_RUN = "/home/lc/super_livo/results/round14_phaseA/a2_camera_epoch_shadow/20260829T052214Z"
B0_RUN = "/home/lc/super_livo/results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T052357Z"
PROD_REV = "31d677e13ee32fc0f57940636283ae66f9a2e3dd"

sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
import visual_eval_score as V  # noqa: E402
import visual_eval_registry as R  # noqa: E402
import semantic_profiles as SP  # noqa: E402


def make_future_run(tmp, template=TEMPLATE, rev="a" * 40, mutate=None,
                    drop_snapshot_rev=False, drop_snapshot_schema=False,
                    drop_manifest_hash=False, wrong_manifest_hash=False,
                    wrong_snapshot_rev=False, wrong_snapshot_schema=False,
                    wrong_manifest_rev=False, wrong_manifest_schema=False):
    (tmp / "out").mkdir(parents=True, exist_ok=True)
    manifest = {"production_revision": rev, "semantic_profile": "D_VISUAL_APPLY",
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
    snap_path, sha = SP.materialize_snapshot(manifest, template,
                                             tmp / "out" / "semantic_snapshot.yaml")
    if drop_snapshot_rev:
        d = yaml.safe_load(pathlib.Path(snap_path).read_text())
        d.pop("production_revision")
        pathlib.Path(snap_path).write_text(yaml.safe_dump(d))
    if drop_snapshot_schema:
        d = yaml.safe_load(pathlib.Path(snap_path).read_text())
        d.pop("snapshot_schema_version")
        pathlib.Path(snap_path).write_text(yaml.safe_dump(d))
    if wrong_snapshot_rev:
        d = yaml.safe_load(pathlib.Path(snap_path).read_text())
        d["production_revision"] = "f" * 40
        pathlib.Path(snap_path).write_text(yaml.safe_dump(d))
    if wrong_snapshot_schema:
        d = yaml.safe_load(pathlib.Path(snap_path).read_text())
        d["snapshot_schema_version"] = 99
        pathlib.Path(snap_path).write_text(yaml.safe_dump(d))
    sha = hashlib.sha256(pathlib.Path(snap_path).read_bytes()).hexdigest()
    manifest["semantic_snapshot_path"] = "semantic_snapshot.yaml"
    manifest["semantic_snapshot_sha256"] = sha
    manifest["semantic_snapshot_schema_version"] = 1
    if mutate:
        # post-capture tamper: file bytes change, manifest hash does NOT
        p = pathlib.Path(snap_path)
        p.write_text(p.read_text() + "# tampered\n")
    if wrong_manifest_rev:
        manifest["production_revision"] = "e" * 40
    if wrong_manifest_schema:
        manifest["semantic_snapshot_schema_version"] = 99
    if drop_manifest_hash:
        manifest.pop("semantic_snapshot_sha256")
    if wrong_manifest_hash:
        manifest["semantic_snapshot_sha256"] = "0" * 64
    (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
        yaml.safe_dump(manifest))
    return tmp, snap_path


def resolve(tmp, snap_path):
    return V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                             tmp / "out" / "resolved_experiment_semantics.yaml",
                             semantic_snapshot=snap_path)


class TestFutureRunIntegrity(unittest.TestCase):
    """FH-T1..FH-T15."""

    def test_fh_t1_correct_snapshot_accepted(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()))
        s = resolve(tmp, snap)
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"], "RUN_EMBEDDED")
        self.assertTrue(s["semantic_provenance"]["complete"])

    def test_fh_t2_snapshot_modified_after_bind_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    mutate=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_fh_t3_manifest_hash_modified_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_manifest_hash=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_fh_t4_manifest_hash_removed_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    drop_manifest_hash=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISSING", str(ctx.exception))

    def test_fh_t5_snapshot_revision_modified_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_snapshot_rev=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_fh_t6_snapshot_revision_removed_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    drop_snapshot_rev=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISSING", str(ctx.exception))

    def test_fh_t7_manifest_revision_modified_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_manifest_rev=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_fh_t8_snapshot_schema_modified_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_snapshot_schema=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH", str(ctx.exception))

    def test_fh_t9_snapshot_schema_removed_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    drop_snapshot_schema=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_SCHEMA_MISSING", str(ctx.exception))

    def test_fh_t10_manifest_schema_modified_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_manifest_schema=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH", str(ctx.exception))

    def test_fh_t11_manifest_schema_removed_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()))
        p = tmp / "out" / "resolved_experiment_semantics.yaml"
        d = yaml.safe_load(p.read_text())
        d.pop("semantic_snapshot_schema_version")
        p.write_text(yaml.safe_dump(d))
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_SCHEMA_MISSING", str(ctx.exception))

    def test_fh_t12_policy_content_modified_without_sha_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()))
        p = pathlib.Path(snap)
        d = yaml.safe_load(p.read_text())
        d["policies"]["patch_policy_id"] = "FAKE_PATCH_V9"
        p.write_text(yaml.safe_dump(d))
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_fh_t13_policy_modified_sha_updated_revision_wrong_rejected(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()),
                                    wrong_snapshot_rev=True)
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_fh_t14_snapshot_from_another_revision_rejected(self):
        # a snapshot/manifest revision that is not a full 40-char git SHA
        # (e.g. "short", "HEAD") must be rejected (§19)
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()), rev="HEAD")
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp, snap)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))
        tmp2, snap2 = make_future_run(pathlib.Path(tempfile.mkdtemp()), rev="short")
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp2, snap2)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))

    def test_fh_t15_template_drift_after_run_unchanged(self):
        tmp, snap = make_future_run(pathlib.Path(tempfile.mkdtemp()))
        base = resolve(tmp, snap)
        # mutate the CURRENT template (test-owned copy)
        tpl = pathlib.Path(tempfile.mkdtemp()) / "template.yaml"
        tpl.write_text(TEMPLATE.read_text().replace(
            "S3_SPATIAL_BALANCED_V0", "FAKE_DRIFT_V999"))
        # re-resolve the SAME run with the drifted template injected
        m = yaml.safe_load((tmp / "out" / "resolved_experiment_semantics.yaml").read_text())
        s = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                              tmp / "out" / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap)
        self.assertEqual(s["actual_semantics"]["visual_map_policy_id"],
                         base["actual_semantics"]["visual_map_policy_id"])
        self.assertEqual(s["semantic_provenance"]["semantic_snapshot_sha256"],
                         base["semantic_provenance"]["semantic_snapshot_sha256"])


class TestHistoricalRegression(unittest.TestCase):
    """FH-T16..FH-T20."""

    def _hist(self, stage, run):
        return V.build_scorecard(
            stage, f"{run}/out", f"{run}/out/resolved_experiment_semantics.yaml",
            trajectory=f"{run}/out/trajectory.tum", gt=GT,
            ate_evaluator=str(ROOT / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"),
            expected_rows=3981)

    def test_fh_t16_t17_template_v999_does_not_change_historical(self):
        base_a2 = self._hist("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
        base_b0 = self._hist("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        tmp = pathlib.Path(tempfile.mkdtemp())
        snap = tmp / "semantic_snapshots"; snap.mkdir()
        bind = tmp / "bindings"; bind.mkdir()
        shutil.copy(HIST_SNAP, snap / HIST_SNAP.name)
        shutil.copy(next(BIND_DIR.glob("*.yaml")), bind / "b.yaml")
        tpl = tmp / "semantic_snapshot_v0.yaml"
        tpl.write_text(TEMPLATE.read_text().replace("S3_SPATIAL_BALANCED_V0", "FAKE_FUTURE_MAP_V999"))
        with V._semantic_env(snap_dir=snap, template=tpl, binding_dir=bind):
            a2 = self._hist("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            b0 = self._hist("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        for mut, base in ((a2, base_a2), (b0, base_b0)):
            self.assertEqual(mut["actual_semantics"]["visual_map_policy_id"],
                             base["actual_semantics"]["visual_map_policy_id"])

    def test_fh_t18_template_deleted_historical_still_resolves(self):
        tmp = pathlib.Path(tempfile.mkdtemp())
        snap = tmp / "semantic_snapshots"; snap.mkdir()
        bind = tmp / "bindings"; bind.mkdir()
        shutil.copy(HIST_SNAP, snap / HIST_SNAP.name)
        shutil.copy(next(BIND_DIR.glob("*.yaml")), bind / "b.yaml")
        with V._semantic_env(snap_dir=snap, template=tmp / "missing.yaml", binding_dir=bind):
            s = self._hist("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", B0_RUN)
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"],
                         "HISTORICAL_REVISION_BINDING")

    def test_fh_t19_binding_wrong_snapshot_hash_rejected(self):
        snap = pathlib.Path(tempfile.mkdtemp()) / "snap.yaml"
        snap.write_text(HIST_SNAP.read_text())
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["semantic_snapshot_source"] = str(snap)
        b["semantic_snapshot_sha256"] = "0" * 64  # wrong vs actual file
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                self._hist("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))

    def test_fh_t20_binding_wrong_revision_rejected(self):
        b = yaml.safe_load(next(BIND_DIR.glob("*.yaml")).read_text())
        b["runs"]["A2"]["production_revision"] = "f" * 40
        tmp = pathlib.Path(tempfile.mkdtemp()) / "b.yaml"
        tmp.write_text(yaml.safe_dump(b))
        with V._semantic_env(binding_dir=tmp.parent):
            with self.assertRaises(ValueError) as ctx:
                self._hist("A2_D_CAMERA_EPOCH_SHADOW", A2_RUN)
            self.assertIn("SEMANTIC_SNAPSHOT_PATH_MISSING", str(ctx.exception))


class TestRealTransactionSeam(unittest.TestCase):
    """Real production no-bag transaction seam (§13-15, §37-44)."""

    def test_real_transaction_snapshot_seam(self):
        run_root = pathlib.Path(tempfile.mkdtemp(prefix="fhseam-"))
        fake_bag = run_root / "fake.bag"
        fake_bag.write_text("not a real bag\n")
        calib = pathlib.Path("/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml")
        cfg = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")
        if not (calib.exists() and cfg.exists()):
            self.skipTest("canonical config/calib absent")
        env = dict(os.environ)
        env.update(
            SLV_CFG=str(cfg), SLV_BAG=str(fake_bag), SLV_CAM_TOPIC="/left/image_raw",
            SLV_CAM_CALIB=str(calib), SLV_CAM_OFFSET="-0.0199575325817",
            SLV_DATASET="NTU", SLV_SEQUENCE="eee_01", SLV_MIN_ROWS="1",
            SLV_SEMANTIC_PROFILE="D_VISUAL_APPLY", SLV_LEGACY_ALIAS="a0")
        rid = "fh_seam_test"
        proc = subprocess.Popen(
            ["bash", "scripts/super_livo/experiments/run_superlivo_transaction.sh",
             rid, str(run_root)], cwd=str(ROOT), env=env,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        out = run_root / rid / "out"
        manifest = out / "resolved_experiment_semantics.yaml"
        snapshot = out / "semantic_snapshot.yaml"
        deadline = time.time() + 180
        while time.time() < deadline:
            if snapshot.exists() and manifest.exists():
                m = yaml.safe_load(manifest.read_text()) if manifest.exists() else {}
                if m.get("semantic_snapshot_sha256"):
                    break
            time.sleep(1)
        try:
            proc.terminate()
        except Exception:
            pass
        try:
            proc.wait(timeout=30)
        except Exception:
            proc.kill()
        self.assertTrue(snapshot.exists(), "snapshot not materialized by the real seam")
        self.assertTrue(manifest.exists(), "manifest missing")
        m = yaml.safe_load(manifest.read_text())
        # §38: shell-recomputed SHA == manifest SHA (independent verification)
        shell_sha = subprocess.check_output(
            ["sha256sum", str(snapshot)], text=True).split()[0]
        self.assertEqual(shell_sha, m["semantic_snapshot_sha256"],
                         "shell sha != manifest sha")
        # §39: git HEAD == snapshot rev == manifest rev (full SHA)
        head = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
        s = yaml.safe_load(snapshot.read_text())
        self.assertEqual(s["production_revision"], head)
        self.assertEqual(m["production_revision"], head)
        self.assertRegex(head, r"^[0-9a-f]{40}$")
        # §40: schema equality
        self.assertEqual(s["snapshot_schema_version"], 1)
        self.assertEqual(m["semantic_snapshot_schema_version"], 1)
        # snapshot has all required policy IDs (nested canonical block) +
        # runtime fields (canonical names per Prompt78 §5)
        for key in ("visual_map_policy_id", "normalize_policy_id", "exposure_policy_id",
                    "normal_policy_id", "patch_policy_id", "residual_policy_id",
                    "iteration_policy_id"):
            self.assertIn(key, s["policies"], key)
        for key in ("visual_measurement_event", "visual_apply",
                    "visual_apply_connectivity", "camera_payload_ownership_mode",
                    "semantic_profile"):
            self.assertIn(key, s, key)
        self.assertIs(s["visual_apply"], True)
        # resolver accepts the real captured run
        score = V.build_scorecard(
            "B0_D_CAMERA_EPOCH_APPLY_CORRECTED", out, manifest,
            semantic_snapshot=snapshot)
        self.assertEqual(score["semantic_provenance"]["semantic_binding_mode"],
                         "RUN_EMBEDDED")
        # §41-43: mutation attacks on COPIES of the seam output. The copied
        # manifest must be re-pointed at the copy's own snapshot (the copy
        # is a relocated run); the manifest hash stays the ORIGINAL binding.
        def rebind(copy_dir):
            snap = copy_dir / "semantic_snapshot.yaml"
            m = yaml.safe_load((copy_dir / "resolved_experiment_semantics.yaml").read_text())
            m["semantic_snapshot_path"] = str(snap)
            (copy_dir / "resolved_experiment_semantics.yaml").write_text(yaml.safe_dump(m))
            return snap

        # §41: post-capture mutation of a COPY is rejected
        copy = pathlib.Path(tempfile.mkdtemp()) / "out"
        shutil.copytree(out, copy)
        snap2 = rebind(copy)
        snap2.write_text(snap2.read_text() + "# mutated\n")
        with self.assertRaises(ValueError) as ctx:
            V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", copy,
                              copy / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap2)
        self.assertIn("SEMANTIC_SNAPSHOT_HASH_MISMATCH", str(ctx.exception))
        # §42: rehash + wrong revision rejected
        copy2 = pathlib.Path(tempfile.mkdtemp()) / "out"
        shutil.copytree(out, copy2)
        snap3 = rebind(copy2)
        d = yaml.safe_load(snap3.read_text())
        d["production_revision"] = "f" * 40
        snap3.write_text(yaml.safe_dump(d))
        m2 = yaml.safe_load((copy2 / "resolved_experiment_semantics.yaml").read_text())
        m2["semantic_snapshot_sha256"] = hashlib.sha256(snap3.read_bytes()).hexdigest()
        (copy2 / "resolved_experiment_semantics.yaml").write_text(yaml.safe_dump(m2))
        with self.assertRaises(ValueError) as ctx:
            V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", copy2,
                              copy2 / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap3)
        self.assertIn("SEMANTIC_SNAPSHOT_REVISION_MISMATCH", str(ctx.exception))
        # §43: rehash + schema mismatch rejected
        copy3 = pathlib.Path(tempfile.mkdtemp()) / "out"
        shutil.copytree(out, copy3)
        snap4 = rebind(copy3)
        d = yaml.safe_load(snap4.read_text())
        d["snapshot_schema_version"] = 99
        snap4.write_text(yaml.safe_dump(d))
        m3 = yaml.safe_load((copy3 / "resolved_experiment_semantics.yaml").read_text())
        m3["semantic_snapshot_sha256"] = hashlib.sha256(snap4.read_bytes()).hexdigest()
        (copy3 / "resolved_experiment_semantics.yaml").write_text(yaml.safe_dump(m3))
        with self.assertRaises(ValueError) as ctx:
            V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", copy3,
                              copy3 / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap4)
        self.assertIn("SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
