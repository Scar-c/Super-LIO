#!/usr/bin/env python3
"""Integration test for signed camera/LiDAR phase audit."""
import json
import pathlib
import subprocess
import sys
import tempfile

import rosbag
import rospy
from sensor_msgs.msg import Imu


ROOT = pathlib.Path(__file__).resolve().parents[3]
AUDITOR = ROOT / "scripts/super_livo/datasets/audit_camera_lidar_phase.py"


def stamped(timestamp):
    message = Imu()
    message.header.stamp = rospy.Time.from_sec(timestamp)
    return message


def main():
    with tempfile.TemporaryDirectory(prefix="round11t-phase-") as tmp:
        tmp = pathlib.Path(tmp)
        bag_path = tmp / "phase.bag"
        json_path = tmp / "phase.json"
        with rosbag.Bag(str(bag_path), "w") as bag:
            events = []
            events.extend((time + 0.001, "/lidar", time)
                          for time in (0.0, 0.1, 0.2, 0.3, 0.4))
            events.extend((time + 0.002, "/camera", time)
                          for time in (0.02, 0.12, 0.22, 0.32))
            for record, topic, header in sorted(events):
                bag.write(topic, stamped(header), t=rospy.Time.from_sec(record))

        result = subprocess.run(
            [
                sys.executable,
                str(AUDITOR),
                "--bag", str(bag_path),
                "--camera-topic", "/camera",
                "--lidar-topic", "/lidar",
                "--json-out", str(json_path),
            ],
            cwd=str(ROOT),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if result.returncode != 0:
            raise AssertionError(result.stdout)
        report = json.loads(json_path.read_text(encoding="utf-8"))
        assert report["camera_count"] == 4, report
        assert report["lidar_count"] == 5, report
        assert abs(report["signed_dt"]["p50"] - 0.02) < 1e-9, report
        assert abs(report["absolute_dt"]["p99"] - 0.02) < 1e-9, report
        assert abs(report["phase_drift_slope"]) < 1e-9, report
        assert abs(report["phase_modulo_lidar_period"]["p50"] - 0.02) < 1e-9, report
        assert abs(report["camera_interarrival"]["p50"] - 0.1) < 1e-9, report
        assert abs(report["lidar_interarrival"]["p50"] - 0.1) < 1e-9, report
        for token in (
            "signed dt:",
            "absolute |dt|:",
            "phase vs time:",
            "phase modulo LiDAR period:",
            "fixed offset claimed: NO",
        ):
            assert token in result.stdout, (token, result.stdout)

    print("CAMERA/LIDAR PHASE TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
