#!/usr/bin/env python3
"""Audit exact production Mid70 scan ends and per-source header monotonicity."""
import argparse
import json
import pathlib
import shlex
import sys

from audit_common import distribution, git_head, iter_record_order


def stamp_ns(message):
    return int(message.header.stamp.to_nsec())


def selected_point_offsets_ns(message, filter_rate=3, blind=2.0, max_range=60.0):
    """Mirror ROSWrapper::livox_handler point acceptance using integer times."""
    if int(message.point_num) < 10:
        return []
    selected = []
    blind2 = blind * blind
    max_range2 = max_range * max_range
    limit = min(int(message.point_num), len(message.points))
    for point_index in range(0, limit, filter_rate):
        point = message.points[point_index]
        tag = int(point.tag) & 0x30
        distance2 = point.x * point.x + point.y * point.y + point.z * point.z
        if tag in (0x00, 0x10) and blind2 < distance2 < max_range2:
            selected.append(int(point.offset_time))
    return selected


def production_scan_timing(message, filter_rate=3, blind=2.0, max_range=60.0):
    offsets = selected_point_offsets_ns(message, filter_rate, blind, max_range)
    start = stamp_ns(message)
    duration = max(offsets) if offsets else 0
    return {
        "scan_start_ns": start,
        "scan_duration_ns": duration,
        "scan_end_ns": start + duration,
        "selected_point_count": len(offsets),
        "production_last_offset_ns": offsets[-1] if offsets else 0,
        "last_offset_matches_max": not offsets or offsets[-1] == duration,
    }


def monotonicity(timestamps):
    equal = 0
    negative = []
    for previous, current in zip(timestamps, timestamps[1:]):
        if current == previous:
            equal += 1
        elif current < previous:
            negative.append(previous - current)
    return {
        "message_count": len(timestamps),
        "equal_stamp_count": equal,
        "negative_step_count": len(negative),
        "negative_step_ns": distribution(negative, percentiles=(50, 90, 99)),
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
    parser.add_argument("--json-out")
    return parser.parse_args(argv)


def audit(args):
    topic_role = {
        args.lidar_topic: "LiDAR",
        args.imu_topic: "IMU",
        args.camera_topic: "Camera",
    }
    headers = {role: [] for role in topic_role.values()}
    durations_ms = []
    record_header_ms = []
    record_end_ms = []
    record_before_start = 0
    record_before_end = 0
    empty_selected = 0
    last_offset_not_max = 0
    for topic, message, record_time in iter_record_order(args.bag, list(topic_role)):
        header = stamp_ns(message)
        headers[topic_role[topic]].append(header)
        if topic != args.lidar_topic:
            continue
        timing = production_scan_timing(
            message, args.filter_rate, args.blind, args.max_range
        )
        if not timing["selected_point_count"]:
            empty_selected += 1
        last_offset_not_max += int(not timing["last_offset_matches_max"])
        record = int(record_time.to_nsec())
        durations_ms.append(timing["scan_duration_ns"] * 1e-6)
        record_header_ms.append((record - header) * 1e-6)
        record_end_ms.append((record - timing["scan_end_ns"]) * 1e-6)
        record_before_start += int(record < header)
        record_before_end += int(record < timing["scan_end_ns"])
    return {
        "scan_duration_ms": distribution(durations_ms, (1, 10, 50, 90, 99)),
        "record_header_ms": distribution(record_header_ms, (1, 10, 50, 90, 99)),
        "record_scan_end_ms": distribution(record_end_ms, (1, 10, 50, 90, 99)),
        "record_before_scan_start": record_before_start,
        "record_before_scan_end": record_before_end,
        "empty_selected_scans": empty_selected,
        "production_last_offset_not_max": last_offset_not_max,
        "monotonicity": {role: monotonicity(values) for role, values in headers.items()},
    }


def fmt(value):
    return "NA" if value is None else f"{value:.6f}"


def render(args, report, argv):
    lines = [
        f"git HEAD: {git_head(__file__)}",
        f"script path: {pathlib.Path(__file__).resolve()}",
        f"arguments: {shlex.join(argv)}",
    ]
    for key in ("scan_duration_ms", "record_header_ms", "record_scan_end_ms"):
        stats = report[key]
        lines.append(
            f"{key}: n={stats['count']} P01={fmt(stats['p01'])} "
            f"P10={fmt(stats['p10'])} P50={fmt(stats['p50'])} "
            f"P90={fmt(stats['p90'])} P99={fmt(stats['p99'])} "
            f"max={fmt(stats['max'])}"
        )
    lines.extend([
        f"record < scan_start: {report['record_before_scan_start']}",
        f"record < scan_end: {report['record_before_scan_end']}",
        f"empty selected scans: {report['empty_selected_scans']}",
        f"production last accepted offset != max: {report['production_last_offset_not_max']}",
    ])
    for role, stats in report["monotonicity"].items():
        negative = stats["negative_step_ns"]
        lines.append(
            f"{role} monotonicity: n={stats['message_count']} "
            f"equal={stats['equal_stamp_count']} negative={stats['negative_step_count']} "
            f"negative_ns_P50/P90/P99/max={fmt(negative['p50'])}/"
            f"{fmt(negative['p90'])}/{fmt(negative['p99'])}/{fmt(negative['max'])}"
        )
    return "\n".join(lines) + "\n"


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        if args.filter_rate <= 0:
            raise ValueError("filter rate must be positive")
        report = audit(args)
        sys.stdout.write(render(args, report, raw_argv))
        if args.json_out:
            pathlib.Path(args.json_out).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
            )
        return 0
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
