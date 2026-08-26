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

    print("LIDAR SLICE ACCOUNTING TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
