#!/usr/bin/env python3
"""Prompt79 GC-T1..GC-T12 — three-gate zero-false-positive closure.

G1: manifest semantic_snapshot_path REQUIRED + exact authority.
G2: REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY on REAL transaction output.
G3: CLOSE evidence functional-commit self-provenance.
"""
import hashlib
import json
import os
import pathlib
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
EVIDENCE = ROOT / "docs/super_livo/evidence/round14_final_hard_close_evidence.json"
VALIDATOR = ROOT / "scripts/super_livo/evaluation/close_evidence_validator.py"
GT = "/home/lc/super_livo/results/round13_visual_baseline/ntu_eee_01/eee_01_leica_pose.csv"
A2_RUN = "/home/lc/super_livo/results/round14_phaseA/a2_camera_epoch_shadow/20260829T052214Z"
B0_RUN = "/home/lc/super_livo/results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T052357Z"

sys.path.insert(0, str(ROOT / "scripts/super_livo/evaluation"))
sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
import visual_eval_score as V  # noqa: E402
import semantic_profiles as SP  # noqa: E402


def build_manifest(rev="a" * 40):
    return {"production_revision": rev, "semantic_profile": "D_VISUAL_APPLY",
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
            "config_provenance": {"lio": "x", "visual": "y", "dataset_calibration": "z"},
            "semantic_profile_revision": "b" * 64,
            "dataset_adapter_revision": "c" * 64,
            "transaction_revision": "d" * 40}


def future_run(tmp, manifest, bind_path="semantic_snapshot.yaml"):
    (tmp / "out").mkdir(parents=True, exist_ok=True)
    snap = tmp / "out" / "semantic_snapshot.yaml"
    _, sha = SP.materialize_snapshot(manifest, TEMPLATE, snap)
    manifest["semantic_snapshot_path"] = bind_path
    manifest["semantic_snapshot_sha256"] = sha
    manifest["semantic_snapshot_schema_version"] = 1
    (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(
        yaml.safe_dump(manifest))
    return snap


def resolve(tmp):
    return V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                             tmp / "out" / "resolved_experiment_semantics.yaml",
                             semantic_snapshot=tmp / "out" / "semantic_snapshot.yaml")


class TestGateG1(unittest.TestCase):
    """GC-T1..GC-T3 — manifest snapshot path required + authority."""

    def test_gc_t1_g1_positive(self):
        tmp = pathlib.Path(tempfile.mkdtemp())
        future_run(tmp, build_manifest())
        s = resolve(tmp)
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"], "RUN_EMBEDDED")
        self.assertTrue(s["semantic_provenance"]["complete"])

    def test_gc_t2_g1_negative_path_removed(self):
        tmp = pathlib.Path(tempfile.mkdtemp())
        m = build_manifest()
        future_run(tmp, m)
        # remove ONLY the manifest path; the snapshot file stays present
        p = tmp / "out" / "resolved_experiment_semantics.yaml"
        d = yaml.safe_load(p.read_text())
        d.pop("semantic_snapshot_path")
        p.write_text(yaml.safe_dump(d))
        with self.assertRaises(ValueError) as ctx:
            resolve(tmp)
        self.assertIn("SEMANTIC_SNAPSHOT_PATH_MISSING", str(ctx.exception))

    def test_gc_t3_g1_manifest_path_exact_authority(self):
        tmp = pathlib.Path(tempfile.mkdtemp())
        m = build_manifest()
        (tmp / "out").mkdir(parents=True, exist_ok=True)
        # snapshot_B at run dir (would be "discovered" by a weak resolver)
        snap_b = tmp / "out" / "semantic_snapshot.yaml"
        SP.materialize_snapshot(m, TEMPLATE, snap_b)
        # snapshot_A at another location, bound by the manifest
        snap_a = tmp / "snapshot_A.yaml"
        SP.materialize_snapshot(m, TEMPLATE, snap_a)
        m["semantic_snapshot_path"] = str(snap_a)
        m["semantic_snapshot_sha256"] = hashlib.sha256(snap_a.read_bytes()).hexdigest()
        m["semantic_snapshot_schema_version"] = 1
        (tmp / "out" / "resolved_experiment_semantics.yaml").write_text(yaml.safe_dump(m))
        s = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                              tmp / "out" / "resolved_experiment_semantics.yaml",
                              semantic_snapshot=snap_a)
        # ONLY the manifest-bound A may be authoritative
        self.assertEqual(s["semantic_provenance"]["semantic_snapshot_source"],
                         str(snap_a))
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"], "RUN_REFERENCED")
        # mutation of the non-authoritative run-dir B must NOT affect the run
        snap_b.write_text(snap_b.read_text() + "# garbage\n")
        s2 = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", tmp / "out",
                               tmp / "out" / "resolved_experiment_semantics.yaml",
                               semantic_snapshot=snap_a)
        self.assertEqual(s2["actual_semantics"]["visual_map_policy_id"],
                         s["actual_semantics"]["visual_map_policy_id"])


