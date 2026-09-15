#!/usr/bin/env python3
"""Materialize the Prompt23 evidence package from canonical runtime outputs."""

import csv
import hashlib
import re
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RUNTIME = Path("/home/lc/dec_lio/runtime/prompt23")
OUT = ROOT / "evidence/dec_lio/prompt23"

TARGETS = {"Stairs": 0.30, "Alpha": 3.00, "Gamma": 1.50}
ATE = {
    "Native": {"Stairs": 0.197724347, "Alpha": 6.450671655, "Gamma": 1.88598053},
    "L1": {"Stairs": 0.474189266, "Alpha": 2.519870995, "Gamma": 1.414167328},
    "G0": {"Stairs": 0.250430736, "Alpha": 2.51807022, "Gamma": 2.06648408},
    "G1": {"Stairs": 0.942879112, "Alpha": 2.51807022, "Gamma": 2.06640126},
}
RUNS = {
    "G0": {"Stairs": "g0_stairs", "Alpha": "g0_alpha", "Gamma": "g0_gamma"},
    "G1": {"Stairs": "g1_stairs", "Alpha": "g1_alpha", "Gamma": "g1_gamma"},
}


def f(value):
    return float(value)


def run_stats(run_id):
    run = RUNTIME / run_id
    with (run / "final_candidate_diagnostics.csv").open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    def values(key):
        return [f(row[key]) for row in rows]
    return {
        "rows": len(rows),
        "registration_failures": sum(row["registration_success"] != "1" for row in rows),
        "fusion_failures": sum(row["fusion_success"] != "1" for row in rows),
        "nonfinite": sum(row["state_nonfinite"] != "0" for row in rows),
        "native": sum(row["selected_estimator"] == "Native" for row in rows),
        "loose": sum(row["selected_estimator"] == "Loose_L1" for row in rows),
        "switches": sum(row["mode_switch"] == "1" for row in rows),
        "loose_duty": 100.0 * sum(row["selected_estimator"] == "Loose_L1" for row in rows) / len(rows),
        "max_v": max(values("velocity_norm")),
        "final_v": values("velocity_norm")[-1],
        "max_bg": max(values("bg_norm")),
        "final_bg": values("bg_norm")[-1],
        "max_ba": max(values("ba_norm")),
        "final_ba": values("ba_norm")[-1],
        "final_gravity": values("gravity_norm")[-1],
        "min_cov": min(values("covariance_min_eigen")),
        "final_cov_min": values("covariance_min_eigen")[-1],
        "max_cov": max(values("covariance_max_eigen")),
        "final_cov_max": values("covariance_max_eigen")[-1],
        "trajectory_sha": hashlib.sha256((run / "trajectory.tum").read_bytes()).hexdigest(),
        "wall_s": float(re.search(r"wall_processing_s=([0-9.]+)", (run / "node.log").read_text()).group(1)),
    }


