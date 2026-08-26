#!/usr/bin/env python3
"""FrozenS0ReferenceOracle — exact integer-ns camera-epoch LiDAR slicing audit.

Frozen P0R2 contract (owner-frozen; this oracle defines PASS):
  - exact physical timestamps: real-bag headers via to_nsec, Livox point
    offsets as integer ns directly (float seconds are REPORTING only)
  - camera offset applied exactly once
  - true spanning LiDAR coverage: pending covers tc iff pending max >= tc;
    a buffered scan covers tc iff start_ns <= tc_ns <= end_ns; future or
    finished scans do NOT cover
  - IMU must reach tc
  - repeated pending-tail re-slicing at every epoch
  - drain all causally-ready cameras after EVERY delivered event
  - no future-data lookup
  - EOF terminal point and camera accounting
  - exact-ns dump (s0_audit_exact.json) for production-vs-oracle comparison
"""
import argparse
from collections import deque
import json
import math
import pathlib
import shlex
import sys

from audit_common import distribution, git_head, header_nsec, iter_record_order


def to_ns(seconds):
    """int passes through; float seconds -> integer ns (reporting ok)."""
    if isinstance(seconds, int):
        return int(seconds)
    return int(round(float(seconds) * 1e9))


class FrozenS0ReferenceOracle:
    """Mirror ROSWrapper::sync_camera_epoch + OfflineReader drain under the
    frozen contract (exact ns; not a mirror of production behavior)."""

    def __init__(self, camera_capacity=10):
        self.camera_capacity = camera_capacity
        self.cameras = deque()
        self.scans = deque()
        self.imus = deque()
        self.last_imu = None
        self.pending = []
        self.pending_max = None
        self.last_epoch = None
        self.status = set()  # emitted ids (sparse (scan_id<<32)|idx)
        self.total_input = 0
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
        # exact-ns comparison structures
        self.emitted_epoch_ns = []          # (id, tc_ns)
        self.boundary_assign = []           # (id, tc_ns) with time_ns == tc_ns
        self.epoch_tcs_ns = []
        self.scan_epochs = {}

    # ---- buffer ingestion (all integer ns) ----
    def add_imu(self, ns):
        ns = to_ns(ns)
        self.imus.append(ns)
        self.last_imu = ns

    def add_camera(self, ns):
        ns = to_ns(ns)
        while len(self.cameras) >= self.camera_capacity:
            self.cameras.popleft()
            self.camera_dropped += 1
        self.cameras.append(ns)

    def add_scan(self, start_ns, indexed_offsets_ns):
        points = []
        start_ns = to_ns(start_ns)
        end_ns = start_ns
        for original_index, offset_ns in indexed_offsets_ns:
            offset_ns = to_ns(offset_ns)
            # production id convention: (raw_scan_id << 32) | raw point index
            identity = (self.scan_index << 32) | int(original_index)
            time_ns = start_ns + offset_ns
            points.append({
                "identity": identity,
                "stable_identity": (self.scan_index, int(original_index)),
                "raw_scan_id": self.scan_index,
                "time_ns": time_ns,
            })
            if time_ns > end_ns:
                end_ns = time_ns
        self.scans.append({"start_ns": start_ns, "end_ns": end_ns,
                           "points": points})
        self.total_input += len(points)
        self.scan_index += 1

    # ---- P0R2-B true spanning coverage ----
    def covered(self, tc_ns):
        if self.pending_max is not None and self.pending_max >= tc_ns:
            return True
        for scan in self.scans:
            if scan["start_ns"] <= tc_ns:
                if tc_ns <= scan["end_ns"]:
                    return True
            else:
                break
        return False

    def emit(self, point, tc_ns):
        identity = point["identity"]
        self.emitted_attempts += 1
        if identity in self.status:
            self.duplicate_emissions += 1
        else:
            self.status.add(identity)
        if point["time_ns"] > tc_ns:
            self.emitted_early += 1
        if point["time_ns"] == tc_ns:
            self.boundary_equality_count += 1
            self.boundary_assign.append((identity, tc_ns))
        self.emitted_epoch_ns.append((identity, tc_ns))
        self.scan_epochs.setdefault(point["raw_scan_id"], set()).add(tc_ns)

    def _append_pending(self, point):
        self.pending.append(point)
        if self.pending_max is None or point["time_ns"] > self.pending_max:
            self.pending_max = point["time_ns"]

    def process_once(self):
        """One camera-epoch attempt; True if an epoch was produced."""
        if not self.cameras or (not self.scans and not self.pending) or \
           not self.imus:
            return False
        tc_ns = self.cameras[0]
        if self.last_epoch is not None and tc_ns <= self.last_epoch:
            self.cameras.popleft()
            self.stale_camera_dropped += 1
            return False
        if not self.covered(tc_ns):
            return False
        if self.last_imu is None or self.last_imu < tc_ns:
            return False

        current = []
        pending_new = []
        for point in self.pending:
            if point["time_ns"] <= tc_ns:
                current.append(point)
            else:
                pending_new.append(point)
        self.pending = pending_new
        self.pending_max = None
        for point in self.pending:
            if self.pending_max is None or point["time_ns"] > self.pending_max:
                self.pending_max = point["time_ns"]
        while self.scans and self.scans[0]["start_ns"] <= tc_ns:
            scan = self.scans.popleft()
            for point in scan["points"]:
                if point["time_ns"] <= tc_ns:
                    current.append(point)
                else:
                    self._append_pending(point)
                    self.retained_future_events += 1

        for point in current:
            self.emit(point, tc_ns)
        if not current:
            self.cameras.popleft()
            self.empty_slices += 1
            return False

        while self.imus and self.imus[0] <= tc_ns:
            self.imus.popleft()
        self.last_epoch = tc_ns
        self.cameras.popleft()
        self.epochs += 1
        self.epoch_tcs_ns.append(tc_ns)
        self.slice_sizes.append(len(current))
        self.pending_sizes.append(len(self.pending))
        return True

    def drain(self):
        """Drain all causally-ready cameras (stop on first no-progress)."""
        while self.process_once():
            pass

    # ---- EOF terminal accounting ----
    def final_report(self):
        final_ids = {point["identity"] for point in self.pending}
        for scan in self.scans:
            final_ids.update(point["identity"] for point in scan["points"])
        final_points = list(self.pending)
        for scan in self.scans:
            final_points.extend(scan["points"])
        final_duplicate = len(final_points) - len(final_ids)
        retained_emitted_overlap = len(final_ids & self.status)
        # lost = |input| - |emitted union final| (set algebra; emitted and
        # final are disjoint for every conserved point)
        lost = (self.total_input - len(self.status) - len(final_ids) +
                retained_emitted_overlap)
        input_count = self.total_input
        final_retained = len(final_points)
        conservation = (
            input_count == self.emitted_attempts + final_retained
            and self.duplicate_emissions == 0
            and final_duplicate == 0
            and retained_emitted_overlap == 0
            and lost == 0
        )
        camera_input = self.epochs + self.stale_camera_dropped + \
            self.empty_slices + self.camera_dropped + len(self.cameras)
        return {
            "stable_identity": "(raw_scan_id, original_point_index)",
            "raw_scan_count": self.scan_index,
            "distinct_raw_scan_ids_emitted": len(self.scan_epochs),
            "scans_emitted_across_multiple_epochs": sum(
                1 for epochs in self.scan_epochs.values() if len(epochs) > 1
            ),
            "input_valid_selected_points": input_count,
            "emitted_points": self.emitted_attempts,
            "unique_emitted_points": len(self.status),
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
            "camera_input": camera_input,
            "camera_buffer_dropped": self.camera_dropped,
            "stale_camera_dropped": self.stale_camera_dropped,
            "empty_slice_count": self.empty_slices,
            "eof_unemitted_cameras": len(self.cameras),
            "camera_unclassified": camera_input - self.epochs -
                self.stale_camera_dropped - self.empty_slices -
                self.camera_dropped - len(self.cameras),
            "slice_points": distribution(self.slice_sizes, percentiles=(10, 50, 90, 95, 99)),
            "pending_after_epoch": distribution(
                self.pending_sizes, percentiles=(10, 50, 90, 95, 99)
            ),
            "conservation_pass": conservation,
        }


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="FrozenS0ReferenceOracle camera-epoch LiDAR slice audit")
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
    parser.add_argument("--exact-out")
    return parser.parse_args(argv)


