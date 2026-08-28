#!/usr/bin/env python3
"""Prompt70 CLOSE TDD — real runtime proof (CI-T7R/T8A/T8B) + production
adapter ambient-environment isolation (AE-T1..T8). All gates behavioral."""

import json
import os
import pathlib
import shutil
import subprocess
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
EXP = ROOT / "scripts/super_livo/experiments"
SUPERVISOR = EXP / "run_superlivo_transaction.sh"
ADAPTER = EXP / "adapters" / "run_ntu_d_visual_shadow.sh"
CFG = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")
ADAPTER_OUT_ROOT = pathlib.Path("/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01")


class Fixture:
    """Test-owned supervisor fixture: an exact copy of the production
    supervisor with a configurable sibling runner (escape / missing / broken)."""

    def __init__(self, tmp, runner_mode="canonical"):
        self.tmp = pathlib.Path(tmp)
        self.dir = self.tmp / "fixture"
        self.dir.mkdir()
        self.runs = self.tmp / "runs"
        self.runs.mkdir()
        self.supervisor = self.dir / "run_superlivo_transaction.sh"
        shutil.copy2(SUPERVISOR, self.supervisor)
        self.supervisor.chmod(0o755)
        self.outside = self.tmp / "outside"
        self.outside.mkdir()
        self.rogue = self.outside / "rogue_runner.sh"
        self.rogue.write_text("#!/bin/bash\necho EXTERNAL_RUNNER_EXECUTED >&2\nexit 1\n")
        self.rogue.chmod(0o755)
        runner = self.dir / "run_offline_variant.sh"
        if runner_mode == "escape":
            runner.symlink_to(self.rogue)
        elif runner_mode == "missing":
            pass  # absent
        elif runner_mode == "broken":
            runner.symlink_to(self.outside / "no_such_target.sh")
        else:
            shutil.copy2(EXP / "run_offline_variant.sh", runner)
            runner.chmod(0o755)
        self.bag = self.tmp / "fake.bag"
        self.bag.write_text("not a real bag")

    def env(self):
        env = dict(os.environ)
        for k in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR",
                  "SLV_RUNNER", "SLV_LOCK_FILE", "SLV_CFG", "SLV_BAG"):
            env.pop(k, None)
        env.update({
            "SLV_CFG": str(CFG),
            "SLV_BAG": str(self.bag),
            "SLV_SEMANTIC_PROFILE": "D_VISUAL_SHADOW",
            "SLV_LEGACY_ALIAS": "d0",
            "SLV_CAM_CALIB": "/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml",
            "SLV_CAM_OFFSET": "-0.0199575325817",
        })
        return env

    def run(self, run_id, env=None, cwd=None, timeout=60):
        p = subprocess.run(["bash", str(self.supervisor), run_id, str(self.runs)],
                           env=env or self.env(), capture_output=True, text=True,
                           cwd=cwd or "/tmp", timeout=timeout)
        run = self.runs / run_id
        return p.returncode, p.stdout + "\n" + p.stderr, run


class TestRunnerContainmentRuntime(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="cit7-")

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_ci_t7r_runner_escape_refused(self):
        f = Fixture(self.tmp, runner_mode="escape")
        rc, out, run = f.run("ci_t7r")
        self.assertNotEqual(rc, 0)
        self.assertIn("CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR", out)
        self.assertNotIn("EXTERNAL_RUNNER_EXECUTED", out)
        self.assertFalse((run / "runner.log").exists(),
                         "external runner must never start")

    def test_ci_t8a_missing_runner_fail_closed(self):
        f = Fixture(self.tmp, runner_mode="missing")
        rc, out, run = f.run("ci_t8a")
        self.assertNotEqual(rc, 0)
        self.assertIn("CANONICAL_RUNNER_MISSING", out)
        self.assertFalse((run / "runner.log").exists())

    def test_ci_t8b_broken_runner_symlink_fail_closed(self):
        f = Fixture(self.tmp, runner_mode="broken")
        rc, out, run = f.run("ci_t8b")
        self.assertNotEqual(rc, 0)
        self.assertFalse((run / "runner.log").exists())
        # bounded: command completed within the test timeout (no hang/fallback)
        self.assertIn("CANONICAL_RUNNER_MISSING", out)

    def test_ci_t7_canonical_fixture_still_runs_resolver(self):
        f = Fixture(self.tmp, runner_mode="canonical")
        rc, out, run = f.run("ci_t7ctl")
        # canonical sibling: the supervisor proceeds (handshake may fail with
        # the fake bag / no fake node, but never an identity failure)
        self.assertNotIn("CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR", out)
        self.assertNotIn("CANONICAL_RUNNER_MISSING", out)
        self.assertNotIn("EXTERNAL_RUNNER_EXECUTED", out)


