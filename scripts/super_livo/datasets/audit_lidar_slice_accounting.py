#!/usr/bin/env python3
"""Identity-based audit of the production camera-epoch LiDAR slicing state machine."""
import argparse
from collections import deque
import json
import math
import pathlib
import shlex
import sys

from audit_common import distribution, git_head, header_seconds, iter_record_order


def to_ns(seconds):
    """Float seconds -> integer nanoseconds (exact classification in int ns)."""
    if isinstance(seconds, int):
        return seconds
    return int(round(float(seconds) * 1e9))


class SliceSimulator:
    """Mirror ROSWrapper::sync_camera_epoch and sliceLidarAt without estimation."""

    def __init__(self, camera_capacity=10):
        self.camera_capacity = camera_capacity
        self.cameras = deque()
        self.scans = deque()
        self.imus = deque()
        self.last_imu = -math.inf
        self.pending = []
        self.last_epoch = -math.inf
        self.status = bytearray()
        self.scan_index = 0
        self.emitted_attempts = 0
        self.duplicate_emissions = 0
        self.emitted_early = 0
        self.boundary_equality_count = 0
        self.retained_future_events = 0
        self.slice_sizes = []
        self.pending_sizes = []
        self.empty_slices = 0
        self.camera_dropped = 0
        self.stale_camera_dropped = 0
        self.epochs = 0
        self.scan_epochs = {}

    def add_imu(self, timestamp):
        self.imus.append(to_ns(timestamp))
        self.last_imu = to_ns(timestamp)

    def add_camera(self, timestamp):
        while len(self.cameras) >= self.camera_capacity:
            self.cameras.popleft()
            self.camera_dropped += 1
        self.cameras.append(to_ns(timestamp))

    def add_scan(self, start_time, indexed_offsets):
        points = []
        for original_index, offset in indexed_offsets:
            identity = len(self.status)
            self.status.append(0)
            points.append({
                "identity": identity,
                "stable_identity": (self.scan_index, int(original_index)),
                "raw_scan_id": self.scan_index,
                "time_ns": to_ns(start_time) + to_ns(offset),
            })
        self.scans.append({"start_ns": to_ns(start_time), "points": points})
        self.scan_index += 1

    def emit(self, point, epoch_ns):
        identity = point["identity"]
        self.emitted_attempts += 1
        if self.status[identity]:
            self.duplicate_emissions += 1
        else:
            self.status[identity] = 1
        if point["time_ns"] > epoch_ns:
            self.emitted_early += 1
        if point["time_ns"] == epoch_ns:
            self.boundary_equality_count += 1
        self.scan_epochs.setdefault(point["raw_scan_id"], set()).add(epoch_ns)

    def process_once(self):
        # F4: pending LiDAR is valid already-received data; entry needs
        # camera + (pending OR scan) + IMU.
        if not self.cameras or (not self.scans and not self.pending) or not self.imus:
            return False
        epoch_ns = self.cameras[0]
        if epoch_ns <= self.last_epoch:
            self.cameras.popleft()
            self.stale_camera_dropped += 1
            return False
        lidar_covers = bool(self.pending) or self.scans[0]["start_ns"] <= epoch_ns
        if not lidar_covers or self.last_imu < epoch_ns:
            return False

        current = []
        # FROZEN S0 semantics: pending tail is re-sliced at this epoch.
        # point_time_ns <= tc_ns -> current; > tc_ns -> future/pending.
        pending_new = []
        for point in self.pending:
            if point["time_ns"] <= epoch_ns:
                current.append(point)
            else:
                pending_new.append(point)
        self.pending = pending_new
        while self.scans and self.scans[0]["start_ns"] <= epoch_ns:
            scan = self.scans.popleft()
            for point in scan["points"]:
                if point["time_ns"] <= epoch_ns:
                    current.append(point)
                else:
                    self.pending.append(point)
                    self.retained_future_events += 1

        for point in current:
            self.emit(point, epoch_ns)
        if not current:
            self.cameras.popleft()
            self.empty_slices += 1
            return False

        while self.imus and self.imus[0] <= epoch_ns:
            self.imus.popleft()
        self.last_epoch = epoch_ns
        self.cameras.popleft()
        self.epochs += 1
        self.slice_sizes.append(len(current))
        self.pending_sizes.append(len(self.pending))
        return True

    def final_report(self):
        final_marker = bytearray(len(self.status))
        final_points = list(self.pending)
        for scan in self.scans:
            final_points.extend(scan["points"])
        final_duplicate = 0
        retained_emitted_overlap = 0
        for point in final_points:
            identity = point["identity"]
            if final_marker[identity]:
                final_duplicate += 1
            final_marker[identity] = 1
            if self.status[identity]:
                retained_emitted_overlap += 1
        lost = sum(
            1
            for identity, emitted in enumerate(self.status)
            if not emitted and not final_marker[identity]
        )
        input_count = len(self.status)
        final_retained = len(final_points)
        conservation = (
            input_count == self.emitted_attempts + final_retained
            and self.duplicate_emissions == 0
            and final_duplicate == 0
            and retained_emitted_overlap == 0
            and lost == 0
        )
        return {
            "stable_identity": "(raw_scan_id, original_point_index)",
            "raw_scan_count": self.scan_index,
            "distinct_raw_scan_ids_emitted": len(self.scan_epochs),
            "scans_emitted_across_multiple_epochs": sum(
                1 for epochs in self.scan_epochs.values() if len(epochs) > 1
            ),
            "input_valid_selected_points": input_count,
            "emitted_points": self.emitted_attempts,
            "unique_emitted_points": int(sum(1 for value in self.status if value)),
            "final_retained_points": final_retained,
            "retained_future_events": self.retained_future_events,
            "duplicate_emission_count": self.duplicate_emissions,
            "final_retained_duplicate_count": final_duplicate,
            "retained_emitted_overlap_count": retained_emitted_overlap,
            "lost_point_count": lost,
            "emitted_before_physical_time": self.emitted_early,
            "wrong_side_count": self.emitted_early,
            "boundary_equality_count": self.boundary_equality_count,
            "camera_epoch_count": self.epochs,
            "camera_buffer_dropped": self.camera_dropped,
            "stale_camera_dropped": self.stale_camera_dropped,
            "empty_slice_count": self.empty_slices,
            "slice_points": distribution(self.slice_sizes, percentiles=(10, 50, 90, 95, 99)),
            "pending_after_epoch": distribution(
                self.pending_sizes, percentiles=(10, 50, 90, 95, 99)
            ),
            "conservation_pass": conservation,
        }


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Camera-epoch LiDAR slice audit")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--bag", action="append")
    source.add_argument("--events-json")
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


