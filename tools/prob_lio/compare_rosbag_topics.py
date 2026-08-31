#!/usr/bin/env python3
"""Compare selected rosbag records without deserializing sensor messages.

The comparison digest includes topic, bag timestamp, and serialized payload.
It is therefore sensitive to record order, timestamps, and message bytes while
keeping the report small enough for Git-tracked evidence.
"""

import argparse
import hashlib
import pathlib
import struct
import sys

import yaml


def _record_digest_update(digest, topic, stamp_ns, payload):
    topic_bytes = topic.encode("utf-8")
    payload = bytes(payload)
    digest.update(struct.pack(">I", len(topic_bytes)))
    digest.update(topic_bytes)
    digest.update(struct.pack(">q", int(stamp_ns)))
    digest.update(struct.pack(">Q", len(payload)))
    digest.update(payload)


def _topic_summary():
    return {
        "records": 0,
        "first_stamp_ns": None,
        "last_stamp_ns": None,
        "serialized_sha256": hashlib.sha256(),
    }


def summarize_records(records):
    """Return a compact deterministic summary for ``(topic, stamp_ns, bytes)``."""

    overall_digest = hashlib.sha256()
    topics = {}
    count = 0
    first_stamp = None
    last_stamp = None
    for topic, stamp_ns, payload in records:
        stamp_ns = int(stamp_ns)
        payload = bytes(payload)
        topic_summary = topics.setdefault(topic, _topic_summary())
        topic_summary["records"] += 1
        if topic_summary["first_stamp_ns"] is None:
            topic_summary["first_stamp_ns"] = stamp_ns
        topic_summary["last_stamp_ns"] = stamp_ns
        _record_digest_update(topic_summary["serialized_sha256"], topic, stamp_ns, payload)
        _record_digest_update(overall_digest, topic, stamp_ns, payload)
        count += 1
        if first_stamp is None:
            first_stamp = stamp_ns
        last_stamp = stamp_ns

    return {
        "records": count,
        "first_stamp_ns": first_stamp,
        "last_stamp_ns": last_stamp,
        "serialized_sha256": overall_digest.hexdigest(),
        "topics": {
            topic: {
                **value,
                "serialized_sha256": value["serialized_sha256"].hexdigest(),
            }
            for topic, value in topics.items()
        },
    }


def compare_summaries(original, candidate, required_topics=None):
    """Compare two summaries and return a machine-readable PASS/FAIL report."""

    required_topics = list(required_topics or [])
    differences = []
    for field in ("records", "first_stamp_ns", "last_stamp_ns", "serialized_sha256"):
        if original.get(field) != candidate.get(field):
            differences.append(field)
    original_topics = original.get("topics", {})
    candidate_topics = candidate.get("topics", {})
    if set(original_topics) != set(candidate_topics):
        differences.append("topic_set")
    for topic in sorted(set(original_topics) | set(candidate_topics)):
        if original_topics.get(topic) != candidate_topics.get(topic):
            differences.append(f"topic:{topic}")
    missing_topics = [topic for topic in required_topics if topic not in candidate_topics]
    if missing_topics:
        differences.append("required_topics")
    return {
        "status": "PASS" if not differences else "FAIL",
        "differences": differences,
        "missing_topics": missing_topics,
    }


def rosbag_records(path, topics):
    """Yield raw records from ROS1 rosbag lazily; ROS is imported only here."""

    try:
        import rosbag
    except ImportError as exc:  # pragma: no cover - exercised in CLI env only
        raise RuntimeError("ROS1 rosbag Python module is required") from exc
    with rosbag.Bag(str(path), "r") as bag:
        for topic, raw, stamp in bag.read_messages(topics=topics or None, raw=True):
            yield topic, stamp.to_nsec(), raw[1]


def summarize_bag(path, topics=None):
    return summarize_records(rosbag_records(path, topics))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original", required=True)
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--topic", action="append", default=[])
    parser.add_argument("--out", required=True)
    args = parser.parse_args(argv)

    original = pathlib.Path(args.original).resolve()
    candidate = pathlib.Path(args.candidate).resolve()
    if not original.is_file() or not candidate.is_file():
        print("both --original and --candidate must be files", file=sys.stderr)
        return 2
    try:
        original_summary = summarize_bag(original, args.topic)
        candidate_summary = summarize_bag(candidate, args.topic)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    comparison = compare_summaries(
        original_summary, candidate_summary, required_topics=args.topic
    )
    report = {
        "schema_version": 1,
        "original": {"path": str(original), **original_summary},
        "candidate": {"path": str(candidate), **candidate_summary},
        "required_topics": args.topic,
        "comparison": comparison,
    }
    output = pathlib.Path(args.out)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(yaml.safe_dump(report, sort_keys=False), encoding="utf-8")
    print(f"status: {comparison['status']}")
    print(f"report: {output.resolve()}")
    return 0 if comparison["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
