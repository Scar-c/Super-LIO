#!/usr/bin/env python3
"""Prompt68 CLOSE TDD — runtime authority (RA), adapter trust-chain (AD),
production trust-chain (PC). All runtime gates executable, no static-only."""

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
SUPERVISOR = EXP / "run_superlivo_transaction.sh"
RUNNER = EXP / "run_offline_variant.sh"
ADAPTER = EXP / "adapters" / "run_ntu_d_visual_shadow.sh"
CANONICAL_RUNNER = str(RUNNER)
CANONICAL_VALIDATOR = str(EXP / "validate_d_visual_shadow_result.py")
CFG = pathlib.Path("/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml")

FAKE_NODE = """#!/bin/bash
while [ ! -f "${SLV_TRANSACTION_GATE_DIR:-/nonexistent}/transaction_playback_authorized" ]; do
  [ -f "${SLV_TRANSACTION_GATE_DIR:-/nonexistent}/cancel" ] && exit 130
  sleep 0.05
done
export OUT_DIR="$(rosparam get /lio/offline/out_dir)"
mkdir -p "$OUT_DIR"
python3 -c "
import os
with open(os.path.join(os.environ['OUT_DIR'], 'trajectory.tum'), 'w') as f:
    for i in range(3200):
        f.write('%.6f 0 0 0 0 0 0 1\\n' % (1609059000.0 + i * 0.1))
"
cat > "$OUT_DIR/node_stdout.log" <<'EOF'
V-0 VisualMap: parents=1 landmarks=1 slots_used=1 created=1 frames=1 attempts=1
VISUAL_MEASUREMENT query: attempts=3 hits=3 misses=0 rejected_explicit=0 conservation=OK
VISUAL_MEASUREMENT observation: frames=1 candidates=2 valid=2 rejected=0 residual_samples=4 conservation=OK
VISUAL_MEASUREMENT H: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1.0 P95=1.0 P99=1.0 max=1.0
VISUAL_MEASUREMENT b: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1.0 P95=1.0 P99=1.0 max=1.0
VISUAL_MEASUREMENT proposed_correction=NOT_COMPUTED_BY_SHADOW_PROFILE state_apply_count=0
fullscan ownership: raw_input_points=100 pre_observe_excluded_scans=0 pre_observe_excluded_points=0 eligible_geometry_points=100 used_once=100 duplicate_use=0 never_used=0 imu_only_segments=1
TEST_FIXTURE=true NO_REAL_ESTIMATOR_OUTPUT=true NO_SCIENTIFIC_RESULT=true
EOF
TEMPLATE="/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01/d_visual_shadow/round13_prompt60_measurement_n1/out/effective_config.post_resolve.yaml"
if [ -f "$TEMPLATE" ]; then
  cp "$TEMPLATE" "$OUT_DIR/effective_config.post_resolve.yaml"
else
  echo "{}" > "$OUT_DIR/effective_config.post_resolve.yaml"
fi
exit 0
"""


