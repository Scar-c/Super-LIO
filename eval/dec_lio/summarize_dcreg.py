#!/usr/bin/env python3
"""Summarize the per-frame D1 shadow diagnostics without changing the run."""

import argparse
import csv
import json
import math
import pathlib
import statistics
import sys


AXES = ("rot_0", "rot_1", "rot_2", "trans_0", "trans_1", "trans_2")


def number(row, name):
    value = row.get(name, "")
    try:
        return float(value)
    except (TypeError, ValueError):
        return math.nan


def percentile(values, fraction):
    if not values:
        return None
    ordered = sorted(values)
    index = min(len(ordered) - 1, max(0, math.ceil(fraction * len(ordered)) - 1))
    return ordered[index]


def condition_stats(rows, name):
    values = [number(row, name) for row in rows]
    values = [value for value in values if math.isfinite(value)]
    if not values:
        return {"count": 0, "median": None, "p95": None, "max": None}
    return {
        "count": len(values),
        "median": statistics.median(values),
        "p95": percentile(values, 0.95),
        "max": max(values),
    }


def weak_windows(rows, axis_name):
    column = "diagnostic_" + axis_name
    windows = []
    current = []
    for row in rows:
        if row.get(column) == "1":
            current.append(row)
            continue
        if current:
            windows.append(current)
            current = []
    if current:
        windows.append(current)
    windows.sort(key=lambda window: (len(window), number(window[0], "lidar_end_time")), reverse=True)
    result = []
    for window in windows[:3]:
        result.append({
            "start_frame": int(float(window[0]["frame"])),
            "end_frame": int(float(window[-1]["frame"])),
            "frames": len(window),
            "start_time": number(window[0], "lidar_end_time"),
            "end_time": number(window[-1], "lidar_end_time"),
            "duration_s": number(window[-1], "lidar_end_time") - number(window[0], "lidar_end_time"),
        })
    return result


def summarize(path, threshold):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        rows = [row for row in csv.DictReader(stream) if row.get("frame")]
    valid = [row for row in rows if row.get("valid") == "1"]
    factorization_failures = sum(row.get("factorization_ok") != "1" for row in rows)
    result = {
        "input": str(pathlib.Path(path).resolve()),
        "condition_threshold": threshold,
        "total_frames": len(rows),
        "valid_frames": len(valid),
        "invalid_frames": len(rows) - len(valid),
        "valid_frame_ratio": len(valid) / len(rows) if rows else 0.0,
        "factorization_failures": factorization_failures,
        "condition_rot": condition_stats(valid, "cond_rot"),
        "condition_trans": condition_stats(valid, "cond_trans"),
        "weak_axis_counts": {},
        "weak_axis_ratios": {},
        "longest_weak_runs": {},
        "top_weak_windows": {},
    }
    for axis in AXES:
        column = "diagnostic_" + axis
        count = sum(row.get(column) == "1" for row in valid)
        result["weak_axis_counts"][axis] = count
        result["weak_axis_ratios"][axis] = count / len(valid) if valid else 0.0
        windows = weak_windows(valid, axis)
        result["longest_weak_runs"][axis] = windows[0]["frames"] if windows else 0
        result["top_weak_windows"][axis] = windows
    return result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("summary_csv", type=pathlib.Path)
    parser.add_argument("--threshold", type=float, default=10.0)
    parser.add_argument("--out", type=pathlib.Path)
    args = parser.parse_args(argv)
    report = json.dumps(summarize(args.summary_csv, args.threshold), indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.write_text(report, encoding="utf-8")
    else:
        sys.stdout.write(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