def selected_livox_offsets_ns(message, args):
    """Exact integer-ns offsets; non-CustomMsg lids fail closed (P0R2-A).

    Returns None for point_num < 10 messages (production skips them
    entirely without consuming a raw-scan id); the caller must NOT
    register a scan for None.
    """
    if not hasattr(message, "point_num"):
        raise ValueError(
            "lidar message is not livox CustomMsg; exact point-time field "
            "unsupported -> fail closed")
    if int(message.point_num) < 10:
        return None
    selected = []
    limit = min(int(message.point_num), len(message.points))
    blind2 = args.blind * args.blind
    max_range2 = args.max_range * args.max_range
    for point_index in range(0, limit, args.filter_rate):
        point = message.points[point_index]
        tag = int(point.tag) & 0x30
        distance2 = point.x * point.x + point.y * point.y + point.z * point.z
        if tag in (0x00, 0x10) and blind2 < distance2 < max_range2:
            # P0R2-A: offset_time is integer ns in the message; no float
            # round-trip for hard classification
            selected.append((point_index, int(point.offset_time)))
    return selected


def run_json_events(args, oracle):
    events = json.loads(pathlib.Path(args.events_json).read_text(encoding="utf-8"))
    for event in sorted(events, key=lambda item: item["record"]):
        kind = event["kind"]
        if kind == "imu":
            ns = event["header_ns"] if "header_ns" in event else to_ns(event["header"])
            oracle.add_imu(ns)
        elif kind == "camera":
            ns = event["header_ns"] if "header_ns" in event else to_ns(event["header"])
            oracle.add_camera(ns + to_ns(args.camera_time_offset))
        elif kind == "lidar":
            offsets = event.get("offsets_ns") if "offsets_ns" in event \
                else [to_ns(o) for o in event.get("offsets", [])]
            start = event["header_ns"] if "header_ns" in event else to_ns(event["header"])
            oracle.add_scan(start, list(enumerate(offsets)))
        else:
            raise ValueError(f"unknown event kind: {kind}")
        oracle.drain()  # P0R2-C: drain after every delivered event


