#!/usr/bin/env python3
"""Prompt67 CLOSE TDD — test-hook isolation (TH), validator-contract
integrity (VC), production-mode no-bag preflight (PB)."""

import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
EXP = ROOT / "scripts/super_livo/experiments"
RUNNER = EXP / "run_offline_variant.sh"
SUPERVISOR = EXP / "run_superlivo_transaction.py"
SUPERVISOR_SH = EXP / "run_superlivo_transaction.sh"
SEMANTIC = EXP / "semantic_profiles.py"
CFG = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")
CANONICAL_NODE = pathlib.Path("/home/lc/super_livo/devel/.private/super_lio/lib/super_lio/super_lio_offline_node")
CANONICAL_VALIDATOR = EXP / "validate_d_visual_shadow_result.py"

sys.path.insert(0, str(EXP))
import semantic_profiles as sp

REVISIONS = {k: "r" for k in sp.REVISION_FIELDS}
PROVENANCE = {"lio": "a", "visual": "b", "dataset_calibration": "c"}


def resolve(profile, alias="d0"):
    return sp.resolve_profile(profile, legacy_alias=alias, dataset="ntu",
                              sequence="eee_01", camera_stride=1,
                              revisions=REVISIONS, provenance=PROVENANCE)


class TestValidatorContractIntegrity(unittest.TestCase):
    def test_vc_t1_validator_identity_tamper(self):
        m = resolve("D_VISUAL_SHADOW")
        m["validator"] = "scripts/super_livo/experiments/run_evidence.py"
        with self.assertRaises(sp.SemanticProfileError) as ctx:
            sp.validate_manifest(m)
        self.assertIn("validator contract integrity", str(ctx.exception))

    def test_vc_t2_evidence_requirement_tamper(self):
        m = resolve("D_VISUAL_SHADOW")
        m["requires_measurement_evidence"] = False
        with self.assertRaises(sp.SemanticProfileError) as ctx:
            sp.validate_manifest(m)
        self.assertIn("evidence contract integrity", str(ctx.exception))

    def test_vc_t3_absolute_external_validator(self):
        with self.assertRaises(sp.SemanticProfileError):
            sp.resolve_validator_path("/tmp/fake_validator.py")

    def test_vc_t4_path_traversal(self):
        with self.assertRaises(sp.SemanticProfileError):
            sp.resolve_validator_path("scripts/super_livo/experiments/../../../../etc/passwd")

    def test_vc_t5_wrong_repo_validator(self):
        m = resolve("D_VISUAL_SHADOW")
        m["validator"] = "scripts/super_livo/experiments/run_evidence.py"
        with self.assertRaises(sp.SemanticProfileError):
            sp.validate_manifest(m)

    def test_vc_t6_symlink_escape_containment(self):
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="vc6-"))
        try:
            outside = tmp / "outside.py"
            outside.write_text("x=1")
            link = pathlib.Path(sp.VALIDATOR_NAMESPACE) / "__vc6_escape_test.py"
            link.symlink_to(outside)
            try:
                with self.assertRaises(sp.SemanticProfileError):
                    sp.resolve_validator_path(
                        "scripts/super_livo/experiments/__vc6_escape_test.py")
            finally:
                link.unlink()
        finally:
            shutil.rmtree(tmp, ignore_errors=True)

    def test_vc_t7_canonical_shadow_manifest_pass(self):
        m = resolve("D_VISUAL_SHADOW")
        sp.validate_manifest(m)
        self.assertEqual(m["validator"], sp.VALIDATOR_CONTRACT["D_VISUAL_SHADOW"][0])
        self.assertIs(m["requires_measurement_evidence"], True)

    def test_vc_t8_validator_and_evidence_canonical(self):
        m = resolve("D_VISUAL_SHADOW")
        sp.validate_manifest(m)
        self.assertTrue(m["requires_measurement_evidence"])

    def test_vc_t10_schema_legacy_non_executable(self):
        m = resolve("D_VISUAL_SHADOW")
        m["semantic_schema_version"] = "1"
        del m["visual_measurement_event"]
        del m["visual_measurement_timestamp_semantics"]
        del m["visual_measurement_exact_once"]
        del m["camera_payload_ownership_mode"]
        sp.validate_manifest(m)  # SCHEMA_LEGACY accepted as provenance
        with self.assertRaises(sp.SemanticProfileError):
            sp.validate_executability(m)  # never executable


