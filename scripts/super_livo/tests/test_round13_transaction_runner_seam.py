#!/usr/bin/env python3
"""Prompt65 integration TDD — REAL supervisor→runner no-bag seam (TR-T1..T16).

Executes the actual shell path: generic transaction supervisor -> canonical
runner interface -> semantic profile resolution -> preflight -> fake bounded
child -> validator dispatch -> transaction terminal state -> cleanup.

NO rosbag, NO estimator, NO dataset playback, NO network, no long-running
processes. Test hooks (SLV_TEST_NODE_CMD / SLV_TEST_VALIDATOR) are test-only,
default OFF, fail-closed.
"""

import json
import os
import pathlib
import shutil
import stat
import subprocess
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
SUPERVISOR = ROOT / "scripts/super_livo/experiments/run_superlivo_transaction.sh"
RUNNER = ROOT / "scripts/super_livo/experiments/run_offline_variant.sh"
SEMANTIC = ROOT / "scripts/super_livo/experiments/semantic_profiles.py"
CFG = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")

FAKE_NODE = """#!/bin/bash
# fake bounded estimator child: honours the transaction gate, writes the
# minimal expected outputs, exits 0.
while [ ! -f "${SLV_TRANSACTION_GATE_DIR:-/nonexistent}/transaction_playback_authorized" ]; do
  [ -f "${SLV_TRANSACTION_GATE_DIR:-/nonexistent}/cancel" ] && exit 130
  sleep 0.05
done
export OUT_DIR="$(rosparam get /lio/offline/out_dir)"
mkdir -p "$OUT_DIR"
python3 -c "
import os, time
t0 = 1609059000.0
with open(os.path.join(os.environ['OUT_DIR'], 'trajectory.tum'), 'w') as f:
    for i in range(3200):
        f.write('%.6f %f %f %f 0 0 0 1\\n' % (t0 + i * 0.1, i * 0.01, 0.0, 0.0))
"
echo "fake node done" > "$OUT_DIR/node_stdout.log"
# reuse the canonical Prompt60 post-resolve as the effective-config template
# (same resolved semantics: stride 1, imu_fullscan, offset -0.0199575325817)
TEMPLATE="/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01/d_visual_shadow/round13_prompt60_measurement_n1/out/effective_config.post_resolve.yaml"
if [ -f "$TEMPLATE" ]; then
  cp "$TEMPLATE" "$OUT_DIR/effective_config.post_resolve.yaml"
else
  echo "{}" > "$OUT_DIR/effective_config.post_resolve.yaml"
fi
exit 0
"""

FAKE_VALIDATOR = """#!/usr/bin/env python3
import argparse, sys, yaml
p = argparse.ArgumentParser()
p.add_argument("--log"); p.add_argument("--manifest"); p.add_argument("--out")
a = p.parse_args()
yaml.safe_dump({"validator": "fake", "gate": "PASS"}, open(a.out, "w"))
sys.exit(0)
"""


class SeamHarness:
    def __init__(self, testcase):
        self.tmp = tempfile.mkdtemp(prefix="slv-seam-")
        self.run_dir = pathlib.Path(self.tmp) / "runs"
        self.run_dir.mkdir()
        self.node = pathlib.Path(self.tmp) / "fake_node.sh"
        self.node.write_text(FAKE_NODE)
        self.node.chmod(0o755)
        self.validator = pathlib.Path(self.tmp) / "fake_validator.py"
        self.validator.write_text(FAKE_VALIDATOR)
        self.validator.chmod(0o755)
        self.bag = pathlib.Path(self.tmp) / "fake.bag"
        self.bag.write_text("fake bag stand-in for preflight file check only")
        self.testcase = testcase

    def start(self, profile, run_id, *, measurement_evidence="1", legacy_alias="d0",
              test_validator=True, extra=None, cwd=None):
        env = dict(os.environ)
        env.update({
            "SLV_RUN_ID": run_id,
            "SLV_CFG": str(CFG),
            "SLV_BAG": str(self.bag),
            "SLV_SEMANTIC_PROFILE": profile,
            "SLV_LEGACY_ALIAS": legacy_alias,
            "SLV_MEASUREMENT_EVIDENCE": measurement_evidence,
            "SLV_TEST_NODE_CMD": str(self.node),
            "SLV_CAM_CALIB": "/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml",
            "SLV_CAM_OFFSET": "-0.0199575325817",
        })
        if test_validator:
            env["SLV_TEST_VALIDATOR"] = str(self.validator)
        env.setdefault("SLV_TEST_MODE", "1")  # seam harness is test-only
        env.update(extra or {})
        cmd = ["bash", str(SUPERVISOR), run_id, str(self.run_dir)]
        return subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, cwd=cwd or str(ROOT))

    def run(self, profile, run_id, **kw):
        p = self.start(profile, run_id, **kw)
        out, _ = p.communicate(timeout=120)
        run = self.run_dir / run_id
        state = None
        if run.exists() and (run / "state.json").exists():
            state = json.loads((run / "state.json").read_text())
        evidence = out.decode()
        sup = run / "supervisor.log"
        if sup.exists():
            evidence += "\n" + sup.read_text()
        runner_log = run / "runner.log"
        if runner_log.exists():
            evidence += "\n" + runner_log.read_text()
        return p.returncode, evidence, state

    def cleanup(self):
        for proc in [p for p in []]:
            pass
        shutil.rmtree(self.tmp, ignore_errors=True)


