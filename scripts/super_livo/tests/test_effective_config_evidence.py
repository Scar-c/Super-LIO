#!/usr/bin/env python3
"""Round11AB effective-configuration evidence contract tests."""

import importlib.util
import tempfile
from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[3]
MODULE_PATH = ROOT / "scripts/super_livo/experiments/run_evidence.py"


def load_module():
    spec = importlib.util.spec_from_file_location("run_evidence", MODULE_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot import {MODULE_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_missing_post_resolve_is_incomplete():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  temporal_stride: 3\n", encoding="utf-8"
        )
        result = evidence.validate_run_dir(run_dir)
        assert not result.complete
        assert result.status == "CONFIG_EVIDENCE_INCOMPLETE"
        assert "effective_config.post_resolve.yaml" in result.reasons


def test_source_yaml_copy_cannot_satisfy_post_resolve():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("camera:\n  temporal_stride: 1\n", encoding="utf-8")
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  temporal_stride: 3\n", encoding="utf-8"
        )
        (run_dir / "effective_config.post_resolve.yaml").write_bytes(
            source.read_bytes()
        )
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert result.status == "CONFIG_EVIDENCE_INCOMPLETE"
        assert "post_resolve_is_source_yaml_copy" in result.reasons


def test_runner_override_must_reach_post_resolve():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("camera:\n  temporal_stride: 1\n", encoding="utf-8")
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  temporal_stride: 3\n", encoding="utf-8"
        )
        post = {
            "schema_version": 1,
            "resolution_complete": True,
            "input": {"camera_temporal_stride": {"value": 1, "source": "rosparam"}},
        }
        (run_dir / "effective_config.post_resolve.yaml").write_text(
            yaml.safe_dump(post, sort_keys=True), encoding="utf-8"
        )
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "post_resolve_mismatch:camera.temporal_stride" in result.reasons


def test_node_default_absent_from_yaml_must_be_recorded():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("camera:\n  enabled: false\n", encoding="utf-8")
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  enabled: false\n", encoding="utf-8"
        )
        post = {
            "schema_version": 1,
            "resolution_complete": True,
            "input": {"camera_temporal_stride": {"value": 1, "source": "default"}},
            "estimator": {},
        }
        (run_dir / "effective_config.post_resolve.yaml").write_text(
            yaml.safe_dump(post, sort_keys=True), encoding="utf-8"
        )
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "missing_resolved_default:camera_frame_buffer_capacity" in result.reasons


def test_derived_option_must_record_source_and_resolved_value():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("lio:\n  sensor:\n    lidar_type: 1\n", encoding="utf-8")
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "lio:\n  sensor:\n    lidar_type: 1\n", encoding="utf-8"
        )
        post = {
            "schema_version": 1,
            "resolution_complete": True,
            "input": {"camera_temporal_stride": {"value": 1, "source": "default"}},
            "estimator": {
                "camera_frame_buffer_capacity": {"value": 10, "source": "default"}
            },
            "lidar": {"point_timestamp_unit": {"value": "nanoseconds"}},
        }
        (run_dir / "effective_config.post_resolve.yaml").write_text(
            yaml.safe_dump(post, sort_keys=True), encoding="utf-8"
        )
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "derived_source_missing:lidar.point_timestamp_unit" in result.reasons


def test_camera_variant_must_match_executed_value():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("camera:\n  enabled: false\n", encoding="utf-8")
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  enabled: true\n  temporal_stride: 1\n", encoding="utf-8"
        )
        post = {
            "schema_version": 1,
            "resolution_complete": True,
            "input": {
                "camera_enabled": {"value": False, "source": "rosparam"},
                "camera_temporal_stride": {"value": 1, "source": "rosparam"},
            },
            "estimator": {
                "camera_frame_buffer_capacity": {"value": 10, "source": "default"}
            },
            "lidar": {
                "point_timestamp_unit": {"value": "nanoseconds", "source": "derived"}
            },
        }
        (run_dir / "effective_config.post_resolve.yaml").write_text(
            yaml.safe_dump(post, sort_keys=True), encoding="utf-8"
        )
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "post_resolve_mismatch:camera.enabled" in result.reasons