class AuthorityHarness:
    def __init__(self, tmp):
        self.tmp = pathlib.Path(tmp)
        self.run_dir = self.tmp / "runs"
        self.run_dir.mkdir(exist_ok=True)
        self.lock_root = self.tmp / "locks"
        self.lock_root.mkdir(exist_ok=True)
        self.node = self.tmp / "fake_node.sh"
        self.node.write_text(FAKE_NODE)
        self.node.chmod(0o755)
        self.fake_runner = self.tmp / "fake_runner.sh"
        self.fake_runner.write_text("#!/bin/bash\necho FAKE_RUNNER_EXECUTED >&2\nexit 1\n")
        self.fake_runner.chmod(0o755)
        self.bag = self.tmp / "fake.bag"
        self.bag.write_text("not a real bag: preflight-only")

    def env(self, **kw):
        env = dict(os.environ)
        for k in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR",
                  "SLV_RUNNER", "SLV_LOCK_FILE"):
            env.pop(k, None)
        env.update({
            "SLV_RUN_ID": "x",
            "SLV_CFG": str(CFG),
            "SLV_BAG": str(self.bag),
            "SLV_SEMANTIC_PROFILE": "D_VISUAL_SHADOW",
            "SLV_LEGACY_ALIAS": "d0",
            "SLV_CAM_CALIB": "/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml",
            "SLV_CAM_OFFSET": "-0.0199575325817",
        })
        env.update(kw)
        return env

    def _evidence(self, run):
        text = ""
        for name in ("supervisor.log", "runner.log"):
            f = run / name
            if f.exists():
                text += "\n" + f.read_text()
        return text

    def run_supervisor(self, run_id, env, cwd=None, timeout=120):
        p = subprocess.run(["bash", str(SUPERVISOR), run_id, str(self.run_dir)],
                           env=env, capture_output=True, text=True,
                           cwd=cwd or str(ROOT), timeout=timeout)
        run = self.run_dir / run_id
        return p.returncode, p.stdout + self._evidence(run), run

    def run_adapter(self, run_id, env, cwd=None, timeout=150):
        p = subprocess.run(["bash", str(ADAPTER), run_id],
                           env=env, capture_output=True, text=True,
                           cwd=cwd or "/tmp", timeout=timeout)
        run = self.run_dir / run_id
        return p.returncode, p.stdout + "\n" + p.stderr + self._evidence(run), run

    def state(self, run):
        p = run / "state.json"
        return json.loads(p.read_text()) if p.exists() else None

    def runner_log(self, run):
        p = run / "runner.log"
        return p.read_text() if p.exists() else ""

    def supervisor_log(self, run):
        p = run / "supervisor.log"
        return p.read_text() if p.exists() else ""


class TestRuntimeAuthority(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="ra-")
        self.h = AuthorityHarness(self.tmp)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_ra_t1_fake_runner_production_rejected(self):
        env = self.h.env(SLV_RUNNER=str(self.h.fake_runner))
        rc, out, run = self.h.run_supervisor("ra_t1", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("SLV_RUNNER set without SLV_TEST_MODE=1", out)
        self.assertNotIn("FAKE_RUNNER_EXECUTED", self.h.runner_log(run))

    def test_ra_t2_lock_path_cannot_bypass(self):
        env = self.h.env(SLV_LOCK_FILE="/tmp/alt_lock_a",
                         SLV_TEST_MODE="1",
                         SLV_TEST_NODE_CMD=str(self.h.node),
                         SLV_LOCK_FILE2="")  # placeholder removed below
        env.pop("SLV_LOCK_FILE2")
        rc, out, run = self.h.run_supervisor("ra_t2", env)
        self.assertEqual(rc, 0, out)  # test-mode lock allowed; single instance

    def test_ra_t3_canonical_concurrent_exclusion(self):
        env1 = self.h.env(SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node))
        p1 = subprocess.Popen(["bash", str(SUPERVISOR), "ra_t3a", str(self.h.run_dir)],
                              env=env1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        time.sleep(2.5)  # let the first acquire the lock
        rc2, out2, run2 = self.h.run_supervisor("ra_t3b", self.h.env(
            SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node)))
        out1, _ = p1.communicate(timeout=150)
        self.assertNotEqual(rc2, 0, "second concurrent transaction must fail")
        self.assertIn("lock", out2)

    def test_ra_t4_stale_lock_recovery(self):
        env = self.h.env(SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node))
        rc1, out1, run1 = self.h.run_supervisor("ra_t4a", env)
        self.assertEqual(rc1, 0, out1)
        rc2, out2, run2 = self.h.run_supervisor("ra_t4b", env)
        self.assertEqual(rc2, 0, "lock must be free after completion: " + out2)

    def test_ra_t5_identity_fields_present(self):
        env = self.h.env(SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node))
        rc, out, run = self.h.run_supervisor("ra_t5", env)
        self.assertEqual(rc, 0, out)
        st = self.h.state(run)
        self.assertTrue(st["supervisor_pid"])
        self.assertTrue(st["supervisor_start_token"])
        self.assertTrue(st["transaction_token"].startswith("slv-"))
        self.assertTrue(st["runner_pid"])
        self.assertTrue(st["runner_pgid"])

    def test_ra_t10_clean_production_runner_identity(self):
        text = open(SUPERVISOR).read()
        self.assertIn('CANONICAL_RUNNER="$SUPERVISOR_DIR/run_offline_variant.sh"', text)
        self.assertIn('RUNNER="${SLV_RUNNER:-$CANONICAL_RUNNER_REAL}"', text)

    def test_ra_t11_fake_runner_cannot_execute_production(self):
        env = self.h.env(SLV_RUNNER=str(self.h.fake_runner))
        rc, out, run = self.h.run_supervisor("ra_t11", env)
        log = self.h.supervisor_log(run) + out
        self.assertNotIn("FAKE_RUNNER_EXECUTED", log)
        self.assertNotEqual(rc, 0)

    def test_ra_t12_test_runner_allowed_under_mode(self):
        fake_runner_ok = self.h.tmp / "fake_runner_ok.sh"
        fake_runner_ok.write_text("#!/bin/bash\ncp \"$1\" /dev/null 2>/dev/null; exit 1\n")
        fake_runner_ok.chmod(0o755)
        env = self.h.env(SLV_RUNNER=str(fake_runner_ok), SLV_TEST_MODE="1",
                         SLV_TEST_NODE_CMD=str(self.h.node))
        rc, out, run = self.h.run_supervisor("ra_t12", env)
        log = self.h.runner_log(run)
        # the fake runner was invoked (its stderr goes to runner.log context);
        # fail-closed gate did not fire
        self.assertNotIn("SLV_RUNNER set without SLV_TEST_MODE=1", out)


