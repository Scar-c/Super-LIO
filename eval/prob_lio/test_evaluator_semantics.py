#!/usr/bin/env python3
"""Prompt11 evaluator semantic tests.

These tests are deliberately synthetic: they validate evaluator routing and
frame/alignment contracts without depending on a particular bag result.
"""

import csv
import importlib.util
import math
import pathlib
import tempfile

import numpy as np
import yaml


ROOT = pathlib.Path(__file__).resolve().parents[2]


def load_module(name, relative):
    path = ROOT / relative
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


ntu = load_module("eval_ntu", "eval/prob_lio/eval_ntu_viral_official.py")
tum = load_module("eval_tum", "eval/prob_lio/eval_tum_translation.py")
m3 = load_module("eval_m3", "eval/prob_lio/eval_m3dgr_aruco.py")
mcd = load_module("prepare_mcd", "eval/prob_lio/prepare_mcd_gt.py")


def pose(rotation, translation):
    result = np.eye(4)
    result[:3, :3] = rotation
    result[:3, 3] = translation
    return result


def quaternion(rotation):
    trace = float(np.trace(rotation))
    w = math.sqrt(1.0 + trace) / 2.0
    return np.array([
        (rotation[2, 1] - rotation[1, 2]) / (4.0 * w),
        (rotation[0, 2] - rotation[2, 0]) / (4.0 * w),
        (rotation[1, 0] - rotation[0, 1]) / (4.0 * w),
        w,
    ])


def tum_rows(poses):
    return np.asarray([
        [float(index), *transform[:3, 3], *quaternion(transform[:3, :3])]
        for index, transform in enumerate(poses)
    ])


def test_ntu():
    gt_times = np.array([0.0, 0.05, 0.10, 0.15])
    gt_positions = np.column_stack(
        (gt_times, gt_times * 0.5, gt_times * -0.25)
    )
    times = np.array([0.01, 0.06, 0.11])
    positions = np.column_stack((times, times * 0.5, times * -0.25))
    quaternions = np.tile([0.0, 0.0, 0.0, 1.0], (len(times), 1))
    result = ntu.evaluate(
        times, positions, quaternions, gt_times, gt_positions, np.zeros(3)
    )
    assert result["primary_metric"] == "NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M"
    assert result["alignment"] == "SE3_UMEYAMA_NO_SCALE"
    assert result["translation_ate_rmse_m"] < 1e-12


def test_tum_no_scale():
    source = np.array([[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 2.0, 0.0]])
    destination = 2.0 * source
    rotation, translation = tum.umeyama_se3(source, destination)
    aligned = (rotation @ source.T).T + translation
    assert np.sqrt(np.mean(np.sum((aligned - destination) ** 2, axis=1))) > 0.1


def test_m3_relative_only():
    angle = 0.31
    rotation = np.array([
        [math.cos(angle), -math.sin(angle), 0.0],
        [math.sin(angle), math.cos(angle), 0.0],
        [0.0, 0.0, 1.0],
    ])
    first = pose(np.eye(3), np.array([3.0, -1.0, 0.5]))
    reference = pose(rotation, np.array([1.2, -0.4, 0.8]))
    last = first @ reference
    metrics = m3.evaluate(
        reference[:3, :3], reference[:3, 3], 10.0,
        np.array([0.0, 1.0]), np.asarray([first, last]),
    )
    assert metrics["primary_metric"] == (
        "M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M"
    )
    assert metrics["translation_error_m"] < 1e-12
    assert metrics["combined_rmse_status"] == "NON_PRIMARY_MIXED_UNITS"


def test_mcd_adapter():
    with tempfile.TemporaryDirectory(prefix="p11-evaluator-") as name:
        directory = pathlib.Path(name)
        source = directory / "pose_inW.csv"
        with source.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(
                stream,
                fieldnames=["num", "t", "x", "y", "z", "qx", "qy", "qz", "qw"],
            )
            writer.writeheader()
            writer.writerow({
                "num": "1", "t": "1645018880.1", "x": "1", "y": "2",
                "z": "3", "qx": "0", "qy": "0", "qz": "0", "qw": "1",
            })
        output = directory / "gt.tum"
        rows, first, last = mcd.convert(source, output)
        assert rows == 1 and first == last == 1645018880.1
        assert abs(float(output.read_text(encoding="utf-8").split()[0]) - 1645018880.1) < 1e-6


def test_registry_metric_routing():
    registry = yaml.safe_load(
        (ROOT / "eval/prob_lio/evaluator_registry.yaml").read_text(encoding="utf-8")
    )
    final_profile = registry["profiles"]["m3dgr_aruco_final_relative"]
    assert final_profile["ground_truth_type"] == "FINAL_RELATIVE_POSE"
    assert "eval_tum_translation.py" in final_profile["forbidden_evaluators"]
    assert registry["profiles"]["mcd_night08_full_trajectory"]["primary_metric"] != (
        "ATE"
    )


if __name__ == "__main__":
    test_ntu()
    test_tum_no_scale()
    test_m3_relative_only()
    test_mcd_adapter()
    test_registry_metric_routing()
    print("Prompt11 evaluator semantics: PASS (5 tests)")
