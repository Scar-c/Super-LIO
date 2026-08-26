#!/usr/bin/env python3
"""Audit record/header deltas and adjacent header inversions in record order."""
import argparse
import json
import pathlib
import shlex
import sys

from audit_common import (
    distribution,
    git_head,
    header_seconds,
    iter_record_order,
    linear_slope,
)


PAIR_KEYS = {
    frozenset(("camera", "lidar")): "camera_lidar",
    frozenset(("camera", "imu")): "camera_imu",
    frozenset(("lidar", "imu")): "lidar_imu",
}


def parse_stream(value):
    if "=" not in value:
        raise argparse.ArgumentTypeError("--stream must be ROLE=/topic")
    role, topic = value.split("=", 1)
    if not role or not topic.startswith("/"):
        raise argparse.ArgumentTypeError("--stream must be ROLE=/topic")
    return role, topic


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Record/header and header inversion audit"
    )
    parser.add_argument("--bag", action="append", required=True)
    parser.add_argument("--stream", action="append", type=parse_stream, required=True)
    parser.add_argument("--json-out")
    return parser.parse_args(argv)


def audit(args):
    topic_to_role = {}
    for role, topic in args.stream:
        if topic in topic_to_role:
            raise ValueError(f"duplicate selected topic: {topic}")
        topic_to_role[topic] = role
    samples = {
        role: {"header": [], "delta": []}
        for role, _ in args.stream
    }
    inversion_values = {
        "all_selected_messages": [],
        "camera_lidar": [],
        "camera_imu": [],
        "lidar_imu": [],
    }
    previous = None
    for topic, message, record_time in iter_record_order(
            args.bag, list(topic_to_role)):
        role = topic_to_role[topic]
        header = header_seconds(message)
        record = float(record_time.to_sec())
        samples[role]["header"].append(header)
        samples[role]["delta"].append(record - header)
        if previous is not None and header < previous["header"]:
            magnitude = previous["header"] - header
            inversion_values["all_selected_messages"].append(magnitude)
            pair_key = PAIR_KEYS.get(
                frozenset((previous["role"].lower(), role.lower()))
            )
            if pair_key:
                inversion_values[pair_key].append(magnitude)
        previous = {"header": header, "role": role}

    streams = {}
    for role, values in samples.items():
        stats = distribution(values["delta"])
        stats["drift_slope"] = linear_slope(values["header"], values["delta"])
        streams[role] = stats
    cross_stream = {}
    roles = [role for role, _ in args.stream]
    for left_index, left in enumerate(roles):
        for right in roles[left_index + 1:]:
            cross_stream[f"{left}-{right}"] = {
                "mean_delta_difference": streams[left]["mean"] - streams[right]["mean"],
                "median_delta_difference": streams[left]["p50"] - streams[right]["p50"],
            }
    inversions = {
        key: distribution(values, percentiles=(50, 90, 99))
        for key, values in inversion_values.items()
    }
    return {
        "streams": streams,
        "cross_stream_differences": cross_stream,
        "header_inversions": inversions,
    }


def fmt(value):
    return "NA" if value is None else f"{value:.9f}"


def render(args, report, argv):
    script = pathlib.Path(__file__).resolve()
    lines = [
        f"git HEAD: {git_head(script)}",
        f"script path: {script}",
        f"arguments: {shlex.join(argv)}",
    ]
    for role, _ in args.stream:
        stats = report["streams"][role]
        lines.append(
            f"{role} record-header: n={stats['count']} "
            f"min={fmt(stats['min'])} P10={fmt(stats['p10'])} "
            f"P50={fmt(stats['p50'])} P90={fmt(stats['p90'])} "
            f"P99={fmt(stats['p99'])} max={fmt(stats['max'])} "
            f"mean={fmt(stats['mean'])} std={fmt(stats['std'])} "
            f"drift_slope={fmt(stats['drift_slope'])}"
        )
    lines.append(
        "cross-stream differences: "
        + json.dumps(report["cross_stream_differences"], sort_keys=True)
    )
    lines.append("header inversions:")
    for key, stats in report["header_inversions"].items():
        lines.append(
            f"  {key}: n={stats['count']} P50={fmt(stats['p50'])} "
            f"P90={fmt(stats['p90'])} P99={fmt(stats['p99'])} "
            f"max={fmt(stats['max'])}"
        )
    return "\n".join(lines) + "\n"


def main(argv=None):
    raw_argv = sys.argv[1:] if argv is None else list(argv)
    try:
        args = parse_args(raw_argv)
        report = audit(args)
        output = render(args, report, raw_argv)
        sys.stdout.write(output)
        if args.json_out:
            pathlib.Path(args.json_out).write_text(
                json.dumps(report, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
        return 0
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