def selected_livox_offsets(message, args):
    if int(message.point_num) < 10:
        return []
    selected = []
    limit = min(int(message.point_num), len(message.points))
    blind2 = args.blind * args.blind
    max_range2 = args.max_range * args.max_range
    for point_index in range(0, limit, args.filter_rate):
        point = message.points[point_index]
        tag = int(point.tag) & 0x30
        distance2 = point.x * point.x + point.y * point.y + point.z * point.z
        if tag in (0x00, 0x10) and blind2 < distance2 < max_range2:
            selected.append((point_index, float(point.offset_time) * 1e-9))
    return selected


def run_json_events(args, simulator):
    events = json.loads(pathlib.Path(args.events_json).read_text(encoding="utf-8"))
    for event in sorted(events, key=lambda item: item["record"]):
        kind = event["kind"]
        if kind == "imu":
            simulator.add_imu(event["header"])
        elif kind == "camera":
            # F5: HandleImage applies offset exactly once; t_c = frame.timestamp.
            event_time_ns = (
                event["header_ns"] if "header_ns" in event else to_ns(event["header"])
            )
            simulator.add_camera(event_time_ns + to_ns(args.camera_time_offset))
        elif kind == "lidar":
            offsets = event.get("offsets_ns") if "offsets_ns" in event else event.get("offsets", [])
            start_time = event["header_ns"] if "header_ns" in event else event["header"]
            simulator.add_scan(start_time, list(enumerate(offsets)))
        else:
            raise ValueError(f"unknown event kind: {kind}")
        simulator.process_once()


def run_bags(args, simulator):
    topics = [args.lidar_topic, args.imu_topic, args.camera_topic]
    for topic, message, _ in iter_record_order(args.bag, topics):
        if topic == args.imu_topic:
            simulator.add_imu(header_seconds(message))
        elif topic == args.camera_topic:
            # F5: /camera/time_offset applied exactly once at ingestion.
            simulator.add_camera(
                to_ns(header_seconds(message)) + to_ns(args.camera_time_offset)
            )
        elif topic == args.lidar_topic:
            simulator.add_scan(
                header_seconds(message), selected_livox_offsets(message, args)
            )
        simulator.process_once()


def fmt(value):
    return "NA" if value is None else f"{value:.3f}"


def render(args, report, argv):
    script = pathlib.Path(__file__).resolve()
    lines = [
        f"git HEAD: {git_head(script)}",
        f"script path: {script}",
        f"arguments: {shlex.join(argv)}",
        f"stable identity: {report['stable_identity']}",
        f"raw scans: {report['raw_scan_count']}",
        f"distinct raw scan ids emitted: {report['distinct_raw_scan_ids_emitted']}",
        f"scans emitted across multiple epochs: {report['scans_emitted_across_multiple_epochs']}",
        f"input valid selected LiDAR points: {report['input_valid_selected_points']}",
        f"emitted points: {report['emitted_points']}",
        f"final retained: {report['final_retained_points']}",
        f"duplicate emission count: {report['duplicate_emission_count']}",
        f"lost point count: {report['lost_point_count']}",
        f"emitted before physical time: {report['emitted_before_physical_time']}",
        f"wrong-side count: {report['wrong_side_count']}",
        f"boundary-equality count: {report['boundary_equality_count']}",
        f"retained-at-final semantics: pending slice plus unconsumed scan buffer",
        f"camera epochs: {report['camera_epoch_count']}",
        f"empty slices: {report['empty_slice_count']}",
        f"camera buffer drops: {report['camera_buffer_dropped']}",
        (
            "slice points P10/P50/P90/P95/P99: "
            f"{fmt(report['slice_points']['p10'])}/"
            f"{fmt(report['slice_points']['p50'])}/"
            f"{fmt(report['slice_points']['p90'])}/"
            f"{fmt(report['slice_points']['p95'])}/"
            f"{fmt(report['slice_points']['p99'])}"
        ),
        f"conservation: {'PASS' if report['conservation_pass'] else 'FAIL'}",
    ]
    return "\n".join(lines) + "\n"


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        if args.filter_rate <= 0 or args.camera_buffer_capacity <= 0:
            raise ValueError("filter rate and camera capacity must be positive")
        simulator = SliceSimulator(args.camera_buffer_capacity)
        if args.events_json:
            run_json_events(args, simulator)
        else:
            run_bags(args, simulator)
        # Match OfflineReader::drain: stop on the first call without a new epoch.
        while simulator.process_once():
            pass
        report = simulator.final_report()
        output = render(args, report, raw_argv)
        sys.stdout.write(output)
        if args.json_out:
            pathlib.Path(args.json_out).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
