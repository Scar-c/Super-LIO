#!/usr/bin/env python3
"""Streaming GEODE PointCloud2 authority audit for Prompt07.

The audit deliberately reads the bag payload without routing it through the
native estimator.  This keeps raw population and point-time claims independent
of Super-LIO preprocessing.
"""

from __future__ import annotations

import argparse
import json
import math
import random
from collections import Counter
from pathlib import Path

import numpy as np
import rosbag


DATATYPES = {
    1: ("int8", np.dtype("<i1")),
    2: ("uint8", np.dtype("<u1")),
    3: ("int16", np.dtype("<i2")),
    4: ("uint16", np.dtype("<u2")),
    5: ("int32", np.dtype("<i4")),
    6: ("uint32", np.dtype("<u4")),
    7: ("float32", np.dtype("<f4")),
    8: ("float64", np.dtype("<f8")),
}


class Reservoir:
    def __init__(self, size: int, seed: int = 707):
        self.size = size
        self.values: list[float] = []
        self.seen = 0
        self.rng = random.Random(seed)

    def add(self, values: np.ndarray) -> None:
        for value in values.tolist():
            self.seen += 1
            if len(self.values) < self.size:
                self.values.append(float(value))
            else:
                slot = self.rng.randrange(self.seen)
                if slot < self.size:
                    self.values[slot] = float(value)

    def summary(self) -> dict[str, float | int | None]:
        if not self.values:
            return {"count_seen": self.seen, "min": None, "median": None,
                    "p95": None, "max": None, "sample_size": 0}
        values = np.asarray(self.values, dtype=np.float64)
        return {
            "count_seen": self.seen,
            "min": float(np.min(values)),
            "median": float(np.median(values)),
            "p95": float(np.quantile(values, 0.95)),
            "max": float(np.max(values)),
            "sample_size": len(values),
        }


def make_dtype(fields, point_step: int) -> np.dtype:
    names = []
    formats = []
    offsets = []
    for field in fields:
        if field.name in {"x", "y", "z", "ring", "time", "t", "timestamp"}:
            if field.datatype not in DATATYPES:
                raise RuntimeError(f"unsupported PointField datatype {field.datatype}")
            names.append(field.name)
            formats.append(DATATYPES[field.datatype][1])
            offsets.append(field.offset)
    return np.dtype({"names": names, "formats": formats, "offsets": offsets,
                     "itemsize": point_step})


def array_for(message, dtype: np.dtype) -> np.ndarray:
    count = int(message.width) * int(message.height)
    return np.frombuffer(message.data, dtype=dtype, count=count)


def finite_xyz(array: np.ndarray) -> np.ndarray:
    return (np.isfinite(array["x"]) & np.isfinite(array["y"]) &
            np.isfinite(array["z"]))


def point_stats(array: np.ndarray, blind_values=(1.5, 2.0),
                upper_values=(100.0, 150.0)) -> dict:
    xyz_finite = finite_xyz(array)
    x = array["x"].astype(np.float64, copy=False)
    y = array["y"].astype(np.float64, copy=False)
    z = array["z"].astype(np.float64, copy=False)
    d2 = x * x + y * y + z * z
    finite_range = np.isfinite(d2) & xyz_finite
    ranges = np.sqrt(d2[finite_range])
    result = {
        "N_raw": int(len(array)),
        "N_finite": int(np.count_nonzero(xyz_finite)),
        "N_nan_xyz": int(np.count_nonzero(np.isnan(x) | np.isnan(y) | np.isnan(z))),
        "N_inf_xyz": int(np.count_nonzero(np.isinf(x) | np.isinf(y) | np.isinf(z))),
        "max_range": float(np.max(ranges)) if len(ranges) else None,
        "range_median": float(np.median(ranges)) if len(ranges) else None,
    }
    for blind in blind_values:
        result[f"N_after_blind_{str(blind).replace('.', 'p')}"] = int(
            np.count_nonzero(finite_range & (d2 > blind * blind)))
    for upper in upper_values:
        result[f"N_gt_{str(upper).replace('.', 'p')}"] = int(
            np.count_nonzero(finite_range & (np.sqrt(d2) > upper)))
    if "ring" in array.dtype.names:
        rings = array["ring"]
        result["ring_min"] = int(np.min(rings)) if len(rings) else None
        result["ring_max"] = int(np.max(rings)) if len(rings) else None
        result["ring_unique"] = sorted(int(v) for v in np.unique(rings))
    return result


