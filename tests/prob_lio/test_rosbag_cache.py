#!/usr/bin/env python3
"""Tests for the Prompt13 LIO-only rosbag parity seam."""

import importlib.util
import pathlib


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = REPO_ROOT / "tools/prob_lio/compare_rosbag_topics.py"
SPEC = importlib.util.spec_from_file_location("compare_rosbag_topics", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def test_record_summary_is_order_and_payload_sensitive():
    records = [
        ("/hesai/pandar", 10, b"lidar-1"),
        ("/alphasense_driver_ros/imu", 11, b"imu-1"),
        ("/hesai/pandar", 20, b"lidar-2"),
    ]

    first = MODULE.summarize_records(records)
    assert first["records"] == 3
    assert first["topics"]["/hesai/pandar"]["records"] == 2
    assert first["topics"]["/alphasense_driver_ros/imu"]["first_stamp_ns"] == 11

    assert MODULE.compare_summaries(first, MODULE.summarize_records(records))["status"] == "PASS"
    assert MODULE.compare_summaries(
        first, MODULE.summarize_records(list(reversed(records)))
    )["status"] == "FAIL"
    mutated = list(records)
    mutated[-1] = (mutated[-1][0], mutated[-1][1], b"changed")
    assert MODULE.compare_summaries(
        first, MODULE.summarize_records(mutated)
    )["status"] == "FAIL"


def test_empty_or_missing_required_topic_fails():
    summary = MODULE.summarize_records([])
    result = MODULE.compare_summaries(
        summary, MODULE.summarize_records([]),
        required_topics=["/hesai/pandar", "/alphasense_driver_ros/imu"],
    )
    assert result["status"] == "FAIL"
    assert set(result["missing_topics"]) == {
        "/hesai/pandar", "/alphasense_driver_ros/imu"
    }
