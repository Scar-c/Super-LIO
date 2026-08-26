#!/usr/bin/env python3
"""Integration test for record/header timing and inversion audit CLI."""
import json
import pathlib
import subprocess
import sys
import tempfile

import rosbag
import rospy
from sensor_msgs.msg import Imu


ROOT = pathlib.Path(__file__).resolve().parents[3]
AUDITOR = ROOT / "scripts/super_livo/datasets/audit_record_header_time.py"


def message(header_time):
    msg = Imu()
    msg.header.stamp = rospy.Time.from_sec(header_time)
    return msg


def main():
    with tempfile.TemporaryDirectory(prefix="round11t-time-") as tmp:
        tmp = pathlib.Path(tmp)
        bag_path = tmp / "synthetic.bag"
        json_path = tmp / "audit.json"
        events = [
            (0.10, "/imu", 0.00),
            (0.20, "/lidar", 0.50),
            (0.30, "/camera", 0.25),  # camera vs lidar inversion
            (1.10, "/imu", 1.00),
            (1.20, "/camera", 0.90),  # camera vs imu inversion
            (1.30, "/lidar", 1.50),
            (2.10, "/imu", 2.00),
            (2.20, "/lidar", 1.90),   # lidar vs imu inversion
        ]
        with rosbag.Bag(str(bag_path), "w") as bag:
            for record_time, topic, header_time in events:
                bag.write(
                    topic,
                    message(header_time),
                    t=rospy.Time.from_sec(record_time),
                )

        result = subprocess.run(
            [
                sys.executable,
                str(AUDITOR),
                "--bag", str(bag_path),
                "--stream", "IMU=/imu",
                "--stream", "LiDAR=/lidar",
                "--stream", "Camera=/camera",
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
        assert report["streams"]["IMU"]["count"] == 3, report
        assert abs(report["streams"]["IMU"]["mean"] - 0.1) < 1e-9, report
        assert abs(report["streams"]["IMU"]["drift_slope"] - 0.0) < 1e-9, report
        inversions = report["header_inversions"]
        assert inversions["all_selected_messages"]["count"] == 3, report
        assert inversions["camera_lidar"]["count"] == 1, report
        assert inversions["camera_imu"]["count"] == 1, report
        assert inversions["lidar_imu"]["count"] == 1, report
        assert abs(inversions["camera_lidar"]["max"] - 0.25) < 1e-9, report
        directional = report["directional_header_inversions"]
        assert directional["IMU -> LiDAR"]["count"] == 1, report
        assert directional["LiDAR -> IMU"]["count"] == 0, report
        assert directional["LiDAR -> Camera"]["count"] == 1, report
        assert directional["Camera -> LiDAR"]["count"] == 0, report
        assert directional["IMU -> Camera"]["count"] == 1, report
        assert directional["Camera -> IMU"]["count"] == 0, report
        for token in (
            "git HEAD:",
            "script path:",
            "IMU record-header:",
            "LiDAR record-header:",
            "Camera record-header:",
            "cross-stream differences:",
            "header inversions:",
        ):
            assert token in result.stdout, (token, result.stdout)

    print("RECORD/HEADER AUDIT TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