def summarize_time(array: np.ndarray) -> dict:
    field = "time" if "time" in array.dtype.names else (
        "t" if "t" in array.dtype.names else "timestamp")
    values = array[field].astype(np.float64, copy=False)
    finite = values[np.isfinite(values)]
    diff = np.diff(values)
    return {
        "field": field,
        "dtype": str(array.dtype[field]),
        "min": float(np.min(finite)) if len(finite) else None,
        "median": float(np.median(finite)) if len(finite) else None,
        "p95": float(np.quantile(finite, 0.95)) if len(finite) else None,
        "max": float(np.max(finite)) if len(finite) else None,
        "finite_count": int(len(finite)),
        "nonfinite_count": int(len(values) - len(finite)),
        "monotonic_non_decreasing": bool(np.all(diff >= 0)) if len(diff) else True,
        "decrease_count": int(np.count_nonzero(diff < 0)),
        "scan_duration": float(np.max(finite) - np.min(finite)) if len(finite) else None,
    }


def merge_counts(target: Counter, source: dict) -> None:
    for key in ("N_raw", "N_finite", "N_nan_xyz", "N_inf_xyz", "N_after_blind_1p5",
                "N_after_blind_2p0", "N_gt_100p0", "N_gt_150p0"):
        target[key] += int(source.get(key, 0))


