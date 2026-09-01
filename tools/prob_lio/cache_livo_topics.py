#!/usr/bin/env python3
"""Build an exact, persistent Oxford LIVO transport cache.

The cache is a topic subset plus a new rosbag container/compression.  Sensor
messages are copied in raw serialized form; no image decode, re-encode,
timestamp rewrite, resize, crop, or ordering change is permitted.
"""

import argparse
import hashlib
import os
import pathlib
import struct
import sys
import time

import yaml


DEFAULT_TOPICS = [
    "/hesai/pandar",
    "/alphasense_driver_ros/imu",
    "/alphasense_driver_ros/cam0/debayered/image/compressed",
]


def sha256_file(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _record_update(digest, topic, stamp_ns, payload):
    topic_bytes = topic.encode("utf-8")
    payload = bytes(payload)
    digest.update(struct.pack(">I", len(topic_bytes)))
    digest.update(topic_bytes)
    digest.update(struct.pack(">q", int(stamp_ns)))
    digest.update(struct.pack(">Q", len(payload)))
    digest.update(payload)


def _new_summary():
    return {
        "records": 0,
        "first_stamp_ns": None,
        "last_stamp_ns": None,
        "record_sha256": hashlib.sha256(),
        "payload_sha256": hashlib.sha256(),
        "topics": {},
    }


def _topic_summary():
    return {
        "records": 0,
        "first_stamp_ns": None,
        "last_stamp_ns": None,
        "record_sha256": hashlib.sha256(),
        "payload_sha256": hashlib.sha256(),
        "type": None,
    }


def _update_summary(summary, topic, raw, stamp):
    stamp_ns = stamp.to_nsec() if hasattr(stamp, "to_nsec") else int(stamp)
    payload = bytes(raw[1])
    topic_summary = summary["topics"].setdefault(topic, _topic_summary())
    topic_summary["records"] += 1
    topic_summary["first_stamp_ns"] = (
        stamp_ns if topic_summary["first_stamp_ns"] is None
        else topic_summary["first_stamp_ns"]
    )
    topic_summary["last_stamp_ns"] = stamp_ns
    topic_summary["type"] = raw[0]
    _record_update(topic_summary["record_sha256"], topic, stamp_ns, payload)
    topic_summary["payload_sha256"].update(payload)
    summary["records"] += 1
    summary["first_stamp_ns"] = (
        stamp_ns if summary["first_stamp_ns"] is None else summary["first_stamp_ns"]
    )
    summary["last_stamp_ns"] = stamp_ns
    _record_update(summary["record_sha256"], topic, stamp_ns, payload)
    summary["payload_sha256"].update(payload)


def _freeze_summary(summary):
    return {
        "records": summary["records"],
        "first_stamp_ns": summary["first_stamp_ns"],
        "last_stamp_ns": summary["last_stamp_ns"],
        "record_sha256": summary["record_sha256"].hexdigest(),
        "payload_sha256": summary["payload_sha256"].hexdigest(),
        "topics": {
            topic: {
                "records": value["records"],
                "first_stamp_ns": value["first_stamp_ns"],
                "last_stamp_ns": value["last_stamp_ns"],
                "record_sha256": value["record_sha256"].hexdigest(),
                "payload_sha256": value["payload_sha256"].hexdigest(),
                "type": value["type"],
            }
            for topic, value in sorted(summary["topics"].items())
        },
    }


def _copy_raw(source_path, partial_path, topics):
    try:
        import rosbag
    except ImportError as exc:  # pragma: no cover - CLI environment only
        raise RuntimeError("ROS1 rosbag Python module is required") from exc

    summary = _new_summary()
    with rosbag.Bag(str(source_path), "r") as source:
        with rosbag.Bag(str(partial_path), "w", compression="lz4") as target:
            for topic, raw, stamp in source.read_messages(topics=topics, raw=True):
                _update_summary(summary, topic, raw, stamp)
                target.write(topic, raw, t=stamp, raw=True)
    return _freeze_summary(summary)


def _summarize(path, topics):
    try:
        import rosbag
    except ImportError as exc:  # pragma: no cover - CLI environment only
        raise RuntimeError("ROS1 rosbag Python module is required") from exc

    summary = _new_summary()
    with rosbag.Bag(str(path), "r") as bag:
        topic_info = bag.get_type_and_topic_info().topics
        if set(topic_info) != set(topics):
            raise RuntimeError(
                "cache topic set mismatch: expected %s, got %s"
                % (sorted(topics), sorted(topic_info))
            )
        for topic, raw, stamp in bag.read_messages(topics=topics, raw=True):
            _update_summary(summary, topic, raw, stamp)
    return _freeze_summary(summary)


def _parity(source, cache, topics):
    differences = []
    for field in ("records", "first_stamp_ns", "last_stamp_ns", "record_sha256"):
        if source[field] != cache[field]:
            differences.append(field)
    if set(source["topics"]) != set(cache["topics"]):
        differences.append("topic_set")

    topic_report = {}
    for topic in topics:
        left = source["topics"].get(topic, {})
        right = cache["topics"].get(topic, {})
        payload_ok = left.get("payload_sha256") == right.get("payload_sha256")
        timestamp_ok = (
            left.get("records") == right.get("records")
            and left.get("first_stamp_ns") == right.get("first_stamp_ns")
            and left.get("last_stamp_ns") == right.get("last_stamp_ns")
        )
        order_ok = left.get("record_sha256") == right.get("record_sha256")
        if not payload_ok:
            differences.append("payload:%s" % topic)
        if not timestamp_ok:
            differences.append("timestamp:%s" % topic)
        if not order_ok:
            differences.append("ordering:%s" % topic)
        topic_report[topic] = {
            "type": right.get("type"),
            "source_count": left.get("records"),
            "cache_count": right.get("records"),
            "payload_sha256_source": left.get("payload_sha256"),
            "payload_sha256_cache": right.get("payload_sha256"),
            "payload_parity": "PASS" if payload_ok else "FAIL",
            "timestamp_parity": "PASS" if timestamp_ok else "FAIL",
            "ordering_parity": "PASS" if order_ok else "FAIL",
        }
    return {
        "status": "PASS" if not differences else "FAIL",
        "differences": differences,
        "ordering_parity": "PASS" if not differences else "FAIL",
        "timestamp_parity": "PASS" if not differences else "FAIL",
        "topics": topic_report,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--manifest", required=True)
    parser.add_argument("--topic", action="append", default=None)
    parser.add_argument("--overwrite", action="store_true")
    args = parser.parse_args(argv)

    source = pathlib.Path(args.input).resolve()
    output = pathlib.Path(args.output).resolve()
    manifest = pathlib.Path(args.manifest).resolve()
    topics = args.topic or list(DEFAULT_TOPICS)
    if not source.is_file():
        print("source bag missing: %s" % source, file=sys.stderr)
        return 2
    if (output.exists() or manifest.exists()) and not args.overwrite:
        print("refusing to overwrite existing cache or manifest", file=sys.stderr)
        return 2
    output.parent.mkdir(parents=True, exist_ok=True)
    manifest.parent.mkdir(parents=True, exist_ok=True)
    partial = pathlib.Path(str(output) + ".partial")
    if partial.exists():
        print("refusing to overwrite partial cache: %s" % partial, file=sys.stderr)
        return 2

    started = time.monotonic()
    try:
        source_summary = _copy_raw(source, partial, topics)
        os.replace(partial, output)
        cache_summary = _summarize(output, topics)
    except (OSError, RuntimeError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    build_time = time.monotonic() - started
    comparison = _parity(source_summary, cache_summary, topics)
    if comparison["status"] != "PASS":
        print("cache parity failed: %s" % comparison["differences"], file=sys.stderr)
        return 1

    document = {
        "schema_version": 1,
        "status": "PASS",
        "source_bag": {
            "path": str(source),
            "size_bytes": source.stat().st_size,
            "sha256": sha256_file(source),
            "compression": "bz2",
        },
        "cache_bag": {
            "path": str(output),
            "size_bytes": output.stat().st_size,
            "sha256": sha256_file(output),
            "compression": "lz4",
        },
        "build_time_s": build_time,
        "topics": topics,
        "source_summary": source_summary,
        "cache_summary": cache_summary,
        "parity": comparison,
        "camera_policy": {
            "official_camera": "cam0",
            "retained_topic": "/alphasense_driver_ros/cam0/debayered/image/compressed",
            "excluded_topics": [
                "/alphasense_driver_ros/cam1/debayered/image/compressed",
                "/alphasense_driver_ros/cam2/debayered/image/compressed",
            ],
            "conversion": "none",
        },
        "transport": "topic subset plus LZ4 bag container; raw serialized payloads preserved",
    }
    manifest.write_text(yaml.safe_dump(document, sort_keys=False), encoding="utf-8")
    print("cache: %s" % output)
    print("manifest: %s" % manifest)
    print("build_time_s=%.3f" % build_time)
    print("status: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