def test_d_family_stride_offset_and_policy_match_execution():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td)
        source = run_dir / "source.yaml"
        source.write_text("lio:\n  sensor:\n    lidar_type: 1\n", encoding="utf-8")
        pre = {
            "camera": {"enabled": True, "temporal_stride": 3, "time_offset": 0.1},
            "lio": {"camera_epoch": {"lidar_update_policy": "imu_fullscan"}},
        }
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            yaml.safe_dump(pre, sort_keys=True), encoding="utf-8"
        )
        post = {
            "schema_version": 1,
            "resolution_complete": True,
            "input": {
                "camera_enabled": {"value": True, "source": "rosparam"},
                "camera_temporal_stride": {"value": 3, "source": "rosparam"},
            },
            "time": {"camera_offset_s": {"value": 0.0, "source": "rosparam"}},
            "estimator": {
                "camera_frame_buffer_capacity": {"value": 10, "source": "default"},
                "lidar_update_policy": {"value": "partial", "source": "normalized"},
            },
            "lidar": {
                "point_timestamp_unit": {"value": "nanoseconds", "source": "derived"}
            },
        }
        post_path = run_dir / "effective_config.post_resolve.yaml"
        post_path.write_text(yaml.safe_dump(post, sort_keys=True), encoding="utf-8")
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "post_resolve_mismatch:camera.time_offset" in result.reasons

        post["time"]["camera_offset_s"]["value"] = 0.1
        post_path.write_text(yaml.safe_dump(post, sort_keys=True), encoding="utf-8")
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert not result.complete
        assert "post_resolve_mismatch:lidar_update_policy" in result.reasons

        post["estimator"]["lidar_update_policy"]["value"] = "imu_fullscan"
        post_path.write_text(yaml.safe_dump(post, sort_keys=True), encoding="utf-8")
        result = evidence.validate_run_dir(run_dir, source_config=source)
        assert result.complete


def test_provenance_manifest_tracks_inputs_outputs_and_completion():
    evidence = load_module()
    with tempfile.TemporaryDirectory() as td:
        run_dir = Path(td) / "run"
        run_dir.mkdir()
        source = Path(td) / "source.yaml"
        bag = Path(td) / "input.bag"
        calib = Path(td) / "camera.yaml"
        source.write_text("source: config\n", encoding="utf-8")
        bag.write_bytes(b"bag-fixture")
        calib.write_text("camera: fixture\n", encoding="utf-8")
        evidence.begin_provenance(
            run_dir=run_dir,
            repo_root=ROOT,
            dataset="M3DGR",
            sequence="Corridor01",
            variant="d_s3",
            command_line="run_offline_variant.sh ...",
            source_config=source,
            bags=[bag],
            camera_calibration=calib,
        )
        (run_dir / "effective_rosparams.pre_node.yaml").write_text(
            "camera:\n  enabled: true\n  temporal_stride: 3\n  time_offset: 0.1\n"
            "lio:\n  camera_epoch:\n    lidar_update_policy: imu_fullscan\n",
            encoding="utf-8",
        )
        (run_dir / "effective_config.post_resolve.yaml").write_text(
            "schema_version: 1\nresolution_complete: true\n"
            "input:\n  camera_enabled: {value: true, source: rosparam}\n"
            "  camera_temporal_stride: {value: 3, source: normalized}\n"
            "time:\n  camera_offset_s: {value: 0.1, source: rosparam}\n"
            "estimator:\n"
            "  camera_frame_buffer_capacity: {value: 10, source: default}\n"
            "  lidar_update_policy: {value: imu_fullscan, source: normalized}\n"
            "lidar:\n"
            "  point_timestamp_unit: {value: nanoseconds, source: derived}\n",
            encoding="utf-8",
        )
        (run_dir / "trajectory.tum").write_text("1 0 0 0 0 0 0 1\n", encoding="utf-8")
        result = evidence.finalize_provenance(
            run_dir=run_dir, source_config=source, process_return_code=0
        )
        assert result.complete
        manifest = yaml.safe_load((run_dir / "run_provenance.yaml").read_text())
        assert manifest["completion"]["status"] == "COMPLETE"
        assert manifest["source_config"]["sha256"] == evidence.sha256_file(source)
        assert manifest["inputs"][0]["sha256"] == evidence.sha256_file(bag)
        assert manifest["effective_config_post_resolve"]["sha256"]
        assert manifest["trajectory"]["sha256"]
        assert (run_dir / "git_diff.patch").is_file()


if __name__ == "__main__":
    test_missing_post_resolve_is_incomplete()
    test_source_yaml_copy_cannot_satisfy_post_resolve()
    test_runner_override_must_reach_post_resolve()
    test_node_default_absent_from_yaml_must_be_recorded()
    test_derived_option_must_record_source_and_resolved_value()
    test_camera_variant_must_match_executed_value()
    test_d_family_stride_offset_and_policy_match_execution()
    test_provenance_manifest_tracks_inputs_outputs_and_completion()
    print("Round11AB effective-config T1..T7 + provenance: PASS")