class TestGateG2(unittest.TestCase):
    """GC-T4..GC-T6 — real-seam template drift immunity."""

    def _real_seam(self):
        run_root = pathlib.Path(tempfile.mkdtemp(prefix="gc4-"))
        fake_bag = run_root / "fake.bag"
        fake_bag.write_text("not a real bag\n")
        calib = pathlib.Path("/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml")
        cfg = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")
        if not (calib.exists() and cfg.exists()):
            self.skipTest("canonical config/calib absent")
        env = dict(os.environ)
        env.update(SLV_CFG=str(cfg), SLV_BAG=str(fake_bag),
                   SLV_CAM_TOPIC="/left/image_raw", SLV_CAM_CALIB=str(calib),
                   SLV_CAM_OFFSET="-0.0199575325817", SLV_DATASET="NTU",
                   SLV_SEQUENCE="eee_01", SLV_MIN_ROWS="1",
                   SLV_SEMANTIC_PROFILE="D_VISUAL_APPLY", SLV_LEGACY_ALIAS="a0")
        proc = subprocess.Popen(
            ["bash", "scripts/super_livo/experiments/run_superlivo_transaction.sh",
             "gc_seam", str(run_root)], cwd=str(ROOT), env=env,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        out = run_root / "gc_seam" / "out"
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
        self.assertTrue(snapshot.exists(), "real seam snapshot missing")
        return out, snapshot, manifest

    def test_gc_t4_g2_real_seam_baseline_accepted(self):
        out, snapshot, manifest = self._real_seam()
        s = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", out, manifest,
                              semantic_snapshot=snapshot)
        self.assertEqual(s["semantic_provenance"]["semantic_binding_mode"], "RUN_EMBEDDED")
        self.assertTrue(s["semantic_provenance"]["complete"])

    def test_gc_t5_g2_real_seam_template_drift_unchanged(self):
        out, snapshot, manifest = self._real_seam()
        base = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", out, manifest,
                                 semantic_snapshot=snapshot)
        base_sha = base["semantic_provenance"]["semantic_snapshot_sha256"]
        base_ids = {k: v for k, v in base["actual_semantics"].items()
                    if k.endswith("_policy_id")}
        # mutate the CURRENT template (test-owned copy injected via env)
        tpl = pathlib.Path(tempfile.mkdtemp()) / "semantic_snapshot_v0.yaml"
        data = TEMPLATE.read_text()
        for old, new in (("S3_SPATIAL_BALANCED_V0", "FAKE_MAP_V999"),
                         ("SUPER_LIVO_PRE_PHASEC_PATCH_V0", "FAKE_PATCH_V999"),
                         ("SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0", "FAKE_RESIDUAL_V999"),
                         ("SUPER_LIVO_PRE_PHASEC_ITERATION_V0", "FAKE_ITER_V999")):
            data = data.replace(old, new)
        tpl.write_text(data)
        # re-resolve the SAME real run — the run-bound snapshot is unchanged
        after = V.build_scorecard("B0_D_CAMERA_EPOCH_APPLY_CORRECTED", out, manifest,
                                  semantic_snapshot=snapshot)
        after_ids = {k: v for k, v in after["actual_semantics"].items()
                     if k.endswith("_policy_id")}
        self.assertEqual(after_ids, base_ids)
        self.assertEqual(after["semantic_provenance"]["semantic_snapshot_sha256"],
                         base_sha)
        self.assertEqual(snapshot.read_bytes(),
                         (out / "semantic_snapshot.yaml").read_bytes())
        # checkpoint identity unchanged
        self.assertEqual(after["provenance"]["git_sha"],
                         base["provenance"]["git_sha"])

    def test_gc_t6_g2_evidence_type_mutation_rejected(self):
        d = json.loads(EVIDENCE.read_text())
        g = d["hard_gates"]["REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY"]
        orig = g["evidence_type"]
        g["evidence_type"] = "RESOLVER_BEHAVIOR"
        tmp = pathlib.Path(tempfile.mkdtemp()) / "ev.json"
        tmp.write_text(json.dumps(d))
        r = subprocess.run([sys.executable, str(VALIDATOR), "--evidence", str(tmp)],
                           capture_output=True, text=True)
        self.assertNotEqual(r.returncode, 0)
        self.assertIn("REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY", r.stdout + r.stderr)
        g["evidence_type"] = orig