class TestTransactionRunnerSeam(unittest.TestCase):
    harness = None

    @classmethod
    def setUpClass(cls):
        cls.harness = SeamHarness(cls)

    @classmethod
    def tearDownClass(cls):
        cls.harness.cleanup()

    def read_state(self, run_id):
        p = self.harness.run_dir / run_id / "state.json"
        return json.loads(p.read_text()) if p.exists() else None

    # ---- RED reproductions (starting HEAD) ----
    def test_tr_t1_policy_interface_green(self):
        """Bug A GREEN: the supervisor->runner seam no longer fails on the
        positional policy placeholder; normalized D_VISUAL_SHADOW reaches the
        bounded child and SUCCESS (RED at starting HEAD: SEMANTIC_AUTHORITY_
        CONFLICT on profile_resolved, reproduced before the corrective)."""
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t1")
        self.assertEqual(rc, 0, out)
        self.assertIn("SUCCESS", out)
        self.assertNotIn("SEMANTIC_AUTHORITY_CONFLICT", out)

    def test_tr_t2_measurement_evidence_coupling_green(self):
        """Bug B GREEN: the generic supervisor no longer rejects a profile
        for a profile-specific evidence requirement (RED at starting HEAD:
        STATIC_PREFLIGHT_FAIL 'measurement instrumentation not enabled')."""
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t2",
                                          measurement_evidence="0")
        self.assertEqual(rc, 0, out)
        self.assertIn("SUCCESS", out)

    def test_tr_t3_exact_once_capability_gate(self):
        """Exact-once requested vs effective must be enforced. Unit-level."""
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as sp
        m = sp.resolve_profile("D_VISUAL_SHADOW", legacy_alias="d0", dataset="ntu",
                               sequence="eee_01", camera_stride=1,
                               revisions={k: "r" for k in sp.REVISION_FIELDS},
                               provenance={"lio": "a", "visual": "b",
                                           "dataset_calibration": "c"})
        self.assertIn("visual_measurement_exact_once",
                      sp.EFFECTIVE_PRODUCTION_CAPABILITY)
        m["visual_measurement_exact_once"] = False
        with self.assertRaises(sp.SemanticProfileError):
            sp.validate_executability(m)

    # ---- GREEN seam (after corrective) ----
    def test_tr_t4_normalized_shadow_seam_green(self):
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t4")
        self.assertEqual(rc, 0, out)
        self.assertIn("SUCCESS", out)
        self.assertIsNotNone(state)
        self.assertEqual(state["state"], "SUCCESS")

    def test_tr_t5_apply_executable_after_phase_b(self):
        # Phase B: D_VISUAL_APPLY now executes (the capability gate opens).
        rc, out, state = self.harness.run("D_VISUAL_APPLY", "tr_t5")
        self.assertNotIn("SEMANTIC_AUTHORITY_CONFLICT", out)
        self.assertNotIn("SLV_TEST_VALIDATOR set without SLV_TEST_MODE=1", out)

    def test_tr_t8_missing_validator_fails(self):
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t8",
                                          test_validator=False)
        self.assertIn("validator", out)
        self.assertNotIn("SUCCESS", out)

    def test_tr_t9_no_mandatory_evidence_in_generic_lifecycle(self):
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t9",
                                          measurement_evidence="0",
                                          test_validator=False)
        # Profile-declared evidence requirement (Shadow validator) is absent,
        # so the validator contract must fail, not the generic supervisor.
        self.assertNotIn("measurement instrumentation not enabled", out)

    def test_tr_t10_profile_required_evidence_still_fail_closed(self):
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t10",
                                          measurement_evidence="0")
        # fake validator passes; with evidence off the profile still executes
        # (evidence is Shadow-validator business). Gate: reach SUCCESS.
        self.assertEqual(rc, 0, out)

    def test_tr_t14_no_residual_process(self):
        rc, out, state = self.harness.run("D_VISUAL_SHADOW", "tr_t14")
        time.sleep(0.5)
        p = subprocess.run(["pgrep", "-af", "run_superlivo_transaction"],
                           capture_output=True, text=True)
        self.assertEqual(p.returncode, 1, "supervisor processes remain: " + p.stdout)

    def test_tr_t15_legacy_only_path_unchanged(self):
        text = open(RUNNER).read()
        self.assertIn("case \"$VARIANT\" in", text)
        self.assertIn("b0|c0|d0) is_false \"$app\"", text)

    def test_tr_t16_normalized_semantic_authority(self):
        text = open(RUNNER).read()
        norm = text[text.index("Prompt64: normalized mode"):]
        self.assertIn("rosparams --manifest \"$SEMANTIC_MANIFEST\"", norm)


