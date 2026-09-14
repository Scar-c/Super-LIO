#!/usr/bin/env python3
"""Exact GEODE finite/stride and given_offset_time audit for Prompt07R."""

from __future__ import annotations

import argparse
import json
import math
from collections import Counter
from pathlib import Path

import numpy as np
import rosbag

from prompt07r_semantics import (blind_indices, finite_xyz, geode_given_offset_time,
                                 geode_indices, native_indices,
                                 quantile_summary, reconstruct_offsets_ms,
                                 set_metrics)


DATATYPES = {
    1: ("int8", np.dtype("<i1")), 2: ("uint8", np.dtype("<u1")),
    3: ("int16", np.dtype("<i2")), 4: ("uint16", np.dtype("<u2")),
    5: ("int32", np.dtype("<i4")), 6: ("uint32", np.dtype("<u4")),
    7: ("float32", np.dtype("<f4")), 8: ("float64", np.dtype("<f8")),
}


def make_dtype(fields, point_step: int) -> np.dtype:
    names, formats, offsets = [], [], []
    for field in fields:
        if field.name in {"x", "y", "z", "ring", "time"}:
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


def run_lengths(values: list[bool], target: bool) -> dict:
    best = current = 0
    best_start = None
    current_start = None
    for index, value in enumerate(values):
        if value == target:
            if current == 0:
                current_start = index
            current += 1
            if current > best:
                best = current
                best_start = current_start
        else:
            current = 0
            current_start = None
    return {"length": best,
            "start": best_start,
            "end": None if best_start is None else best_start + best - 1}


def add_values(store: list[float], values: np.ndarray, limit: int = 300000) -> None:
    """Keep deterministic bounded values; exact extrema are tracked separately."""
    if len(store) >= limit or len(values) == 0:
        return
    take = min(limit - len(store), len(values))
    store.extend(float(value) for value in values[:take])


def distribution(values: list[float], minimum: float | None,
                 maximum: float | None, count: int) -> dict:
    result = quantile_summary(values)
    result["count_seen"] = count
    result["min_seen"] = minimum
    result["max_seen"] = maximum
    return result


