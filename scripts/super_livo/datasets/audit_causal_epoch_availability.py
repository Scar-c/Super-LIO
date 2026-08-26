#!/usr/bin/env python3
"""Record-order oracle for camera-epoch causal availability and accounting."""
import argparse
from collections import deque
import json
import math
import pathlib
import shlex
import sys

from audit_common import distribution, git_head, iter_record_order
from audit_lidar_scan_end import production_scan_timing, stamp_ns


class CausalEpochSimulator:
    """Mirror the current camera-epoch scheduler without estimator execution."""

    def __init__(self, camera_capacity=10):
        self.camera_capacity = camera_capacity
        self.cameras = deque()
        self.scans = deque()
        self.imus = deque()
        self.pending = []
        self.last_imu = -math.inf
        self.latest_available_lidar_end = -math.inf
        self.last_epoch = -math.inf
        self.camera_input = 0
        self.next_camera_id = 0
        self.emitted_ids = set()
        self.waited_lidar = set()
        self.waited_imu = set()
        self.waited_both = set()
        self.buffer_overflow_drop = 0
        self.stale_epoch_drop = 0
        self.empty_slice_drop = 0
        self.emit_without_lidar = 0
        self.emit_without_imu = 0
        self.coverage_lidar = []
        self.coverage_imu = []
        self.emission_delay = []
        self.drained_eof = 0

    def add_imu(self, timestamp_ns, record_ns):
        del record_ns
        timestamp_ns = int(timestamp_ns)
        self.imus.append(timestamp_ns)
        self.last_imu = timestamp_ns

    def add_camera(self, timestamp_ns, record_ns):
        while len(self.cameras) >= self.camera_capacity:
            self.cameras.popleft()
            self.buffer_overflow_drop += 1
        self.cameras.append({
            "id": self.next_camera_id,
            "time": int(timestamp_ns),
            "arrival": int(record_ns),
        })
        self.next_camera_id += 1
        self.camera_input += 1

    def add_lidar(self, start_ns, end_ns, record_ns, point_times=None):
        del record_ns
        start_ns, end_ns = int(start_ns), int(end_ns)
        if point_times is None:
            point_times = [start_ns, end_ns]
        self.scans.append({"start": start_ns, "points": list(point_times)})
        self.latest_available_lidar_end = max(self.latest_available_lidar_end, end_ns)

    def _mark_wait(self, camera, lidar, imu):
        if lidar:
            self.waited_lidar.add(camera["id"])
        if imu:
            self.waited_imu.add(camera["id"])
        if lidar and imu:
            self.waited_both.add(camera["id"])

    def process_once(self, record_ns, eof=False):
        if not self.cameras:
            return False
        camera = self.cameras[0]
        tc = camera["time"]
        lacks_lidar = self.latest_available_lidar_end < tc
        lacks_imu = self.last_imu < tc
        if not self.scans or not self.imus:
            self._mark_wait(camera, lacks_lidar or not self.scans, lacks_imu or not self.imus)
            return False
        if tc <= self.last_epoch:
            self.cameras.popleft()
            self.stale_epoch_drop += 1
            return False
        scheduler_lidar_covers = bool(self.pending) or self.scans[0]["start"] <= tc
        scheduler_imu_covers = self.last_imu >= tc
        if not scheduler_lidar_covers or not scheduler_imu_covers:
            self._mark_wait(camera, not scheduler_lidar_covers, not scheduler_imu_covers)
            return False

        # FROZEN S0: re-slice pending tail at this tc (<= tc current, > tc future)
        current = []
        pending_new = []
        for pt in self.pending:
            if pt <= tc:
                current.append(pt)
            else:
                pending_new.append(pt)
        self.pending = pending_new
        while self.scans and self.scans[0]["start"] <= tc:
            scan = self.scans.popleft()
            for point_time in scan["points"]:
                if point_time <= tc:
                    current.append(point_time)
                else:
                    self.pending.append(point_time)
        if not current:
            self.cameras.popleft()
            self.empty_slice_drop += 1
            return False
        while self.imus and self.imus[0] <= tc:
            self.imus.popleft()

        self.cameras.popleft()
        if camera["id"] in self.emitted_ids:
            raise RuntimeError("camera identity emitted twice")
        self.emitted_ids.add(camera["id"])
        self.emit_without_lidar += int(self.latest_available_lidar_end < tc)
        self.emit_without_imu += int(self.last_imu < tc)
        self.coverage_lidar.append((self.latest_available_lidar_end - tc) * 1e-6)
        self.coverage_imu.append((self.last_imu - tc) * 1e-6)
        self.emission_delay.append((int(record_ns) - camera["arrival"]) * 1e-6)
        self.last_epoch = tc
        if eof:
            self.drained_eof += 1
        return True

    def report(self):
        legitimate_stale = self.buffer_overflow_drop + self.stale_epoch_drop
        unemittable_eof = len(self.cameras)
        strict_accounted = len(self.emitted_ids) + legitimate_stale + unemittable_eof
        explained_accounted = strict_accounted + self.empty_slice_drop
        return {
            "input_camera_messages": self.camera_input,
            "camera_epochs_emitted": len(self.emitted_ids),
            "camera_epochs_waited_for_lidar": len(self.waited_lidar),
            "camera_epochs_waited_for_imu": len(self.waited_imu),
            "camera_epochs_waited_for_both": len(self.waited_both),
            "emit_without_lidar_coverage": self.emit_without_lidar,
            "emit_without_imu_coverage": self.emit_without_imu,
            "latest_lidar_end_minus_tc_ms": distribution(self.coverage_lidar),
            "latest_imu_minus_tc_ms": distribution(self.coverage_imu),
            "arrival_to_emission_ms": distribution(self.emission_delay),
            "legitimate_stale_drop": legitimate_stale,
            "buffer_overflow_drop": self.buffer_overflow_drop,
            "stale_epoch_drop": self.stale_epoch_drop,
            "empty_slice_drop": self.empty_slice_drop,
            "unemittable_eof": unemittable_eof,
            "drained_eof": self.drained_eof,
            "duplicate_epochs": 0,
            "strict_three_term_mismatch": self.camera_input - strict_accounted,
            "unexplained_camera_accounting_mismatch": (
                self.camera_input - explained_accounted
            ),
        }


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bag", action="append", required=True)
    parser.add_argument("--lidar-topic", default="/livox/lidar")
    parser.add_argument("--imu-topic", default="/vn100/imu")
    parser.add_argument("--camera-topic", default="/d435i/infra1/image_rect_raw")
    parser.add_argument("--filter-rate", type=int, default=3)
    parser.add_argument("--blind", type=float, default=2.0)
    parser.add_argument("--max-range", type=float, default=60.0)
    parser.add_argument("--camera-time-offset", type=float, default=0.0)
    parser.add_argument("--camera-buffer-capacity", type=int, default=10)
    parser.add_argument("--json-out")
    return parser.parse_args(argv)