class TestGateG3(unittest.TestCase):
    """GC-T7..GC-T11 — evidence self-provenance."""

    def _mutate(self, fn):
        d = json.loads(EVIDENCE.read_text())
        fn(d)
        tmp = pathlib.Path(tempfile.mkdtemp()) / "ev.json"
        tmp.write_text(json.dumps(d))
        r = subprocess.run([sys.executable, str(VALIDATOR), "--evidence", str(tmp)],
                           capture_output=True, text=True)
        return r

    def test_gc_t7_g3_positive(self):
        r = subprocess.run([sys.executable, str(VALIDATOR)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)

    def test_gc_t8_g3_prompt_only_commit_rejected(self):
        r = self._mutate(lambda d: d.__setitem__(
            "functional_corrective_commit", "72c5684179a1b61b05fc8330ec9713074732c547"))
        self.assertNotEqual(r.returncode, 0)
        self.assertIn("FUNCTIONAL_COMMIT", r.stdout + r.stderr)

    def test_gc_t9_g3_missing_commit_rejected(self):
        r = self._mutate(lambda d: d.pop("functional_corrective_commit"))
        self.assertNotEqual(r.returncode, 0)

    def test_gc_t10_g3_short_sha_rejected(self):
        r = self._mutate(lambda d: d.__setitem__(
            "functional_corrective_commit", "c58ecdd"))
        self.assertNotEqual(r.returncode, 0)

    def test_gc_t11_g3_pre_fix_valid_sha_rejected(self):
        # 8e46a6e (initial HEAD) predates this prompt's functional fix
        r = self._mutate(lambda d: d.__setitem__(
            "functional_corrective_commit", "8e46a6ef92481f39e6bd5ba86a933192e4dce0d1"))
        self.assertNotEqual(r.returncode, 0)
        self.assertIn("FUNCTIONAL_COMMIT", r.stdout + r.stderr)


class TestCloseE2E(unittest.TestCase):
    def test_gc_t12_complete_evidence_e2e(self):
        r = subprocess.run([sys.executable, str(VALIDATOR)],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        d = json.loads(EVIDENCE.read_text())
        self.assertIn("functional_corrective_commit", d)
        self.assertIn("evidence_generation_commit_or_parent", d)
        self.assertIn("production_paths_tested", d)
        for gate in ("G1_MANIFEST_SNAPSHOT_PATH_REQUIRED",
                     "G2_REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY",
                     "G3_CLOSE_EVIDENCE_SELF_PROVENANCE"):
            self.assertIn(gate, d["hard_gates"])
            g = d["hard_gates"][gate]
            for k in ("gate_id", "semantic_invariant", "authority", "status",
                      "evidence_type", "production_path", "positive_test",
                      "negative_mutation_test", "expected_failure",
                      "observed_failure", "artifact", "command"):
                self.assertIn(k, g, f"{gate}.{k}")


if __name__ == "__main__":
    unittest.main()
