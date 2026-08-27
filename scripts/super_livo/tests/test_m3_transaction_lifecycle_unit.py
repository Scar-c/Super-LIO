#!/usr/bin/env python3
"""Fast contract tests for the M3DGR transaction adapter public entrypoints."""

import json
import os
import pathlib
import signal
import subprocess
import tempfile
import time


ROOT = pathlib.Path(__file__).resolve().parents[3]
RUNNER = ROOT / "scripts/super_livo/experiments/run_m3_transaction.sh"
WATCHER = ROOT / "scripts/super_livo/experiments/wait_for_m3_run.py"


def assert_source_contract():
    source = RUNNER.read_text()
    assert "flock -u 9" not in source, "lock is released before transaction terminates"
    assert "ps -o pgid= -p" in source, "PID/PGID must be read from ps"
    assert "cleanup_verified" in source
    assert "experiment_valid" in source
    assert "process_start_token" in source


def watcher_cancel_contract():
    with tempfile.TemporaryDirectory(prefix="m3-watcher-contract-") as td:
        run_dir = pathlib.Path(td)
        sleeper = subprocess.Popen(["sleep", "60"])
        state = {
            "run_id": "watcher-cancel",
            "state": "PLAYBACK_STARTED",
            "failure_class": "",
            "cleanup_verified": False,
            "supervisor_pid": sleeper.pid,
            "supervisor_start_token": pathlib.Path(
                f"/proc/{sleeper.pid}/stat"
            ).read_text().split()[21],
        }
        (run_dir / "state.json").write_text(json.dumps(state))
        watcher = subprocess.Popen(
            [str(WATCHER), "--run-dir", str(run_dir), "--poll", "0.05", "--max-wall", "5"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        time.sleep(0.15)
        watcher.send_signal(signal.SIGINT)
        sleeper.wait(timeout=2)
        # Model supervisor's EXIT-trap result after the authoritative PID got TERM.
        state.update(
            state="CANCELLED", failure_class="USER_CANCELLED", cleanup_verified=True
        )
        (run_dir / "state.json").write_text(json.dumps(state))
        out, _ = watcher.communicate(timeout=2)
        assert watcher.returncode == 1, out
        assert (run_dir / "cancel").exists()
        assert "cleanup_verified=true" in out


if __name__ == "__main__":
    assert_source_contract()
    watcher_cancel_contract()
    print("TRANSACTION LIFECYCLE UNIT: ALL PASS")