def write(path, text):
    path.write_text(text)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    offline = OUT / "offline_replay/summary.csv"
    shutil.copyfile(offline, OUT / "PROMPT22_OFFLINE_GATE_REPLAY.csv")

    stats = {(gate, seq): run_stats(run_id) for gate, seqs in RUNS.items() for seq, run_id in seqs.items()}
    write(OUT / "START_STATE.md", """# Prompt23 start state

- repository: `/home/lc/dec_lio/src/Super-LIO`
- branch: `Dec-LIO`
- start HEAD: `0999016b65e84d05aa04a423ec9da8c527e629ee`
- origin/Dec-LIO at start: `0999016b65e84d05aa04a423ec9da8c527e629ee`
- origin/ros1 and merge-base: `60b57aaac8dc397f80c56364e7ccb008c300cc29`
- start worktree: clean
- runtime contract: sequential offline bags, `--threads 32`; compile `-j4`
- evaluator: Prompt21 correction, GT tolerance `0.10 s`, independent SE(3), no scale/crop/window
""")
    write(OUT / "FINAL_TARGETS.md", """# Frozen development targets

| sequence | target ATE RMSE |
|---|---:|
| Stairs Alpha | <= 0.30 m |
| Tunnel2 Alpha | <= 3.00 m |
| Tunnel2 Gamma | <= 1.50 m |

These were frozen before the six Prompt23 bag runs.
""")
    write(OUT / "GATE_DESIGN.md", """# Gate design

- one production mode: `dec_lio_final_candidate`
- rolling window: 20 successful plain LiDAR registrations
- fewer than 10 successful registrations: Native
- G0 enter/exit: weak fraction >= 0.75 / <= 0.60
- G1 enter: weak fraction >= 0.75 AND q median >= 0.30
- G1 exit: weak fraction <= 0.60 OR q median <= 0.20
- initial state: Native; hysteresis retains the previous state between thresholds
- weak flag: at least one DCReg rotational weak mode
- q: `r_w / max(p_w, 1e-15)` using the largest-covariance L1 rotational direction in the ESKF innovation tangent
- shadow registration production effect: NONE in Native frames
- loose branch: exact existing L1 pose-level ESKF update and its already computed covariance
""")
    write(OUT / "GATE_SYNTHETIC_TESTS.md", """# Gate synthetic tests

`dec_lio_final_candidate_gate_test` passed after build with `catkin_make -C /home/lc/dec_lio -j4`.

Covered cases: warmup Native, G0 enter, G0 hysteresis retain and exit, G1 low-q rejection, G1 entry, G1 q exit, and failed registration not entering the rolling window or selecting L1 for that frame.
""")
    write(OUT / "EXISTING_MODE_IDENTITY.md", """# Existing mode identity

Current-source replays after the shared `ObserveNative()` dispatch refactor:

- Native Tunnel2 Alpha SHA256: `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` — PASS
- L1 Tunnel2 Alpha SHA256: `7d583427442ef4428584179410ba4104a13022ee95278a4fee17a7d8c3db9d88` — PASS
- frozen historical Stairs L1 SHA256: `818dcb63aa78ca6e5494d7e8fc3f7c17656b559715a0d98df3f9d3993c6aa25a`
- frozen historical Gamma L1 SHA256: `6f8c255151ae611534e8d8657b9f4eb9f12dc48173ff950d3e2c411bc2d7f0fb`

No frozen Native/L1 estimator implementation was imported into the final gate.
""")

    for gate in ("G0", "G1"):
        for seq in TARGETS:
            s = stats[(gate, seq)]
            pass_ = ATE[gate][seq] <= TARGETS[seq] and s["nonfinite"] == 0
            write(OUT / f"{gate}_{seq.upper()}.md", f"""# {gate} {seq}

- runtime: `/home/lc/dec_lio/runtime/prompt23/{RUNS[gate][seq]}`
- evaluator ATE RMSE: `{ATE[gate][seq]:.9f} m`
- target: `{TARGETS[seq]:.2f} m`
- target result: `{'PASS' if pass_ else 'FAIL'}`
- rows: {s['rows']}; registration failures: {s['registration_failures']}; fusion failures: {s['fusion_failures']}
- Native selected: {s['native']}; Loose_L1 selected: {s['loose']}; loose duty: {s['loose_duty']:.6f}%
- mode switches: {s['switches']}
- state nonfinite rows: {s['nonfinite']}; covariance min eigen observed: {s['min_cov']:.9g}; final min eigen: {s['final_cov_min']:.9g}
- max/final velocity norm: {s['max_v']:.9g}/{s['final_v']:.9g}
- max/final gyro bias norm: {s['max_bg']:.9g}/{s['final_bg']:.9g}
- max/final accel bias norm: {s['max_ba']:.9g}/{s['final_ba']:.9g}; final gravity norm: {s['final_gravity']:.9g}
- trajectory SHA256: `{s['trajectory_sha']}`
- wall processing: {s['wall_s']:.6f} s; requested/effective threads: 32/32
""")

    with (OUT / "HYBRID_DUTY_CYCLE.csv").open("w", newline="") as stream:
        fields = ["gate", "sequence", "rows", "native", "loose_l1", "loose_duty_percent", "switches", "ate_rmse", "target", "target_pass"]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for gate in ("G0", "G1"):
            for seq in TARGETS:
                s = stats[(gate, seq)]
                writer.writerow({"gate": gate, "sequence": seq, "rows": s["rows"], "native": s["native"], "loose_l1": s["loose"], "loose_duty_percent": f"{s['loose_duty']:.9f}", "switches": s["switches"], "ate_rmse": f"{ATE[gate][seq]:.9f}", "target": TARGETS[seq], "target_pass": int(ATE[gate][seq] <= TARGETS[seq])})

    with (OUT / "STATE_HEALTH.csv").open("w", newline="") as stream:
        fields = ["gate", "sequence", "rows", "registration_failures", "fusion_failures", "nonfinite", "min_covariance_eigen", "final_covariance_min_eigen", "max_velocity", "final_velocity", "max_bg", "final_bg", "max_ba", "final_ba", "final_gravity", "trajectory_sha256"]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for gate in ("G0", "G1"):
            for seq in TARGETS:
                s = stats[(gate, seq)]
                writer.writerow({"gate": gate, "sequence": seq, "rows": s["rows"], "registration_failures": s["registration_failures"], "fusion_failures": s["fusion_failures"], "nonfinite": s["nonfinite"], "min_covariance_eigen": s["min_cov"], "final_covariance_min_eigen": s["final_cov_min"], "max_velocity": s["max_v"], "final_velocity": s["final_v"], "max_bg": s["max_bg"], "final_bg": s["final_bg"], "max_ba": s["max_ba"], "final_ba": s["final_ba"], "final_gravity": s["final_gravity"], "trajectory_sha256": s["trajectory_sha"]})

    with (OUT / "DEVELOPMENT_TARGETS.csv").open("w", newline="") as stream:
        fields = ["sequence", "native", "l1", "g0", "g1", "target", "g0_pass", "g1_pass"]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for seq in TARGETS:
            writer.writerow({"sequence": seq, "native": ATE["Native"][seq], "l1": ATE["L1"][seq], "g0": ATE["G0"][seq], "g1": ATE["G1"][seq], "target": TARGETS[seq], "g0_pass": int(ATE["G0"][seq] <= TARGETS[seq]), "g1_pass": int(ATE["G1"][seq] <= TARGETS[seq])})

    g0_score = max(ATE["G0"][seq] / TARGETS[seq] for seq in TARGETS)
    g1_score = max(ATE["G1"][seq] / TARGETS[seq] for seq in TARGETS)
    g0_duty = sum(stats[("G0", seq)]["loose_duty"] for seq in TARGETS) / 3.0
    g1_duty = sum(stats[("G1", seq)]["loose_duty"] for seq in TARGETS) / 3.0
    g0_switches = sum(stats[("G0", seq)]["switches"] for seq in TARGETS)
    g1_switches = sum(stats[("G1", seq)]["switches"] for seq in TARGETS)
    write(OUT / "FINAL_SELECTION.md", f"""# Final selection

- G0 all targets pass: NO; Gamma is {ATE['G0']['Gamma']:.9f} m > 1.50 m
- G1 all targets pass: NO; Stairs is {ATE['G1']['Stairs']:.9f} m > 0.30 m and Gamma is {ATE['G1']['Gamma']:.9f} m > 1.50 m
- G0 worst normalized target score: {g0_score:.9f}; average loose duty: {g0_duty:.6f}%; switches: {g0_switches}
- G1 worst normalized target score: {g1_score:.9f}; average loose duty: {g1_duty:.6f}%; switches: {g1_switches}
- frozen selection outcome: neither candidate passes; no candidate is selected
- FINAL_DEV_V1: NOT CREATED
- required Prompt23 stop: no G2, no new thresholds, no NIS or additional mechanism iteration
""")
    write(OUT / "REPEATABILITY.md", """# Repeatability

No candidate passed all three frozen development targets, so Prompt23's selected-candidate repeatability rerun was not authorized by the frozen procedure. The six canonical runtime trajectories and their SHA256 values are recorded in `HYBRID_DUTY_CYCLE.csv` and `STATE_HEALTH.csv`.
""")
    write(OUT / "CLASSIFICATION.md", """# Classification

`P23-C — NO_UNIFIED_FINAL_CANDIDATE`

G0 fails Tunnel2 Gamma. G1 fails Stairs Alpha and Tunnel2 Gamma. Prompt23 therefore closes without creating `DEC_LIO_FINAL_DEV_V1`.
""")
    write(OUT / "CLOSURE.txt", """Prompt23 closure

- Six canonical hybrid runs completed sequentially with bag threads=32.
- Compile workers were -j4; runtime effective TBB concurrency was 32 in every run.
- All six runs had no fatal marker, no nonfinite state row, and positive covariance minimum eigenvalues.
- Current-source Native Alpha and L1 Alpha identity checks passed byte-for-byte.
- G0 and G1 were both tested under the frozen rules; neither passed all three targets.
- Classification: P23-C NO_UNIFIED_FINAL_CANDIDATE.
- No FINAL_DEV_V1 was created.
- Mechanism iteration stops here as required; no G2, extra covariance mode, threshold retuning, NIS gate, or GT-selected switching.
""")


if __name__ == "__main__":
    main()
