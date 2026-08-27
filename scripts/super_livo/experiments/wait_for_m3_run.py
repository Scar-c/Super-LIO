#!/usr/bin/env python3
"""Watch one M3 transaction; interruption cancels its authoritative supervisor."""
import argparse
import json
import os
import pathlib
import signal
import sys
import time

TERMINAL = {"SUCCESS", "FAILED", "CANCELLED"}

def start_token(pid):
    try:
        return pathlib.Path(f"/proc/{pid}/stat").read_text().split()[21]
    except (FileNotFoundError, ProcessLookupError, PermissionError):
        return None

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--run-dir", required=True)
    ap.add_argument("--supervisor-pid", type=int)  # compatibility only
    ap.add_argument("--poll", type=float, default=1.0)
    ap.add_argument("--max-wall", type=float, default=1800.0)
    args = ap.parse_args()
    run_dir = pathlib.Path(args.run_dir)
    state_path = run_dir / "state.json"
    cancel_path = run_dir / "cancel"
    interrupted = False
    last = None
    started = time.time()

    def read_state():
        try:
            return json.loads(state_path.read_text())
        except (FileNotFoundError, ValueError):
            return {}

    def cancel_transaction(_sig, _frame):
        nonlocal interrupted
        if interrupted:
            return
        interrupted = True
        cancel_path.write_text("user-interrupt\n")
        state = read_state()
        pid = state.get("supervisor_pid")
        expected = state.get("supervisor_start_token")
        print("[watch] interrupt -> transaction cancel request", flush=True)
        if pid and expected and start_token(pid) == str(expected):
            os.kill(pid, signal.SIGTERM)

    signal.signal(signal.SIGINT, cancel_transaction)
    signal.signal(signal.SIGTERM, cancel_transaction)
    while time.time() - started <= args.max_wall:
        state = read_state()
        key = (state.get("state"), state.get("failure_class"), state.get("cleanup_verified"))
        if key != last:
            print(f"[run] state={key[0]} class={key[1] or ''} cleanup_verified={str(bool(key[2])).lower()}", flush=True)
            last = key
        if state.get("state") in TERMINAL and state.get("cleanup_verified") is True:
            print(f"RUN_TERMINAL state={state['state']} class={state.get('failure_class', '')} cleanup_verified=true", flush=True)
            return 0 if state["state"] == "SUCCESS" else 1
        pid = state.get("supervisor_pid")
        expected = state.get("supervisor_start_token")
        if pid and expected and start_token(pid) != str(expected) and state.get("state") not in TERMINAL:
            print("SUPERVISOR_FAILURE authoritative owner absent/token mismatch", flush=True)
            return 2
        time.sleep(args.poll)
    print("WATCHER_TIMEOUT", flush=True)
    return 3

if __name__ == "__main__":
    sys.exit(main())