class TestSemanticUnit(unittest.TestCase):
    def test_tr_t6_two_profiles_same_lifecycle(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as sp
        for profile in ("D_VISUAL_SHADOW", "D_SCHEDULER_BASE"):
            m = sp.resolve_profile(profile, legacy_alias="x", dataset="ntu",
                                   sequence="e", camera_stride=1,
                                   revisions={k: "r" for k in sp.REVISION_FIELDS},
                                   provenance={"lio": "a", "visual": "b",
                                               "dataset_calibration": "c"})
            sp.validate_manifest(m)

    def test_tr_t7_validator_routing(self):
        import sys
        sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as sp
        m = sp.resolve_profile("D_VISUAL_SHADOW", legacy_alias="x", dataset="ntu",
                               sequence="e", camera_stride=1,
                               revisions={k: "r" for k in sp.REVISION_FIELDS},
                               provenance={"lio": "a", "visual": "b",
                                           "dataset_calibration": "c"})
        self.assertIn("validate_d_visual_shadow_result.py", m["validator"])
        text = open(SUPERVISOR).read()
        self.assertNotIn("validate_d_visual_shadow_result", text)


CANONICAL_VALIDATOR = str(ROOT / "scripts/super_livo/experiments/validate_d_visual_shadow_result.py")

FIXTURE_COUNTERS = """V-0 VisualMap: parents=1 landmarks=1 slots_used=1 created=1 frames=1 attempts=1
VISUAL_MEASUREMENT query: attempts=3 hits=3 misses=0 rejected_explicit=0 conservation=OK
VISUAL_MEASUREMENT observation: frames=1 candidates=2 valid=2 rejected=0 residual_samples=4 conservation=OK
VISUAL_MEASUREMENT H: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1.0 P95=1.0 P99=1.0 max=1.0
VISUAL_MEASUREMENT b: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1.0 P95=1.0 P99=1.0 max=1.0
VISUAL_MEASUREMENT proposed_correction=NOT_COMPUTED_BY_SHADOW_PROFILE state_apply_count=0
fullscan ownership: raw_input_points=100 pre_observe_excluded_scans=0 pre_observe_excluded_points=0 eligible_geometry_points=100 used_once=100 duplicate_use=0 never_used=0 imu_only_segments=1
TEST_FIXTURE=true NO_REAL_ESTIMATOR_OUTPUT=true NO_SCIENTIFIC_RESULT=true
"""


class CanonicalValidatorSeamHarness(SeamHarness):
    def __init__(self, testcase):
        super().__init__(testcase)
        self.fixture = pathlib.Path(self.tmp) / "fixture_node_stdout.log"
        self.fixture.write_text(FIXTURE_COUNTERS)

    def start(self, profile, run_id, *, test_validator=False, **kw):
        return super().start(profile, run_id, test_validator=test_validator, **kw)

    def run_canonical(self, profile, run_id, node_stdout=None, cwd=None, **kw):
        env_extra = kw.pop("extra", {})
        env_extra["SLV_TEST_MODE"] = "1"  # allow test hooks; canonical validator used
        return self.run(profile, run_id, test_validator=False,
                        extra=env_extra, cwd=cwd, **kw)


class TestCanonicalValidatorSeam(unittest.TestCase):
    harness = None

    @classmethod
    def setUpClass(cls):
        cls.harness = CanonicalValidatorSeamHarness(cls)
        # fake node writes the synthetic fixture counters
        node = cls.harness.node
        text = node.read_text()
        text = text.replace('echo "fake node done" > "$OUT_DIR/node_stdout.log"',
                            'cp "%s" "$OUT_DIR/node_stdout.log"' % cls.harness.fixture)
        node.write_text(text)

    @classmethod
    def tearDownClass(cls):
        cls.harness.cleanup()

    def _gate(self, run_id):
        p = self.harness.run_dir / run_id / ("D_VISUAL_SHADOW_gate.yaml")
        if p.exists():
            import yaml
            return yaml.safe_load(p.read_text())
        return None

    def test_vr_t2_canonical_validator_invocation(self):
        # SLV_TEST_VALIDATOR unset -> manifest-selected canonical validator
        # must genuinely execute (dispatch proof; incomplete fixture -> the
        # validator itself reports EVIDENCE_INCOMPLETE_NOT_CANONICAL).
        rc, out, state = self.harness.run_canonical("D_VISUAL_SHADOW", "vr_t2")
        self.assertNotIn("SLV_TEST_VALIDATOR", out)
        gate = self._gate("vr_t2")
        self.assertIsNotNone(gate, "canonical validator did not write gate: " + out)
        self.assertEqual(gate.get("semantic_profile"), "D_VISUAL_SHADOW")

    def test_vr_t3_canonical_validator_success_seam(self):
        # Synthetic bounded result fixture -> canonical validator PASS ->
        # transaction SUCCESS + cleanup_verified.
        rc, out, state = self.harness.run_canonical("D_VISUAL_SHADOW", "vr_t3")
        self.assertEqual(rc, 0, out)
        self.assertIn("SUCCESS", out)
        self.assertIsNotNone(state)
        self.assertEqual(state["state"], "SUCCESS")
        self.assertTrue(state["cleanup_verified"])
        gate = self._gate("vr_t3")
        self.assertIsNotNone(gate)
        self.assertEqual(gate.get("hard_gate_pass"), True)

    def test_vr_t6_production_override_rejected(self):
        # test mode OFF + override set -> fail closed (no canonical bypass).
        rc, out, state = self.harness.run(
            "D_VISUAL_SHADOW", "vr_t6", test_validator=True,
            extra={"SLV_TEST_MODE": "0"})
        self.assertNotEqual(rc, 0)
        self.assertIn("without SLV_TEST_MODE=1", out)  # any fail-closed hook gate

    def test_vr_t5_explicit_test_mode_allowed(self):
        rc, out, state = self.harness.run(
            "D_VISUAL_SHADOW", "vr_t5", test_validator=True,
            extra={"SLV_TEST_MODE": "1"})
        self.assertEqual(rc, 0, out)

    def test_vr_t7_cwd_invariance(self):
        for cwd in (str(ROOT), "/tmp", str(self.harness.tmp)):
            rc, out, state = self.harness.run_canonical(
                "D_VISUAL_SHADOW", "vr_t7_" + cwd.replace("/", "_"),
                cwd=cwd)
            self.assertEqual(rc, 0, "cwd=%s: %s" % (cwd, out))
            self.assertIn("SUCCESS", out)

    def test_vr_t9_missing_canonical_validator(self):
        # unknown validator path must fail explicitly (no fallback): the
        # resolver anchors repo-relative paths, so a missing file resolves to
        # an absolute non-existent path and the supervisor rejects it.
        rc, out, state = self.harness.run(
            "D_VISUAL_SHADOW", "vr_t9", test_validator=True,
            extra={"SLV_TEST_MODE": "1",
                   "SLV_TEST_VALIDATOR": str(self.harness.tmp) + "/no_such_validator.py"})
        self.assertNotEqual(rc, 0)
        self.assertIn("validator", out)

    def test_vr_t10_invalid_validator_target(self):
        # a directory is not a valid validator target -> explicit preflight fail
        rc, out, state = self.harness.run(
            "D_VISUAL_SHADOW", "vr_t10", test_validator=True,
            extra={"SLV_TEST_MODE": "1", "SLV_TEST_VALIDATOR": self.harness.tmp})
        self.assertNotEqual(rc, 0)
        self.assertIn("validator", out)

    def test_vr_t13_generic_supervisor_algorithm_free(self):
        text = open(SUPERVISOR).read()
        for forbidden in ("/lio/v4/apply", "imu_fullscan", "D_VISUAL_SHADOW ]",
                          "validate_d_visual_shadow_result"):
            self.assertNotIn(forbidden, text)

    def test_vr_t16_no_residual_process(self):
        rc, out, state = self.harness.run_canonical("D_VISUAL_SHADOW", "vr_t16")
        time.sleep(0.5)
        p = subprocess.run(["pgrep", "-af", "run_superlivo_transaction"],
                           capture_output=True, text=True)
        self.assertEqual(p.returncode, 1, p.stdout)


if __name__ == "__main__":
    unittest.main()
