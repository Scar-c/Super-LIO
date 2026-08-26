#!/usr/bin/env python3
"""CLI test for identity-based camera-epoch LiDAR slice accounting."""
import json
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[3]
AUDITOR = ROOT / "scripts/super_livo/datasets/audit_lidar_slice_accounting.py"


def main():
    with tempfile.TemporaryDirectory(prefix="round11t-slice-") as tmp:
        tmp = pathlib.Path(tmp)
        events_path = tmp / "events.json"
        report_path = tmp / "report.json"
        # First epoch splits scan 0 at 1.03. FROZEN S0: at epoch 1.05 the
        # pending tail (1.04,1.06,1.08) is re-sliced; only 1.04 <= 1.05 is
        # emitted, 1.06/1.08 remain pending (no premature emission).
        events = [
            {"record": 0.01, "kind": "imu", "header": 2.0},
            {"record": 0.02, "kind": "lidar", "header": 1.0,
             "offsets": [0.00, 0.02, 0.04, 0.06, 0.08]},
            {"record": 0.03, "kind": "camera", "header": 1.03},
            {"record": 0.04, "kind": "lidar", "header": 1.1,
             "offsets": [0.00, 0.02, 0.04, 0.06, 0.08]},
            {"record": 0.05, "kind": "camera", "header": 1.05},
        ]
        events_path.write_text(json.dumps(events), encoding="utf-8")
        result = subprocess.run(
            [
                sys.executable,
                str(AUDITOR),
                "--events-json", str(events_path),
                "--json-out", str(report_path),
            ],
            cwd=str(ROOT),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if result.returncode != 0:
            raise AssertionError(result.stdout)
        report = json.loads(report_path.read_text(encoding="utf-8"))
        assert report["input_valid_selected_points"] == 10, report
        # FROZEN S0: pending tail re-sliced at every epoch. Epoch 1.03 emits 2
        # points (1.00,1.02); epoch 1.05 emits 1 (1.04). 1.06/1.08 stay pending.
        assert report["emitted_points"] == 3, report
        assert report["wrong_side_count"] == 0, report
        assert report["duplicate_emission_count"] == 0, report
        assert report["final_retained_points"] == 7, report
        assert report["lost_point_count"] == 0, report
        assert report["emitted_before_physical_time"] == 0, report
        assert report["conservation_pass"] is True, report
        assert report["slice_points"]["p50"] == 1.5, report
        for token in (
            "stable identity:",
            "input valid selected LiDAR points: 10",
            "emitted points: 3",
            "final retained: 7",
            "duplicate emission count: 0",
            "lost point count: 0",
            "emitted before physical time: 0",
            "conservation: PASS",
        ):
            assert token in result.stdout, (token, result.stdout)

    # --- F3: integer-ns hard boundary (tc_ns-1 / tc_ns / tc_ns+1) ---
    # scan starts at 0.9 s; points at 1.000000000 / 1.000000002 / 1.000000004 s.
    # tc=0.999999999 -> empty slice; tc=1.000000000 -> exact boundary emitted;
    # tc=1.000000001 -> next point emitted; no epsilon branch.
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "lidar", "header_ns": 900000000,
             "offsets_ns": [100000000, 100000001, 100000003]},
            {"record": 0.003, "kind": "camera", "header_ns": 999999999},
            {"record": 0.004, "kind": "camera", "header_ns": 1000000000},
            {"record": 0.005, "kind": "camera", "header_ns": 1000000001},
        ],
        {
            "emitted_points": 2,
            "wrong_side_count": 0,
            "boundary_equality_count": 2,
            "empty_slice_count": 1,
            "final_retained_points": 1,
            "conservation_pass": True,
        },
    )

    # --- F2: raw scan lineage ---
    # two scans before one epoch -> distinct raw scan ids;
    # scan 0 split across two epochs -> same raw scan id.
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "lidar", "header": 1.0,
             "offsets": [0.00, 0.02, 0.04, 0.06]},
            {"record": 0.003, "kind": "lidar", "header": 1.01,
             "offsets": [0.00, 0.02]},
            {"record": 0.004, "kind": "camera", "header": 1.03},
            {"record": 0.005, "kind": "camera", "header": 1.05},
        ],
        {
            "emitted_points": 5,
            "wrong_side_count": 0,
            "distinct_raw_scan_ids_emitted": 2,
            "scans_emitted_across_multiple_epochs": 1,
            "conservation_pass": True,
        },
    )

    # --- F5: camera offset applied exactly once ---
    # offset 0.001 s: epoch = header + 0.001 (NOT 2*offset).
    # points at 1.030/1.032; tc=1.031 -> only 1.030 emits.
    # double application would put tc=1.032 and emit both.
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "lidar", "header": 1.0,
             "offsets": [0.030, 0.032]},
            {"record": 0.003, "kind": "camera", "header": 1.030},
        ],
        {
            "emitted_points": 1,
            "wrong_side_count": 0,
            "final_retained_points": 1,
            "conservation_pass": True,
        },
        offset=0.001,
    )

    # --- F4: pending-only coverage ---
    # pending + adequate IMU + points <= tc -> legal current slice
    # (lidar deque empty after scan consumed).
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "lidar", "header": 1.0,
             "offsets": [0.00, 0.04, 0.08]},
            {"record": 0.003, "kind": "camera", "header": 1.03},
            {"record": 0.004, "kind": "camera", "header": 1.05},
        ],
        {
            "emitted_points": 2,
            "wrong_side_count": 0,
            "final_retained_points": 1,
            "camera_epoch_count": 2,
            "conservation_pass": True,
        },
    )
    # pending-only + all points > tc -> empty-slice path, no fake emission.
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "lidar", "header": 1.0,
             "offsets": [0.08]},
            {"record": 0.003, "kind": "camera", "header": 1.03},
            {"record": 0.004, "kind": "camera", "header": 1.05},
        ],
        {
            "emitted_points": 0,
            "empty_slice_count": 2,
            "final_retained_points": 1,
            "wrong_side_count": 0,
        },
    )

    # --- DRAIN-T1/T2: one LiDAR arrival unlocks tc1/tc2/tc3; tc4 stays ---
    run_events(
        [
            {"record": 0.001, "kind": "imu", "header": 2.0},
            {"record": 0.002, "kind": "camera", "header": 1.02},
            {"record": 0.003, "kind": "camera", "header": 1.03},
            {"record": 0.004, "kind": "camera", "header": 1.04},
            {"record": 0.005, "kind": "camera", "header": 1.06},
            {"record": 0.006, "kind": "lidar", "header": 1.0,
             "offsets": [0.01, 0.02, 0.03, 0.04, 0.05]},
        ],
        {
            "camera_epoch_count": 3,
            "emitted_points": 4,
            "eof_unemitted_cameras": 1,
            "camera_input": 4,
            "camera_unclassified": 0,
            "wrong_side_count": 0,
            "conservation_pass": True,
        },
    )

    # --- DRAIN-T3: IMU arrival provides the final missing coverage ---
    run_events(
        [
            {"record": 0.001, "kind": "lidar", "header": 1.0,
             "offsets": [0.004, 0.008]},
            {"record": 0.002, "kind": "camera", "header": 1.005},
            {"record": 0.003, "kind": "camera", "header": 1.008},
            {"record": 0.004, "kind": "imu", "header": 1.009},
        ],
        {
            "camera_epoch_count": 2,
            "emitted_points": 2,
            "wrong_side_count": 0,
            "camera_unclassified": 0,
            "conservation_pass": True,
        },
    )

    # --- R2-T8: future data cannot make a camera ready (all points > tc) ---
    run_events(
        [
            {"record": 0.001, "kind": "camera", "header": 1.05},
            {"record": 0.002, "kind": "imu", "header": 2.0},
            {"record": 0.003, "kind": "lidar", "header": 1.0,
             "offsets": [0.06, 0.08]},
        ],
        {
            "emitted_points": 0,
            "wrong_side_count": 0,
            "empty_slice_count": 1,
            "camera_unclassified": 0,
            "conservation_pass": True,
        },
    )

    print("LIDAR SLICE ACCOUNTING TDD (F1-F5 + COV + DRAIN-T1..T3 + future): ALL PASS")
    return 0


def run_events(events, expects, offset=0.0):
    with tempfile.TemporaryDirectory(prefix="round11t-slice2-") as tmp:
        tmp = pathlib.Path(tmp)
        events_path = tmp / "events.json"
        report_path = tmp / "report.json"
        events_path.write_text(json.dumps(events), encoding="utf-8")
        result = subprocess.run(
            [
                sys.executable,
                str(AUDITOR),
                "--events-json", str(events_path),
                "--json-out", str(report_path),
                "--camera-time-offset", str(offset),
            ],
            cwd=str(ROOT),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if result.returncode != 0:
            raise AssertionError(result.stdout)
        report = json.loads(report_path.read_text(encoding="utf-8"))
        for key, value in expects.items():
            assert report[key] == value, (key, report)


if __name__ == "__main__":
    sys.exit(main())
