#!/usr/bin/env python3
"""NTU-TX1..TX8 public lifecycle contract for both parent adapters."""
import json
import os
import pathlib
import signal
import subprocess
import tempfile
import time

ROOT = pathlib.Path(__file__).resolve().parents[3]
RUNNER = ROOT / "scripts/super_livo/experiments/run_ntu_transaction.sh"
WATCHER = ROOT / "scripts/super_livo/experiments/wait_for_m3_run.py"

def exe(path, body):
    path.write_text("#!/bin/bash\nset -u\n" + body)
    path.chmod(0o755)

def read_state(run):
    return json.loads((run / "state.json").read_text())

def wait_state(run, wanted, timeout=6):
    end = time.time() + timeout
    while time.time() < end:
        try:
            if read_state(run)["state"] == wanted:
                return
        except (FileNotFoundError, ValueError):
            pass
        time.sleep(0.03)
    raise AssertionError((wanted, read_state(run)))

with tempfile.TemporaryDirectory(prefix="ntu-tx-") as td:
    root = pathlib.Path(td); fake = root / "bin"; fake.mkdir(); runs = root / "runs"; runs.mkdir()
    bag = root / "sbs_01.bag"; cfg = root / "ntu.yaml"; launch = root / "ntu.launch"; output = root / "trajectory.tum"
    for path in (bag, cfg, launch): path.write_text("fixture\n")
    exe(fake / "roscore", "exec sleep 60\n")
    exe(fake / "rosparam", 'if [ "$1" = dump ]; then echo "/parent/config_loaded: true" > "$2"; fi\nexit 0\n')
    exe(fake / "rosnode", '[ "${NTU_FAKE_LAUNCH_FAIL:-0}" = 1 ] && exit 1\nprintf "/parent_node\\n"\n')
    exe(fake / "roslaunch", '[ "${NTU_FAKE_LAUNCH_FAIL:-0}" = 1 ] && exit 7\nexec sleep 60\n')
    exe(fake / "rosbag", 'sleep "${NTU_FAKE_PLAY_SEC:-0.2}"\n[ "${NTU_FAKE_EMPTY:-0}" = 1 ] && : > "$NTU_OUTPUT_PATH" || printf "1 0 0 0 0 0 0 1\\n2 0 0 0 0 0 0 1\\n" > "$NTU_OUTPUT_PATH"\n')
    base = os.environ.copy(); base.update(PATH=f"{fake}:{base['PATH']}", NTU_TEST_MODE="1", NTU_RUN_MODE="smoke", NTU_BAG=str(bag), NTU_CONFIG=str(cfg), NTU_LAUNCH_PATH=str(launch), NTU_LAUNCH_PACKAGE="fixture", NTU_LAUNCH_FILE="ntu.launch", NTU_REQUIRED_NODE="parent_node", NTU_OUTPUT_PATH=str(output), NTU_MIN_ROWS="2", NTU_LOCK_FILE=str(root / "ntu.lock"))

    def start(name, extra=None):
        env = base.copy(); env.update(extra or {}); run = runs / name
        return subprocess.Popen([str(RUNNER), name, str(runs)], env=env), run

    # NTU-TX1 missing input fails before playback.
    p, run = start("missing", {"NTU_CONFIG": str(root / "absent")}); assert p.wait(timeout=4) != 0
    assert read_state(run)["failure_class"] == "STATIC_PREFLIGHT_FAIL" and not (run / "play.log").exists()

    # NTU-TX2/TX3/TX4: isolated master, pre-play snapshot and loaded parent config.
    p, run = start("success1"); assert p.wait(timeout=8) == 0; state = read_state(run)
    assert state["state"] == "SUCCESS" and state["cleanup_verified"] is True
    assert (run / "effective_rosparams.after_launch.yaml").exists()
    assert (run / "snapshot_before_playback").exists()
    assert "config_loaded" in (run / "effective_rosparams.after_launch.yaml").read_text()

    # NTU-TX5 launch failure blocks playback.
    p, run = start("launch_fail", {"NTU_FAKE_LAUNCH_FAIL": "1"}); assert p.wait(timeout=8) != 0
    assert read_state(run)["failure_class"] == "PROCESS_LIFECYCLE_FAIL" and not (run / "play.log").exists()

    # NTU-TX6 missing/empty output is OUTPUT_FAIL.
    p, run = start("empty", {"NTU_FAKE_EMPTY": "1"}); assert p.wait(timeout=8) != 0
    assert read_state(run)["failure_class"] == "OUTPUT_FAIL"

    # NTU-TX7 watcher interruption cancels authoritative supervisor and cleans only owned groups.
    p, run = start("cancel", {"NTU_FAKE_PLAY_SEC": "5"}); wait_state(run, "PLAYBACK_STARTED")
    watcher = subprocess.Popen([str(WATCHER), "--run-dir", str(run), "--poll", "0.03", "--max-wall", "8"])
    time.sleep(0.1); watcher.send_signal(signal.SIGINT); assert watcher.wait(timeout=8) == 1; p.wait(timeout=8)
    state = read_state(run); assert state["state"] == "CANCELLED" and state["cleanup_verified"] is True

    # NTU-TX8 two sequential smokes leave no duplicate/residual transaction.
    for name in ("smoke1", "smoke2"):
        p, run = start(name); assert p.wait(timeout=8) == 0; assert read_state(run)["cleanup_verified"] is True

print("NTU TRANSACTION: ALL PASS")
