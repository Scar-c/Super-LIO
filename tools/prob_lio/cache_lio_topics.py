#!/usr/bin/env python3
"""Create a LIO-only ROS1 bag by copying selected serialized records."""

import argparse
import hashlib
import os
import pathlib
import sys

import yaml


DEFAULT_TOPICS = ["/hesai/pandar", "/alphasense_driver_ros/imu"]


def sha256_file(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_cache(input_path, output_path, topics):
    """Copy raw records in original bag order, using LZ4 chunk compression."""

    try:
        import rosbag
    except ImportError as exc:  # pragma: no cover - exercised in CLI env only
        raise RuntimeError("ROS1 rosbag Python module is required") from exc

    counts = {topic: 0 for topic in topics}
    with rosbag.Bag(str(input_path), "r") as source:
        with rosbag.Bag(str(output_path), "w", compression="lz4") as target:
            for topic, raw, stamp in source.read_messages(topics=topics, raw=True):
                target.write(topic, raw, t=stamp, raw=True)
                counts[topic] = counts.get(topic, 0) + 1
    return counts


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--topic", action="append", default=None)
    parser.add_argument("--report", required=True)
    parser.add_argument("--overwrite", action="store_true")
    args = parser.parse_args(argv)

    input_path = pathlib.Path(args.input).resolve()
    output_path = pathlib.Path(args.output).resolve()
    report_path = pathlib.Path(args.report).resolve()
    topics = args.topic or list(DEFAULT_TOPICS)
    if not input_path.is_file():
        print(f"input bag missing: {input_path}", file=sys.stderr)
        return 2
    if output_path.exists() and not args.overwrite:
        print(f"refusing to overwrite cache: {output_path}", file=sys.stderr)
        return 2
    output_path.parent.mkdir(parents=True, exist_ok=True)
    partial_path = pathlib.Path(str(output_path) + ".partial")
    if partial_path.exists():
        print(f"refusing to overwrite partial cache: {partial_path}", file=sys.stderr)
        return 2
    try:
        counts = build_cache(input_path, partial_path, topics)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    os.replace(partial_path, output_path)
    report = {
        "schema_version": 1,
        "input": {
            "path": str(input_path),
            "size_bytes": input_path.stat().st_size,
            "sha256": sha256_file(input_path),
        },
        "output": {
            "path": str(output_path),
            "size_bytes": output_path.stat().st_size,
            "sha256": sha256_file(output_path),
            "compression": "lz4",
        },
        "topics": topics,
        "message_counts": counts,
        "transport": "raw serialized records; original record order and bag timestamps",
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(yaml.safe_dump(report, sort_keys=False), encoding="utf-8")
    print(f"cache: {output_path}")
    print(f"report: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
