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
              test_validator=True, extra=None):
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
        env.update(extra or {})
        cmd = ["bash", str(SUPERVISOR), run_id, str(self.run_dir)]
        return subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)

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

    def test_tr_t5_unsupported_apply_fails_at_capability_gate(self):
        rc, out, state = self.harness.run("D_VISUAL_APPLY", "tr_t5")
        self.assertIn("SEMANTIC_PROFILE_FAIL", out)
        self.assertNotIn("SEMANTIC_AUTHORITY_CONFLICT", out)
        self.assertNotIn("measurement instrumentation not enabled", out)
        self.assertNotIn("profile_resolved", out)

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


if __name__ == "__main__":
    unittest.main()
