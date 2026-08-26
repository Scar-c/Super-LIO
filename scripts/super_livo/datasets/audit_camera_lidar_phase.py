#!/usr/bin/env python3
"""Report camera-to-nearest-LiDAR signed phase without claiming an offset."""
import argparse
import json
import pathlib
import shlex
import sys

import numpy as np

from audit_common import (
    distribution,
    git_head,
    header_seconds,
    iter_record_order,
    linear_slope,
)


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Camera/LiDAR header phase audit")
    parser.add_argument("--bag", action="append", required=True)
    parser.add_argument("--camera-topic", required=True)
    parser.add_argument("--lidar-topic", required=True)
    parser.add_argument("--json-out")
    return parser.parse_args(argv)


def nearest_signed(camera_times, lidar_times):
    lidar = np.asarray(lidar_times, dtype=float)
    signed = []
    for camera in camera_times:
        index = int(np.searchsorted(lidar, camera))
        candidates = []
        if index:
            candidates.append(index - 1)
        if index < lidar.size:
            candidates.append(index)
        nearest = min(candidates, key=lambda item: (abs(camera - lidar[item]), item))
        signed.append(float(camera - lidar[nearest]))
    return signed


def histogram_milliseconds(values):
    bins = {}
    for value in values:
        key = str(int(np.floor(value * 1000.0)))
        bins[key] = bins.get(key, 0) + 1
    return bins


def audit(args):
    camera_times, lidar_times = [], []
    for topic, message, _ in iter_record_order(
            args.bag, [args.camera_topic, args.lidar_topic]):
        timestamp = header_seconds(message)
        if topic == args.camera_topic:
            camera_times.append(timestamp)
        elif topic == args.lidar_topic:
            lidar_times.append(timestamp)
    camera_times.sort()
    lidar_times.sort()
    if not camera_times or not lidar_times:
        raise ValueError(
            f"missing selected headers: camera={len(camera_times)} lidar={len(lidar_times)}"
        )
    signed = nearest_signed(camera_times, lidar_times)
    absolute = np.abs(np.asarray(signed))
    lidar_interarrival = np.diff(lidar_times)
    camera_interarrival = np.diff(camera_times)
    if not lidar_interarrival.size:
        raise ValueError("at least two LiDAR headers are required for phase modulo period")
    lidar_period = float(np.median(lidar_interarrival))
    phase_modulo = np.mod(np.asarray(camera_times) - lidar_times[0], lidar_period)
    return {
        "camera_count": len(camera_times),
        "lidar_count": len(lidar_times),
        "signed_dt": distribution(signed),
        "signed_histogram_ms": histogram_milliseconds(signed),
        "absolute_dt": distribution(absolute),
        "phase_drift_slope": linear_slope(camera_times, signed),
        "lidar_period_median": lidar_period,
        "phase_modulo_lidar_period": distribution(phase_modulo),
        "camera_interarrival": distribution(camera_interarrival),
        "lidar_interarrival": distribution(lidar_interarrival),
    }


def fmt(value):
    return "NA" if value is None else f"{value:.9f}"


def summary(label, stats):
    return (
        f"{label}: n={stats['count']} min={fmt(stats['min'])} "
        f"P10={fmt(stats['p10'])} P50={fmt(stats['p50'])} "
        f"P90={fmt(stats['p90'])} P99={fmt(stats['p99'])} "
        f"max={fmt(stats['max'])} mean={fmt(stats['mean'])} "
        f"std={fmt(stats['std'])}"
    )


def render(args, report, argv):
    script = pathlib.Path(__file__).resolve()
    lines = [
        f"git HEAD: {git_head(script)}",
        f"script path: {script}",
        f"arguments: {shlex.join(argv)}",
        f"camera count: {report['camera_count']}",
        f"LiDAR count: {report['lidar_count']}",
        summary("signed dt", report["signed_dt"]),
        "signed histogram (1 ms bins): "
        + json.dumps(report["signed_histogram_ms"], sort_keys=True),
        summary("absolute |dt|", report["absolute_dt"]),
        f"phase vs time: linear drift slope={report['phase_drift_slope']:.12g}",
        (
            "phase modulo LiDAR period: "
            f"period={report['lidar_period_median']:.9f} "
            f"P50={fmt(report['phase_modulo_lidar_period']['p50'])} "
            f"P90={fmt(report['phase_modulo_lidar_period']['p90'])} "
            f"P99={fmt(report['phase_modulo_lidar_period']['p99'])}"
        ),
        summary("camera inter-arrival", report["camera_interarrival"]),
        summary("LiDAR inter-arrival", report["lidar_interarrival"]),
        "fixed offset claimed: NO",
    ]
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
