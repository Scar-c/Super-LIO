#!/usr/bin/env python3
"""Shared bounded-memory helpers for Round 11T ROS bag audits."""
import heapq
import pathlib
import subprocess

import numpy as np
import rosbag


def git_head(script_path):
    repo_root = pathlib.Path(script_path).resolve().parents[3]
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=str(repo_root),
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "UNKNOWN"


def header_seconds(message):
    if not hasattr(message, "header") or not hasattr(message.header, "stamp"):
        raise ValueError("selected message has no header.stamp")
    return float(message.header.stamp.to_sec())


def iter_record_order(bag_paths, topics):
    """K-way merge selected messages with one live item per bag."""
    bags, iterators, heap = [], [], []
    for bag_index, path in enumerate(bag_paths):
        bag = rosbag.Bag(path)
        bags.append(bag)
        iterator = bag.read_messages(topics=topics)
        iterators.append(iterator)
        try:
            topic, message, record_time = next(iterator)
            heapq.heappush(
                heap,
                (
                    int(record_time.to_nsec()),
                    bag_index,
                    0,
                    topic,
                    message,
                    record_time,
                ),
            )
        except StopIteration:
            pass
    try:
        while heap:
            record_ns, bag_index, sequence, topic, message, record_time = (
                heapq.heappop(heap)
            )
            yield topic, message, record_time
            try:
                next_topic, next_message, next_record = next(iterators[bag_index])
                heapq.heappush(
                    heap,
                    (
                        int(next_record.to_nsec()),
                        bag_index,
                        sequence + 1,
                        next_topic,
                        next_message,
                        next_record,
                    ),
                )
            except StopIteration:
                pass
    finally:
        for bag in bags:
            bag.close()


def distribution(values, percentiles=(10, 50, 90, 99)):
    array = np.asarray(values, dtype=float)
    result = {"count": int(array.size)}
    names = {10: "p10", 50: "p50", 90: "p90", 95: "p95", 99: "p99"}
    if not array.size:
        for key in ("min", "p10", "p50", "p90", "p95", "p99",
                    "max", "mean", "std"):
            result[key] = None
        return result
    result.update({
        "min": float(array.min()),
        "max": float(array.max()),
        "mean": float(array.mean()),
        "std": float(array.std()),
    })
    for percentile in percentiles:
        result[names[percentile]] = float(np.percentile(array, percentile))
    for percentile in (10, 50, 90, 95, 99):
        result.setdefault(names[percentile], None)
    return result


def linear_slope(x_values, y_values):
    x_array = np.asarray(x_values, dtype=float)
    y_array = np.asarray(y_values, dtype=float)
    if x_array.size < 2 or np.all(x_array == x_array[0]):
        return 0.0
    centered = x_array - x_array[0]
    return float(np.polyfit(centered, y_array, 1)[0])
