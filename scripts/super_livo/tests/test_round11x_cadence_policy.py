#!/usr/bin/env python3
"""Round 11X policy and parallel-safety contract tests."""
import pathlib
import re
import sys


ROOT = pathlib.Path(__file__).resolve().parents[3]


def expect(name, condition):
    print(f"{name}: {'PASS' if condition else 'FAIL'}")
    if not condition:
        raise AssertionError(name)


def test_parallel_result_masks_are_not_bit_packed():
    source = (ROOT / "src/super_lio/src/ros/ROSWrapper.cpp").read_text(
        encoding="utf-8"
    )
    match = re.search(
        r"void\s+livox2pcl\s*\([^)]*\)\s*\{(?P<body>.*?)\n\}",
        source,
        flags=re.DOTALL,
    )
    expect("X-T10 livox2pcl function found", match is not None)
    body = match.group("body")
    bit_packed = re.search(r"std::vector\s*<\s*bool\s*>", body)
    expect("X-T10 parallel result mask uses byte/word storage", bit_packed is None)
    expect("X-T10 parallel result mask has per-index writes", "is_valid_pt[i]" in body)
    expect(
        "X-T10 parallel worker reads immutable previous Livox point",
        "cloud_full->at(i - 1)" not in body and "msg->points[i - 1]" in body,
    )


def test_lidar_hb_has_deterministic_serial_commit():
    source = (ROOT / "src/super_lio/src/lio/super_lio.cpp").read_text(
        encoding="utf-8"
    )
    observe = source[source.index("void SuperLIO::Observe()"):
                     source.index("void SuperLIO::runG1Shadow")]
    expect(
        "X-T10 LiDAR H/b has no thread-local floating reduction",
        "enumerable_thread_specific<ThreadACC>" not in observe,
    )
    expect(
        "X-T10 LiDAR H/b uses per-index POD output",
        "std::vector<PointACC> point_acc" in observe and "point_acc[r_s]" in observe,
    )
    expect(
        "X-T10 LiDAR H/b commits in canonical index order",
        "for(size_t r_s = 0; r_s < effect_knn_num_; ++r_s)" in observe,
    )


def test_production_cadence_wiring():
    params_h = (ROOT / "src/super_lio/include/lio/params.h").read_text()
    params_cpp = (ROOT / "src/super_lio/src/lio/params.cpp").read_text()
    wrapper_h = (ROOT / "src/super_lio/include/ros/ROSWrapper.h").read_text()
    wrapper_cpp = (ROOT / "src/super_lio/src/ros/ROSWrapper.cpp").read_text()
    ds_h = (ROOT / "src/super_lio/include/common/ds.h").read_text()
    super_h = (ROOT / "src/super_lio/include/lio/super_lio.h").read_text()
    super_cpp = (ROOT / "src/super_lio/src/lio/super_lio.cpp").read_text()

    expect("X-T1 typed global policy",
           "LidarUpdatePolicy g_lidar_update_policy" in params_cpp and
           "extern LidarUpdatePolicy g_lidar_update_policy" in params_h)
    expect("X-T1 startup parser wired",
           '"/lio/camera_epoch/lidar_update_policy"' in wrapper_cpp and
           "parseLidarUpdatePolicy" in wrapper_cpp)
    expect("X-T2 partial remains explicit",
           "g_lidar_update_policy == LidarUpdatePolicy::PARTIAL" in wrapper_cpp and
           "return sync_camera_epoch(meas)" in wrapper_cpp)
    expect("X-T3 shadow accounting action wired",
           "CadenceAction::ACCOUNT_CAMERA_ONLY" in wrapper_cpp)
    expect("X-T4/X-T7 full scan measure identity wired",
           "MeasureKind::FULL_LIDAR" in wrapper_cpp and
           "recordGeometryUse" in wrapper_cpp)
    expect("X-T5 IMU-only measure wired",
           "enum class MeasureKind" in ds_h and "IMU_ONLY," in ds_h and
           "CadenceAction::IMU_ONLY" in wrapper_cpp and
           "statePropagateOnly" in super_cpp)
    expect("X-T6 segmented undistortion history preserved",
           "fullscan_propagate_states_" in super_h and
           "fullscan_propagate_states_" in super_cpp)
    expect("X-T8 raw scan identity wired",
           "raw_scan_id" in ds_h and "raw_scan_seq_" in wrapper_h)
    expect("X-T9 B0 legacy route retained",
           "if (!g_lio_camera_epoch)" in wrapper_cpp and
           "return sync_legacy_lidar_end(meas)" in wrapper_cpp)


def main():
    test_production_cadence_wiring()
    test_parallel_result_masks_are_not_bit_packed()
    test_lidar_hb_has_deterministic_serial_commit()
    print("ROUND11X CADENCE POLICY TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
