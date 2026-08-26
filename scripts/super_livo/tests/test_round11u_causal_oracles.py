#!/usr/bin/env python3
"""Round 11U unit tests for causal scheduling and slice-accounting oracles."""
import pathlib
import sys


ROOT = pathlib.Path(__file__).resolve().parents[3]
DATASETS = ROOT / "scripts/super_livo/datasets"
sys.path.insert(0, str(DATASETS))

from audit_causal_epoch_availability import CausalEpochSimulator
from audit_lidar_scan_end import monotonicity, production_scan_timing
from audit_lidar_slice_accounting import SliceSimulator


class Stamp:
    def __init__(self, nanoseconds):
        self.nanoseconds = int(nanoseconds)

    def to_nsec(self):
        return self.nanoseconds


class Header:
    def __init__(self, nanoseconds):
        self.stamp = Stamp(nanoseconds)


class Point:
    def __init__(self, offset_time, x=3.0, tag=0):
        self.offset_time = offset_time
        self.x = x
        self.y = 0.0
        self.z = 0.0
        self.tag = tag


class LivoxMessage:
    def __init__(self, start_ns, offsets):
        self.header = Header(start_ns)
        self.points = [Point(offset) for offset in offsets]
        self.point_num = len(self.points)


def test_scan_timing():
    # U-T1: production filtering and ns offsets define the exact scan end.
    message = LivoxMessage(1_000_000_000, [0, 10, 20, 30, 40, 50,
                                           60, 70, 80, 90, 100, 110])
    timing = production_scan_timing(message, filter_rate=3)
    assert timing["scan_start_ns"] == 1_000_000_000
    assert timing["scan_duration_ns"] == 90
    assert timing["scan_end_ns"] == 1_000_000_090
    assert timing["production_last_offset_ns"] == 90
    assert timing["last_offset_matches_max"] is True

    # U-T2: a next-message timestamp is not an input and cannot change this end.
    later = LivoxMessage(99_000_000_000, [0] * 12)
    assert production_scan_timing(message, filter_rate=3) == timing
    assert production_scan_timing(later, filter_rate=3)["scan_start_ns"] \
        == 99_000_000_000


def test_monotonicity():
    # U-T3: equals are legal; only strict negative steps fail.
    report = monotonicity([10, 10, 12, 9, 15])
    assert report["message_count"] == 5
    assert report["equal_stamp_count"] == 1
    assert report["negative_step_count"] == 1
    assert report["negative_step_ns"]["max"] == 3


def test_causal_availability():
    simulator = CausalEpochSimulator()
    simulator.add_imu(1_100, record_ns=10)
    simulator.add_camera(1_050, record_ns=20)
    assert not simulator.process_once(record_ns=20)
    assert simulator.report()["camera_epochs_waited_for_lidar"] == 1

    # U-T4/U-T6: a scan that exists only in future bag contents cannot be used.
    assert simulator.report()["camera_epochs_emitted"] == 0
    simulator.add_lidar(1_000, 1_100, record_ns=30)
    # U-T5: once already-delivered LiDAR and IMU cover tc, emission is legal.
    assert simulator.process_once(record_ns=30)
    report = simulator.report()
    assert report["camera_epochs_emitted"] == 1
    assert report["emit_without_lidar_coverage"] == 0
    assert report["emit_without_imu_coverage"] == 0
    assert report["strict_three_term_mismatch"] == 0
    assert report["unexplained_camera_accounting_mismatch"] == 0


def test_slice_identity_and_wrong_side():
    simulator = SliceSimulator()
    simulator.add_imu(2.0)
    simulator.add_scan(1.0, list(enumerate([0.00, 0.03, 0.05, 0.07])))
    simulator.add_camera(1.05)
    assert simulator.process_once()
    # U-T7: exact-boundary point is emitted once in the current slice.
    assert simulator.status[2] == 1
    assert simulator.duplicate_emissions == 0
    assert simulator.final_report()["boundary_equality_count"] == 1

    simulator.add_scan(1.1, list(enumerate([0.00, 0.03, 0.06])))
    simulator.add_camera(1.06)
    assert simulator.process_once()
    # Production's wholesale pending emission is intentionally audited, not hidden.
    assert simulator.final_report()["wrong_side_count"] == 1


def test_slice_conservation_and_fault_detection():
    # U-T8/U-T9: multiple cuts plus EOF retain every stable identity exactly once.
    simulator = SliceSimulator()
    simulator.add_imu(3.0)
    simulator.add_scan(1.0, list(enumerate([0.00, 0.02, 0.04, 0.06])))
    simulator.add_camera(1.03)
    assert simulator.process_once()
    simulator.add_scan(1.1, list(enumerate([0.00, 0.02, 0.04])))
    simulator.add_camera(1.05)
    assert simulator.process_once()
    report = simulator.final_report()
    assert report["raw_scan_count"] == 2
    assert report["conservation_pass"] is True
    assert report["lost_point_count"] == 0
    assert report["duplicate_emission_count"] == 0

    # U-T10: duplicate emission is detected.
    duplicate = SliceSimulator()
    duplicate.add_scan(1.0, [(0, 0.0)])
    point = duplicate.scans[0]["points"][0]
    duplicate.emit(point, 1.0)
    duplicate.emit(point, 1.0)
    assert duplicate.final_report()["duplicate_emission_count"] == 1
    assert duplicate.final_report()["conservation_pass"] is False

    # U-T11: removing an unemitted identity from all retained state is a loss.
    lost = SliceSimulator()
    lost.add_scan(1.0, [(0, 0.0)])
    lost.scans.clear()
    assert lost.final_report()["lost_point_count"] == 1
    assert lost.final_report()["conservation_pass"] is False


def main():
    test_scan_timing()
    test_monotonicity()
    test_causal_availability()
    test_slice_identity_and_wrong_side()
    test_slice_conservation_and_fault_detection()
    print("ROUND11U CAUSAL ORACLES U-T1..U-T11: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