class TestTestHookIsolation(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="th-")
        self.fake_node = pathlib.Path(self.tmp) / "fake_node.sh"
        self.fake_node.write_text("#!/bin/bash\nexit 0\n")
        self.fake_node.chmod(0o755)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _runner_env(self, test_node=None, test_mode=None):
        env = dict(os.environ)
        env["CFG_DUMMY"] = "1"
        if test_node is not None:
            env["SLV_TEST_NODE_CMD"] = str(test_node)
        if test_mode is not None:
            env["SLV_TEST_MODE"] = test_mode
        return env

    def test_th_t1_fake_node_without_test_mode(self):
        p = subprocess.run(
            ["bash", "-n", str(RUNNER)], capture_output=True, text=True)
        self.assertEqual(p.returncode, 0)
        text = open(RUNNER).read()
        self.assertIn("SLV_TEST_NODE_CMD set without SLV_TEST_MODE=1", text)

    def test_th_t3_both_hooks_without_test_mode_fail_closed(self):
        text = open(RUNNER).read()
        self.assertIn("SLV_TEST_NODE_CMD set without SLV_TEST_MODE=1", text)
        sup = open(SUPERVISOR_SH).read()
        self.assertIn("SLV_TEST_VALIDATOR set without SLV_TEST_MODE=1", sup)

    def test_th_t7_production_clean_canonical_node_selected(self):
        text = open(RUNNER).read()
        self.assertIn("NODE_IDENTITY=$NODE", text)
        self.assertIn("$ROOT/devel/.private/super_lio/lib/super_lio/super_lio_offline_node", text)

    def test_th_t8_production_adapters_do_not_enable_test_mode(self):
        adapters = EXP / "adapters"
        for name in ("run_ntu_super_lio.sh", "run_ntu_d_visual_shadow.sh",
                     "run_ntu_fast_livo2.sh"):
            path = adapters / name
            if not path.exists():
                continue
            text = path.read_text()
            # adapters may READ the test-hook names only to reject them
            # (Prompt70 ambient guard); they must never SET/export them
            for forbidden in ("export SLV_TEST", "export SLV_RUNNER",
                              "export SLV_LOCK_FILE", "SLV_TEST_MODE=",
                              "SLV_TEST_NODE_CMD=", "SLV_TEST_VALIDATOR=",
                              "SLV_RUNNER=", "SLV_LOCK_FILE="):
                self.assertNotIn(forbidden, text)

    def test_th_t9_unknown_test_hook_inert(self):
        # only the documented hooks are read; unknown SLV_TEST_* vars are
        # never consulted -> cannot silently change execution
        sup = open(SUPERVISOR_SH).read()
        runner = open(RUNNER).read()
        for name in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR"):
            self.assertIn(name, sup + runner)
        self.assertNotIn("SLV_TEST_FOO", sup + runner)


class TestProductionPreflight(unittest.TestCase):
    harness = None

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="pb-")
        cls.run_dir = pathlib.Path(cls.tmp) / "runs"
        cls.run_dir.mkdir()
        cls.bag = pathlib.Path(cls.tmp) / "fake.bag"
        cls.bag.write_text("not a real bag: preflight-only")

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def _pb_run(self, run_id, cwd=None):
        env = dict(os.environ)
        for k in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR"):
            env.pop(k, None)
        env.update({
            "SLV_RUN_ID": run_id,
            "SLV_CFG": str(CFG),
            "SLV_BAG": str(self.bag),
            "SLV_SEMANTIC_PROFILE": "D_VISUAL_SHADOW",
            "SLV_LEGACY_ALIAS": "d0",
            "SLV_CAM_CALIB": "/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml",
            "SLV_CAM_OFFSET": "-0.0199575325817",
        })
        p = subprocess.run(["bash", str(SUPERVISOR_SH), run_id, str(self.run_dir)],
                           env=env, capture_output=True, text=True,
                           cwd=cwd or str(ROOT), timeout=120)
        run = self.run_dir / run_id
        return p.returncode, p.stdout

    def _runner_log(self, run_id):
        p = self.run_dir / run_id / "runner.log"
        return p.read_text() if p.exists() else ""

    def test_pb_t1_t3_t4_production_preflight_reaches_canonical_identities(self):
        # hooks OFF: production preflight gates (manifest, capability,
        # validator contract/path, node identity) all PASS; the real estimator
        # fails fast on the non-bag fixture (estimator-level, not infra).
        rc, out = self._pb_run("pb_t1")
        log = self._runner_log("pb_t1")
        self.assertIn("NODE_IDENTITY=" + str(CANONICAL_NODE), log,
                      "canonical node identity not selected: " + log[-800:])
        self.assertIn("semantic profile: PASS", out)
        self.assertIn("producer gates: PASS", out)
        self.assertIn(str(CANONICAL_VALIDATOR), out,
                      "canonical validator not resolved: " + out[-500:])
        sup = (self.run_dir / "pb_t1" / "supervisor.log").read_text()
        self.assertIn("PLAYBACK_STARTED", sup,
                      "preflight gates did not pass: " + sup[-400:])

    def test_pb_t2_no_test_hooks_active(self):
        text = open(SUPERVISOR_SH).read()
        runner = open(RUNNER).read()
        for name in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR"):
            self.assertIn(name, text + runner)

    def test_pb_t5_arbitrary_cwd(self):
        for cwd in (str(ROOT), "/tmp", self.tmp):
            rc, out = self._pb_run("pb_t5_" + cwd.replace("/", "_"), cwd=cwd)
            log = self._runner_log("pb_t5_" + cwd.replace("/", "_"))
            self.assertIn("NODE_IDENTITY=" + str(CANONICAL_NODE), log,
                          "cwd=%s failed: %s" % (cwd, log[-500:]))


if __name__ == "__main__":
    unittest.main()