class TestProductionAdapterAmbientEnv(unittest.TestCase):
    def setUp(self):
        self.tmp = pathlib.Path(tempfile.mkdtemp(prefix="ae-"))
        self.runs = []
        self.fake_runner = self.tmp / "fake_runner.sh"
        self.fake_runner.write_text("#!/bin/bash\necho AMBIENT_FAKE_RUNNER_EXECUTED >&2\nexit 1\n")
        self.fake_runner.chmod(0o755)
        self.fake_node = self.tmp / "fake_node.sh"
        self.fake_node.write_text("#!/bin/bash\necho AMBIENT_FAKE_NODE_EXECUTED >&2\nexit 1\n")
        self.fake_node.chmod(0o755)
        self.fake_validator = self.tmp / "fake_validator.py"
        self.fake_validator.write_text("#!/usr/bin/env python3\nimport sys\nsys.stderr.write('AMBIENT_FAKE_VALIDATOR_EXECUTED\\n')\nsys.exit(2)\n")
        self.fake_validator.chmod(0o755)

    def tearDown(self):
        for r in self.runs:
            shutil.rmtree(ADAPTER_OUT_ROOT / r, ignore_errors=True)
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _run_adapter(self, run_id, ambient):
        self.runs.append(run_id)
        env = dict(os.environ)
        for k in ambient:
            env.pop(k, None)
        env.update(ambient)
        env.setdefault("SLV_SEMANTIC_PROFILE", "D_VISUAL_SHADOW")
        env.setdefault("SLV_LEGACY_ALIAS", "d0")
        p = subprocess.run(["bash", str(ADAPTER), run_id], env=env,
                           capture_output=True, text=True, cwd="/tmp", timeout=60)
        return p.returncode, p.stdout + "\n" + p.stderr

    def test_ae_t1_ambient_fake_runner_blocked(self):
        rc, out = self._run_adapter("ae_t1", {
            "SLV_TEST_MODE": "1", "SLV_RUNNER": str(self.fake_runner)})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)
        self.assertNotIn("AMBIENT_FAKE_RUNNER_EXECUTED", out)

    def test_ae_t2_ambient_fake_node_blocked(self):
        rc, out = self._run_adapter("ae_t2", {
            "SLV_TEST_MODE": "1", "SLV_TEST_NODE_CMD": str(self.fake_node)})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)
        self.assertNotIn("AMBIENT_FAKE_NODE_EXECUTED", out)

    def test_ae_t3_ambient_fake_validator_blocked(self):
        rc, out = self._run_adapter("ae_t3", {
            "SLV_TEST_MODE": "1", "SLV_TEST_VALIDATOR": str(self.fake_validator)})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)

    def test_ae_t4_ambient_alt_lock_blocked(self):
        rc, out = self._run_adapter("ae_t4", {
            "SLV_TEST_MODE": "1", "SLV_LOCK_FILE": str(self.tmp / "alt.lock")})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)

    def test_ae_t5_combined_contamination_blocked(self):
        rc, out = self._run_adapter("ae_t5", {
            "SLV_TEST_MODE": "1",
            "SLV_RUNNER": str(self.fake_runner),
            "SLV_TEST_NODE_CMD": str(self.fake_node),
            "SLV_TEST_VALIDATOR": str(self.fake_validator),
            "SLV_LOCK_FILE": str(self.tmp / "alt.lock")})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)
        for sentinel in ("AMBIENT_FAKE_RUNNER_EXECUTED", "AMBIENT_FAKE_NODE_EXECUTED",
                         "AMBIENT_FAKE_VALIDATOR_EXECUTED"):
            self.assertNotIn(sentinel, out)

    def test_ae_t6_unknown_future_test_hook_blocked(self):
        rc, out = self._run_adapter("ae_t6", {
            "SLV_TEST_MODE": "1", "SLV_TEST_FUTURE_FAKE": "1"})
        self.assertEqual(rc, 2)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)

    def test_ae_t7_clean_environment_guard_pass(self):
        # no ambient test vars: the adapter proceeds to the canonical
        # supervisor and reaches the bounded production preflight (identities
        # captured); the run is then cancelled BEFORE the estimator child —
        # no scientific estimator execution, no bag playback.
        run_id = "ae_t7"
        self.runs.append(run_id)
        env = dict(os.environ)
        for k in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR",
                  "SLV_RUNNER", "SLV_LOCK_FILE"):
            env.pop(k, None)
        env["SLV_SEMANTIC_PROFILE"] = "D_VISUAL_SHADOW"
        env["SLV_LEGACY_ALIAS"] = "d0"
        p = subprocess.Popen(["bash", str(ADAPTER), run_id], env=env,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             cwd="/tmp")
        run = ADAPTER_OUT_ROOT / run_id
        evidence = ""
        for _ in range(100):
            ev = run / "preflight_evidence.txt"
            if ev.exists():
                evidence = ev.read_text()
                if "RUNNER_IDENTITY:" in evidence:
                    break
            time.sleep(0.1)
        self.assertIn("RUNNER_IDENTITY: ", evidence)
        self.assertIn("SUPERVISOR_IDENTITY: ", evidence)
        # cancel before any estimator child starts
        (run / "cancel").touch()
        out, _ = p.communicate(timeout=60)
        out = out.decode()
        self.assertNotIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)
        st = json.loads((run / "state.json").read_text())
        self.assertEqual(st["state"], "CANCELLED")

    def test_ae_t8_clean_adapter_no_algorithm_authority(self):
        text = open(ADAPTER).read()
        for protected in ("/lio/v4/apply", "imu_fullscan",
                          "/lio/camera_epoch/lidar_update_policy",
                          "visual_measurement_event"):
            self.assertNotIn(protected, text)


if __name__ == "__main__":
    unittest.main()