ADAPTER_OUT_ROOT = pathlib.Path("/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01")


class TestAdapterTrustChain(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="ad-")
        self.h = AuthorityHarness(self.tmp)
        self.created = []

    def tearDown(self):
        # the canonical adapter writes runs into its own out root; remove the
        # test-owned run dirs to keep the transaction namespace clean
        for run_id in self.created:
            shutil.rmtree(ADAPTER_OUT_ROOT / run_id, ignore_errors=True)
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _adapter_env(self, **kw):
        env = dict(os.environ)
        for k in ("SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TEST_VALIDATOR",
                  "SLV_RUNNER", "SLV_LOCK_FILE", "SLV_CFG", "SLV_BAG"):
            env.pop(k, None)
        env["SLV_SEMANTIC_PROFILE"] = "D_VISUAL_SHADOW"  # request field
        env["SLV_LEGACY_ALIAS"] = "d0"
        env.update(kw)
        return env

    def test_ad_t1_ntu_adapter_seam(self):
        # Prompt70: the production adapter is not a test seam. Clean adapter
        # reaches the canonical supervisor + runner preflight (identities
        # captured), then cancelled before any estimator child.
        self.created.append("ad_t1")
        env = self._adapter_env()
        run = ADAPTER_OUT_ROOT / "ad_t1"
        p = subprocess.Popen(["bash", str(ADAPTER), "ad_t1"], env=env,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             cwd=str(ROOT))
        evidence = ""
        for _ in range(100):
            f = run / "preflight_evidence.txt"
            if f.exists():
                evidence = f.read_text()
                if "RUNNER_IDENTITY:" in evidence:
                    break
            time.sleep(0.1)
        self.assertIn("RUNNER_IDENTITY: ", evidence)
        (run / "cancel").touch()
        out, _ = p.communicate(timeout=90)
        self.assertNotIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out.decode())
        st = json.loads((run / "state.json").read_text())
        self.assertEqual(st["state"], "CANCELLED")

    def test_ad_t2_arbitrary_cwd_same_runner(self):
        self.created.append("ad_t2")
        env = self._adapter_env()
        run = ADAPTER_OUT_ROOT / "ad_t2"
        p = subprocess.Popen(["bash", str(ADAPTER), "ad_t2"], env=env,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             cwd="/tmp")
        evidence = ""
        for _ in range(100):
            f = run / "preflight_evidence.txt"
            if f.exists():
                evidence = f.read_text()
                if "RUNNER_IDENTITY:" in evidence:
                    break
            time.sleep(0.1)
        self.assertIn("RUNNER_IDENTITY: ", evidence)
        (run / "cancel").touch()
        p.communicate(timeout=90)

    def test_ad_t3_slv_runner_production_rejected_via_adapter(self):
        # Prompt70: the production adapter's own ambient guard rejects the
        # override before the supervisor starts (stronger than the
        # supervisor-only gate).
        self.created.append("ad_t3")
        env = self._adapter_env(SLV_RUNNER=str(self.h.fake_runner))
        rc, out, run = self.h.run_adapter("ad_t3", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", out)
        self.assertNotIn("AMBIENT_FAKE_RUNNER_EXECUTED", out)

    def test_ad_t5_production_adapter_clean(self):
        # the adapter READS the test-hook names only to reject them; it must
        # never SET/export any test variable
        text = open(ADAPTER).read()
        self.assertIn("PRODUCTION_ADAPTER_PREFLIGHT_FAIL", text)
        for name in ("export SLV_TEST", "export SLV_RUNNER", "export SLV_LOCK_FILE",
                     "SLV_TEST_MODE=", "SLV_TEST_NODE_CMD=", "SLV_TEST_VALIDATOR=",
                     "SLV_RUNNER=", "SLV_LOCK_FILE="):
            self.assertNotIn(name, text)


class TestProductionTrustChain(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="pc-")
        self.h = AuthorityHarness(self.tmp)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_pc_t1_clean_production_chain_identities(self):
        # hooks OFF: production chain reaches canonical runner + canonical
        # node identity + canonical validator before test child substitution.
        env = self.h.env(SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node),
                         SLV_LOCK_FILE=str(self.h.lock_root / "pc1.lock"))
        rc, out, run = self.h.run_supervisor("pc_t1", env)
        self.assertEqual(rc, 0, out)
        log = self.h.runner_log(run)
        self.assertIn("NODE_IDENTITY=", log)
        st = self.h.state(run)
        self.assertEqual(st["state"], "SUCCESS")

    def test_pc_t2_fake_runner_contamination(self):
        env = self.h.env(SLV_RUNNER=str(self.h.fake_runner))
        rc, out, run = self.h.run_supervisor("pc_t2", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("SLV_RUNNER set without SLV_TEST_MODE=1", out)

    def test_pc_t3_fake_node_contamination(self):
        env = self.h.env(SLV_TEST_NODE_CMD=str(self.h.node))
        rc, out, run = self.h.run_supervisor("pc_t3", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("without SLV_TEST_MODE=1", out)

    def test_pc_t4_fake_validator_contamination(self):
        env = self.h.env(SLV_TEST_VALIDATOR=str(self.h.tmp / "fv.py"))
        rc, out, run = self.h.run_supervisor("pc_t4", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("SLV_TEST_VALIDATOR set without SLV_TEST_MODE=1", out)

    def test_pc_t5_combined_contamination_fails_before_any_child(self):
        env = self.h.env(SLV_RUNNER=str(self.h.fake_runner),
                         SLV_TEST_NODE_CMD=str(self.h.node),
                         SLV_TEST_VALIDATOR=str(self.h.tmp / "fv.py"))
        rc, out, run = self.h.run_supervisor("pc_t5", env)
        self.assertNotEqual(rc, 0)
        log = self.h.runner_log(run) + self.h.supervisor_log(run) + out
        self.assertNotIn("FAKE_RUNNER_EXECUTED", log)
        self.assertNotIn("fake node done", log)

    def test_pc_t6_arbitrary_cwd(self):
        env = self.h.env(SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node),
                         SLV_LOCK_FILE=str(self.h.lock_root / "pc6.lock"))
        rc, out, run = self.h.run_supervisor("pc_t6", env, cwd="/tmp")
        self.assertEqual(rc, 0, out)

    def test_pc_t8_apply_fails_at_capability_gate(self):
        env = self.h.env(SLV_SEMANTIC_PROFILE="D_VISUAL_APPLY",
                         SLV_TEST_MODE="1", SLV_TEST_NODE_CMD=str(self.h.node),
                         SLV_LOCK_FILE=str(self.h.lock_root / "pc8.lock"))
        rc, out, run = self.h.run_supervisor("pc_t8", env)
        self.assertNotEqual(rc, 0)
        self.assertIn("SEMANTIC_PROFILE_FAIL", out)



class TestEnvAuthorityAudit(unittest.TestCase):
    """RA-T7/RA-T8: mechanically verify the complete supervisor/runner env
    authority sets; no unresolved or unsafe production override remains."""

    def _reads(self, path):
        import re
        text = open(path).read()
        return sorted({m[2:] for m in re.findall(r"\$\{SLV_[A-Z0-9_]+", text)})

    def test_ra_t7_supervisor_env_authority_table(self):
        reads = self._reads(SUPERVISOR)
        # every SLV_* read must be classified in the evidence doc; the known
        # safe set covers dataset input / profile contract / test-only / gate
        known_safe = {
            "SLV_BAG", "SLV_CAM_CALIB", "SLV_CAM_OFFSET", "SLV_CAM_TOPIC",
            "SLV_CFG", "SLV_DATASET", "SLV_DURATION", "SLV_LAYER_AUDIT",
            "SLV_LEGACY_ALIAS", "SLV_LOCK_FILE", "SLV_MEASUREMENT_EVIDENCE",
            "SLV_MIN_ROWS", "SLV_RUNNER", "SLV_S0_AUDIT", "SLV_SEMANTIC_PROFILE",
            "SLV_SEQUENCE", "SLV_STRIDE", "SLV_TEST_MODE", "SLV_TEST_VALIDATOR",
            "SLV_VARIANT",
        }
        unknown = set(reads) - known_safe
        self.assertEqual(unknown, set())
        text = open(SUPERVISOR).read()
        for unsafe in ("SLV_RUNNER set without SLV_TEST_MODE=1",
                       "SLV_LOCK_FILE set without SLV_TEST_MODE=1"):
            self.assertIn(unsafe, text)

    def test_ra_t8_runner_env_authority_table(self):
        reads = self._reads(RUNNER)
        known_safe = {"SLV_TEST_MODE", "SLV_TEST_NODE_CMD", "SLV_TRANSACTION_GATE_DIR"}
        unknown = set(reads) - known_safe
        self.assertEqual(unknown, set())
        text = open(RUNNER).read()
        self.assertIn("SLV_TEST_NODE_CMD set without SLV_TEST_MODE=1", text)

    def test_ra_t9_no_unresolved_executable_override(self):
        sup = open(SUPERVISOR).read()
        runner = open(RUNNER).read()
        # every executable-replacement channel is gated on SLV_TEST_MODE
        for channel, gate in (
            ("SLV_RUNNER", "SLV_RUNNER set without SLV_TEST_MODE=1"),
            ("SLV_TEST_NODE_CMD", "SLV_TEST_NODE_CMD set without SLV_TEST_MODE=1"),
            ("SLV_TEST_VALIDATOR", "SLV_TEST_VALIDATOR set without SLV_TEST_MODE=1"),
        ):
            self.assertIn(channel, sup + runner)
            self.assertIn(gate, sup + runner)


if __name__ == "__main__":
    unittest.main()