def audit_bag(path: Path, topic: str, name: str) -> dict:
    bag = rosbag.Bag(str(path), "r")
    expected = bag.get_message_count(topic_filters=[topic])
    dtype = None
    records = []
    branches = []
    final_times = []
    final_indices = []
    raw_total = finite_total = nan_total = inf_total = 0
    pre_metrics = []
    post2_metrics = []
    post15_metrics = []
    true_values_s, false_values_ms, false_selected_ms = [], [], []
    true_count_points = false_count_points = 0
    true_min = true_max = false_min = false_max = None
    true_duration_values = []
    false_duration_values = []

    for frame, (_, message, bag_time) in enumerate(bag.read_messages(topics=[topic])):
        if dtype is None:
            dtype = make_dtype(message.fields, message.point_step)
        array = array_for(message, dtype)
        finite = finite_xyz(array)
        finite_indices = np.flatnonzero(finite).astype(np.int64, copy=False)
        branch = geode_given_offset_time(array, finite)
        given = bool(branch["given_offset_time"])
        branches.append(given)
        raw_total += len(array)
        finite_total += int(np.count_nonzero(finite))
        nan_total += int(np.count_nonzero(np.isnan(array["x"]) |
                                          np.isnan(array["y"]) |
                                          np.isnan(array["z"])))
        inf_total += int(np.count_nonzero(np.isinf(array["x"]) |
                                          np.isinf(array["y"]) |
                                          np.isinf(array["z"])))
        if branch["final_finite_time_s"] is not None:
            final_times.append(branch["final_finite_time_s"])
            final_indices.append(branch["final_finite_original_index"])

        native = native_indices(finite)
        geode = geode_indices(finite)
        native2 = blind_indices(array, native, 2.0)
        geode2 = blind_indices(array, geode, 2.0)
        native15 = blind_indices(array, native, 1.5)
        geode15 = blind_indices(array, geode, 1.5)
        before = set_metrics(native, geode)
        after2 = set_metrics(native2, geode2)
        after15 = set_metrics(native15, geode15)
        pre_metrics.append(before)
        post2_metrics.append(after2)
        post15_metrics.append(after15)

        record = {
            "frame": frame,
            "bag_time": float(bag_time.to_sec()),
            "header_time": float(message.header.stamp.to_sec()),
            "N_raw": int(len(array)),
            **branch,
            "N_native_before_blind": int(len(native)),
            "N_geode_before_blind": int(len(geode)),
            "N_native_blind_2p0": int(len(native2)),
            "N_geode_blind_2p0": int(len(geode2)),
            "N_native_blind_1p5": int(len(native15)),
            "N_geode_blind_1p5": int(len(geode15)),
            "native_vs_geode": before,
            "native_vs_geode_blind_2p0": after2,
            "native_vs_geode_blind_1p5": after15,
        }

        if given:
            raw_time = array["time"][finite].astype(np.float64, copy=False)
            curvature_ms = raw_time * 1.0e-3
            actual_s = curvature_ms / 1000.0
            add_values(true_values_s, actual_s)
            true_count_points += len(actual_s)
            if len(actual_s):
                value_min, value_max = float(np.min(actual_s)), float(np.max(actual_s))
                true_min = value_min if true_min is None else min(true_min, value_min)
                true_max = value_max if true_max is None else max(true_max, value_max)
                true_duration_values.append(value_max - value_min)
            record["true_actual_physical_offset_s"] = distribution(
                actual_s.tolist(), value_min if len(actual_s) else None,
                value_max if len(actual_s) else None, len(actual_s))
        else:
            finite_array = array[finite]
            offsets_ms = reconstruct_offsets_ms(finite_array)
            selected_positions = np.arange(len(finite_array), dtype=np.int64)[::3]
            selected_array = finite_array[selected_positions]
            selected_offsets_ms = offsets_ms[selected_positions]
            selected_mask = np.sqrt(
                selected_array["x"].astype(np.float64) ** 2 +
                selected_array["y"].astype(np.float64) ** 2 +
                selected_array["z"].astype(np.float64) ** 2) > 1.5
            selected_offsets_ms = selected_offsets_ms[selected_mask]
            add_values(false_values_ms, offsets_ms)
            add_values(false_selected_ms, selected_offsets_ms)
            false_count_points += len(offsets_ms)
            if len(offsets_ms):
                value_min, value_max = float(np.min(offsets_ms)), float(np.max(offsets_ms))
                false_min = value_min if false_min is None else min(false_min, value_min)
                false_max = value_max if false_max is None else max(false_max, value_max)
                false_duration_values.append(value_max - value_min)
            record["false_reconstructed_offset_ms"] = distribution(
                offsets_ms.tolist(), value_min if len(offsets_ms) else None,
                value_max if len(offsets_ms) else None, len(offsets_ms))
            record["false_selected_offset_ms_blind_1p5"] = distribution(
                selected_offsets_ms.tolist(),
                float(np.min(selected_offsets_ms)) if len(selected_offsets_ms) else None,
                float(np.max(selected_offsets_ms)) if len(selected_offsets_ms) else None,
                len(selected_offsets_ms))
        records.append(record)

    bag.close()
    true_frames = sum(branches)
    false_frames = len(branches) - true_frames

    def frame_metric_summary(metric_name: str) -> dict:
        values = [float(record[metric_name]["jaccard"]) for record in records]
        exact = [bool(record[metric_name]["exact_set_equal"]) for record in records]
        return {
            "jaccard": quantile_summary(values),
            "exact_equal_fraction": float(sum(exact) / len(exact)) if exact else None,
            "fraction_ge_0p99": float(sum(value >= 0.99 for value in values) / len(values)) if values else None,
            "fraction_ge_0p95": float(sum(value >= 0.95 for value in values) / len(values)) if values else None,
        }

    representatives = {}
    for index in sorted({0, len(records) // 2, len(records) - 1}):
        if 0 <= index < len(records):
            representatives[str(index)] = records[index]

    return {
        "name": name,
        "bag": str(path),
        "topic": topic,
        "frames_total": len(records),
        "expected_frames": int(expected),
        "N_raw_total": raw_total,
        "N_finite_total": finite_total,
        "N_nan_xyz_total": nan_total,
        "N_inf_xyz_total": inf_total,
        "given_offset_time": {
            "true_frames": true_frames,
            "false_frames": false_frames,
            "true_fraction": true_frames / len(records) if records else None,
            "false_fraction": false_frames / len(records) if records else None,
            "true_run": run_lengths(branches, True),
            "false_run": run_lengths(branches, False),
            "first_true_frame": next((i for i, value in enumerate(branches) if value), None),
            "last_true_frame": next((i for i in range(len(branches) - 1, -1, -1) if branches[i]), None),
            "first_false_frame": next((i for i, value in enumerate(branches) if not value), None),
            "last_false_frame": next((i for i in range(len(branches) - 1, -1, -1) if not branches[i]), None),
            "final_finite_point_time_s": quantile_summary(final_times),
            "final_finite_original_index": quantile_summary(final_indices),
        },
        "true_branch": {
            "curvature_scale": "raw seconds-scale field * 1e-3 = GEODE milliseconds",
            "actual_physical_offset_s": distribution(true_values_s, true_min, true_max, true_count_points),
            "intended_physical_offset_s": "raw point time, already seconds",
            "actual_to_intended_ratio": 1.0e-6,
            "scan_duration_actual_s": quantile_summary(true_duration_values),
            "physical_scan_duration_intended_s": "raw per-frame time span, approximately 0.1 s",
        },
        "false_branch": {
            "omega_l_deg_per_ms": 0.361 * 10.0,
            "reconstructed_offset_ms_all_finite": distribution(false_values_ms, false_min, false_max, false_count_points),
            "reconstructed_offset_ms_selected_blind_1p5": quantile_summary(false_selected_ms),
            "scan_duration_ms": quantile_summary(false_duration_values),
        },
        "raw_index_selection": {
            "before_blind": frame_metric_summary("native_vs_geode"),
            "post_blind_2p0": frame_metric_summary("native_vs_geode_blind_2p0"),
            "post_blind_1p5": frame_metric_summary("native_vs_geode_blind_1p5"),
        },
        "representatives": representatives,
        "frames": records,
    }


def main(argv=None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--stairs-bag", type=Path, required=True)
    parser.add_argument("--tunnel-bag", type=Path, required=True)
    parser.add_argument("--topic", default="/velodyne_points")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    result = {
        "schema_version": 1,
        "authority": {
            "geode_commit": "1f008a7249e36393a1752622de50660b77b5b7f4",
            "finite_then_stride": "ordered finite xyz original indices, then [::3]",
            "given_offset_time": "last finite point time > 0",
        },
        "synthetic": {},
        "stairs": audit_bag(args.stairs_bag, args.topic, "Stairs"),
        "tunnel2": audit_bag(args.tunnel_bag, args.topic, "Tunnel2"),
    }
    from prompt07r_semantics import run_synthetic_tests
    result["synthetic"] = run_synthetic_tests()
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n",
                        encoding="utf-8")
    print("PROMPT07R_INPUT_AUDIT_PASS frames=stairs:%d,tunnel2:%d" %
          (result["stairs"]["frames_total"], result["tunnel2"]["frames_total"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