def audit(args):
    simulator = CausalEpochSimulator(args.camera_buffer_capacity)
    topics = [args.lidar_topic, args.imu_topic, args.camera_topic]
    offset_ns = int(round(2.0 * args.camera_time_offset * 1e9))
    last_record = 0
    for topic, message, record_time in iter_record_order(args.bag, topics):
        last_record = int(record_time.to_nsec())
        if topic == args.imu_topic:
            simulator.add_imu(stamp_ns(message), last_record)
        elif topic == args.camera_topic:
            simulator.add_camera(stamp_ns(message) + offset_ns, last_record)
        else:
            timing = production_scan_timing(
                message, args.filter_rate, args.blind, args.max_range
            )
            start = timing["scan_start_ns"]
            point_times = [
                start + value for value in production_point_offsets(
                    message, args.filter_rate, args.blind, args.max_range
                )
            ]
            simulator.add_lidar(start, timing["scan_end_ns"], last_record, point_times)
        # FROZEN ready-camera drain: after this record updates buffers,
        # repeatedly process queued cameras while they are causally ready
        # (emitted, empty-slice dropped, or stale-dropped all count as
        # progress; a waiting camera stops the drain).
        while True:
            before = len(simulator.cameras)
            simulator.process_once(last_record)
            if len(simulator.cameras) == before:
                break
    while simulator.process_once(last_record, eof=True):
        pass
    return simulator.report()


def production_point_offsets(message, filter_rate, blind, max_range):
    from audit_lidar_scan_end import selected_point_offsets_ns
    return selected_point_offsets_ns(message, filter_rate, blind, max_range)


def fmt(value):
    return "NA" if value is None else f"{value:.6f}"


def render(args, report, argv):
    lines = [
        f"git HEAD: {git_head(__file__)}",
        f"script path: {pathlib.Path(__file__).resolve()}",
        f"arguments: {shlex.join(argv)}",
    ]
    for key in (
        "input_camera_messages", "camera_epochs_emitted",
        "camera_epochs_waited_for_lidar", "camera_epochs_waited_for_imu",
        "camera_epochs_waited_for_both", "emit_without_lidar_coverage",
        "emit_without_imu_coverage", "legitimate_stale_drop",
        "buffer_overflow_drop", "stale_epoch_drop", "empty_slice_drop",
        "unemittable_eof", "drained_eof", "duplicate_epochs",
        "strict_three_term_mismatch", "unexplained_camera_accounting_mismatch",
    ):
        lines.append(f"{key}: {report[key]}")
    for key in (
        "latest_lidar_end_minus_tc_ms", "latest_imu_minus_tc_ms",
        "arrival_to_emission_ms",
    ):
        stats = report[key]
        lines.append(
            f"{key}: P10/P50/P90/P99/max={fmt(stats['p10'])}/"
            f"{fmt(stats['p50'])}/{fmt(stats['p90'])}/{fmt(stats['p99'])}/"
            f"{fmt(stats['max'])}"
        )
    return "\n".join(lines) + "\n"


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        report = audit(args)
        sys.stdout.write(render(args, report, raw_argv))
        if args.json_out:
            pathlib.Path(args.json_out).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
            )
        return 0
    except (OSError, ValueError, RuntimeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