def audit_bag(path: Path, topic: str, reservoir_size: int) -> dict:
    bag = rosbag.Bag(str(path), "r")
    expected = bag.get_message_count(topic_filters=[topic])
    count = 0
    raw_counts = Counter()
    time_reservoir = Reservoir(reservoir_size)
    ring_counts: Counter[str] = Counter()
    time_decrease_count = 0
    time_nonfinite = 0
    frame_summaries = []
    representatives = {}
    dtype_meta = None
    first_stamp = None
    last_stamp = None
    max_range = None

    for _, message, bag_time in bag.read_messages(topics=[topic]):
        if dtype_meta is None:
            dtype = make_dtype(message.fields, message.point_step)
            dtype_meta = {
                "fields": [{"name": f.name, "offset": f.offset,
                            "datatype": f.datatype,
                            "datatype_name": DATATYPES.get(f.datatype, ("unknown",))[0],
                            "count": f.count} for f in message.fields],
                "point_step": int(message.point_step),
                "row_step": int(message.row_step),
                "width": int(message.width),
                "height": int(message.height),
            }
        array = array_for(message, dtype)
        stats = point_stats(array)
        strided_stats = point_stats(array[::3])
        time_summary = summarize_time(array)
        merge_counts(raw_counts, stats)
        if "ring" in array.dtype.names:
            for ring, number in zip(*np.unique(array["ring"], return_counts=True)):
                ring_counts[str(int(ring))] += int(number)
        field = time_summary["field"]
        values = array[field].astype(np.float64, copy=False)
        finite_times = values[np.isfinite(values)]
        time_reservoir.add(finite_times)
        time_decrease_count += time_summary["decrease_count"]
        time_nonfinite += time_summary["nonfinite_count"]
        frame_summaries.append({
            "index": count,
            "bag_time": float(bag_time.to_sec()),
            "header_time": float(message.header.stamp.to_sec()),
            "points": int(len(array)),
            "time": time_summary,
            "N_finite": stats["N_finite"],
            "N_finite_after_raw_stride3": strided_stats["N_finite"],
            "N_after_finite_then_stride3": int(np.ceil(stats["N_finite"] / 3.0)),
            "N_after_raw_stride3": len(array[::3]),
            "N_after_raw_stride3_blind_1p5": strided_stats["N_after_blind_1p5"],
            "N_after_raw_stride3_blind_2p0": strided_stats["N_after_blind_2p0"],
            "N_after_raw_stride3_upper_100": strided_stats["N_gt_100p0"],
            "N_after_raw_stride3_upper_150": strided_stats["N_gt_150p0"],
            "N_after_blind_1p5": stats["N_after_blind_1p5"],
            "N_after_blind_2p0": stats["N_after_blind_2p0"],
            "N_gt_100p0": stats["N_gt_100p0"],
            "N_gt_150p0": stats["N_gt_150p0"],
        })
        if first_stamp is None:
            first_stamp = float(message.header.stamp.to_sec())
        last_stamp = float(message.header.stamp.to_sec())
        if stats["max_range"] is not None:
            max_range = stats["max_range"] if max_range is None else max(max_range, stats["max_range"])
        count += 1
        if count in {1, (expected // 2) + 1, expected}:
            representatives[str(count - 1)] = {
                "frame": frame_summaries[-1],
                "ring_counts": {str(int(v)): int(n) for v, n in
                                 zip(*np.unique(array["ring"], return_counts=True))}
                if "ring" in array.dtype.names else {},
            }
    bag.close()

    for frame in frame_summaries:
        # Preserve a small, deterministic set of representative frame records.
        if frame["index"] in {0, count // 2, count - 1}:
            representatives[str(frame["index"])] = {"frame": frame}

    whole_time = time_reservoir.summary()
    raw_counts["N_after_raw_stride3_before_finite"] = int(
        sum(frame["points"] // 3 + (1 if frame["points"] % 3 else 0)
            for frame in frame_summaries))
    raw_counts["N_after_raw_stride3_finite"] = int(
        sum(frame["N_finite_after_raw_stride3"] for frame in frame_summaries))
    raw_counts["N_after_finite_then_stride3"] = int(
        sum(frame["N_after_finite_then_stride3"] for frame in frame_summaries))
    # A finite-then-stride implementation is equivalent on this input only if
    # no non-finite points exist; the exact finite-then-stride count is computed
    # above as a conservative per-frame audit statistic.
    raw_counts["N_after_blind_1p5_then_stride3"] = int(
        sum(frame["N_after_raw_stride3_blind_1p5"] for frame in frame_summaries))
    raw_counts["N_after_blind_2p0_then_stride3"] = int(
        sum(frame["N_after_raw_stride3_blind_2p0"] for frame in frame_summaries))
    raw_counts["N_after_stride3_then_blind_1p5"] = int(
        sum(frame["N_after_raw_stride3_blind_1p5"] for frame in frame_summaries))
    raw_counts["N_after_stride3_then_blind_2p0"] = int(
        sum(frame["N_after_raw_stride3_blind_2p0"] for frame in frame_summaries))
    raw_counts["N_after_raw_stride3_gt_100p0"] = int(
        sum(frame["N_after_raw_stride3_upper_100"] for frame in frame_summaries))
    raw_counts["N_after_raw_stride3_gt_150p0"] = int(
        sum(frame["N_after_raw_stride3_upper_150"] for frame in frame_summaries))

    return {
        "bag": str(path),
        "topic": topic,
        "topic_message_count_expected": int(expected),
        "topic_message_count_read": count,
        "datatype": "sensor_msgs/PointCloud2",
        "pointcloud_layout": dtype_meta,
        "raw_population": dict(raw_counts),
        "whole_bag_time_reservoir": whole_time,
        "time_decrease_count": time_decrease_count,
        "time_nonfinite_count": time_nonfinite,
        "frame_time_monotonic_all_frames": time_decrease_count == 0,
        "ring_counts": dict(sorted(ring_counts.items(), key=lambda item: int(item[0]))),
        "header_stamp_first": first_stamp,
        "header_stamp_last": last_stamp,
        "header_duration": None if first_stamp is None or last_stamp is None else last_stamp - first_stamp,
        "max_observed_range": max_range,
        "representatives": representatives,
        "frame_summaries": frame_summaries,
        "audit_notes": [
            "time quantiles are deterministic reservoir quantiles over finite point times",
            "raw stride uses indices 0,3,6,..., matching the native ROSWrapper loop",
            "range counts use Euclidean XYZ range and strict blind/upper inequalities",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tunnel", required=True, type=Path)
    parser.add_argument("--stairs", required=True, type=Path)
    parser.add_argument("--topic", default="/velodyne_points")
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--reservoir-size", default=1_000_000, type=int)
    args = parser.parse_args()
    result = {
        "schema_version": 1,
        "topic": args.topic,
        "tunnel": audit_bag(args.tunnel, args.topic, args.reservoir_size),
        "stairs": audit_bag(args.stairs, args.topic, args.reservoir_size),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({
        key: {
            "messages": value["topic_message_count_read"],
            "points": value["raw_population"]["N_raw"],
            "time": value["whole_bag_time_reservoir"],
            "max_range": value["max_observed_range"],
        }
        for key, value in (("tunnel", result["tunnel"]), ("stairs", result["stairs"]))
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