def run_bags(args, oracle):
    topics = [args.lidar_topic, args.imu_topic, args.camera_topic]
    for topic, message, _ in iter_record_order(args.bag, topics):
        if topic == args.imu_topic:
            oracle.add_imu(header_nsec(message))
        elif topic == args.camera_topic:
            # P0R2-A/F5: camera offset applied exactly once, in ns
            oracle.add_camera(
                header_nsec(message) + to_ns(args.camera_time_offset))
        elif topic == args.lidar_topic:
            offsets = selected_livox_offsets_ns(message, args)
            if offsets is None:
                continue  # production skips point_num < 10 messages
            oracle.add_scan(header_nsec(message), offsets)
        else:
            continue
        oracle.drain()


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
        f"camera epochs: {report['camera_epoch_count']}",
        f"camera input/evicted/stale/empty/eof-unemitted: "
        f"{report['camera_input']}/{report['camera_buffer_dropped']}/"
        f"{report['stale_camera_dropped']}/{report['empty_slice_count']}/"
        f"{report['eof_unemitted_cameras']}",
        f"camera unclassified: {report['camera_unclassified']}",
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


def write_exact_dump(oracle, path):
    emitted = sorted(oracle.emitted_epoch_ns)
    boundary = sorted(oracle.boundary_assign)
    tcs = sorted(oracle.epoch_tcs_ns)
    final_ids = {point["identity"] for point in oracle.pending}
    for scan in oracle.scans:
        final_ids.update(point["identity"] for point in scan["points"])
    retained = sorted(final_ids)
    payload = {
        "emitted": emitted,
        "boundary": boundary,
        "epoch_tcs": tcs,
        "final_retained": retained,
    }
    pathlib.Path(path).write_text(
        json.dumps(payload) + "\n", encoding="utf-8")


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        if args.filter_rate <= 0 or args.camera_buffer_capacity <= 0:
            raise ValueError("filter rate and camera capacity must be positive")
        oracle = FrozenS0ReferenceOracle(args.camera_buffer_capacity)
        if args.events_json:
            run_json_events(args, oracle)
        else:
            run_bags(args, oracle)
        oracle.drain()
        report = oracle.final_report()
        output = render(args, report, raw_argv)
        sys.stdout.write(output)
        if args.json_out:
            pathlib.Path(args.json_out).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n",
                encoding="utf-8")
        if args.exact_out:
            write_exact_dump(oracle, args.exact_out)
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
