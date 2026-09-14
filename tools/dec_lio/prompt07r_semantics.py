#!/usr/bin/env python3
"""Pure GEODE finite-cloud/stride and Velodyne timing semantics for Prompt07R."""

from __future__ import annotations

import math
from typing import Iterable

import numpy as np


def finite_xyz(array: np.ndarray) -> np.ndarray:
    return (np.isfinite(array["x"]) & np.isfinite(array["y"]) &
            np.isfinite(array["z"]))


def native_indices(finite: np.ndarray, stride: int = 3) -> np.ndarray:
    """Indices selected by the existing raw-index Super seam."""
    return np.flatnonzero(finite)[0:0] if len(finite) == 0 else np.asarray(
        [index for index in range(0, len(finite), stride) if finite[index]],
        dtype=np.int64)


def geode_indices(finite: np.ndarray, stride: int = 3) -> np.ndarray:
    """Original indices selected after GEODE's finite-cloud compaction."""
    finite_indices = np.flatnonzero(finite)
    return finite_indices[::stride].astype(np.int64, copy=False)


def blind_indices(array: np.ndarray, indices: np.ndarray,
                  blind: float) -> np.ndarray:
    x = array["x"][indices].astype(np.float64, copy=False)
    y = array["y"][indices].astype(np.float64, copy=False)
    z = array["z"][indices].astype(np.float64, copy=False)
    d2 = x * x + y * y + z * z
    return indices[d2 > blind * blind]


def geode_given_offset_time(array: np.ndarray, finite: np.ndarray) -> dict:
    finite_indices = np.flatnonzero(finite)
    if len(finite_indices) == 0:
        return {
            "N_finite": 0,
            "final_finite_original_index": None,
            "final_finite_time_s": None,
            "given_offset_time": False,
        }
    index = int(finite_indices[-1])
    value = float(array["time"][index])
    return {
        "N_finite": int(len(finite_indices)),
        "final_finite_original_index": index,
        "final_finite_time_s": value,
        "given_offset_time": bool(value > 0.0),
    }


def reconstruct_offsets_ms(array: np.ndarray, scan_rate: float = 10.0,
                           n_scans: int = 16) -> np.ndarray:
    """Reproduce GEODE's non-given-offset Velodyne branch.

    This is the feature-disabled branch at preprocess.cpp:429-459.  The
    returned values are curvature in milliseconds, including the zero assigned
    to the first point observed on each ring.
    """
    omega_l = 0.361 * scan_rate
    period = 360.0 / omega_l
    is_first = np.ones(n_scans, dtype=bool)
    yaw_first = np.zeros(n_scans, dtype=np.float64)
    time_last = np.zeros(n_scans, dtype=np.float64)
    result = np.empty(len(array), dtype=np.float64)
    for i, point in enumerate(array):
        layer = int(point["ring"])
        if layer < 0 or layer >= n_scans:
            raise ValueError(f"ring {layer} outside GEODE scan_line={n_scans}")
        yaw = math.atan2(float(point["y"]), float(point["x"])) * 57.2957
        if is_first[layer]:
            yaw_first[layer] = yaw
            is_first[layer] = False
            result[i] = 0.0
            time_last[layer] = 0.0
            continue
        if yaw <= yaw_first[layer]:
            value = (yaw_first[layer] - yaw) / omega_l
        else:
            value = (yaw_first[layer] - yaw + 360.0) / omega_l
        if value < time_last[layer]:
            value += period
        result[i] = value
        time_last[layer] = value
    return result


def quantile_summary(values: Iterable[float], percentiles=(5, 50, 90, 95,
                                                            99)) -> dict:
    values = np.asarray(list(values), dtype=np.float64)
    values = values[np.isfinite(values)]
    if len(values) == 0:
        return {"count": 0, "min": None, "max": None,
                **{f"p{p}": None for p in percentiles}}
    return {
        "count": int(len(values)),
        "min": float(np.min(values)),
        "max": float(np.max(values)),
        **{f"p{p}": float(np.percentile(values, p)) for p in percentiles},
    }


def set_metrics(native: np.ndarray, geode: np.ndarray) -> dict:
    native_set, geode_set = set(native.tolist()), set(geode.tolist())
    intersection = len(native_set & geode_set)
    union = len(native_set | geode_set)
    symmetric_difference = len(native_set ^ geode_set)
    return {
        "native_count": len(native_set),
        "geode_count": len(geode_set),
        "intersection": intersection,
        "union": union,
        "symmetric_difference": symmetric_difference,
        "jaccard": float(intersection / union) if union else 1.0,
        "exact_set_equal": native_set == geode_set,
    }


def run_synthetic_tests() -> dict[str, bool]:
    finite = np.ones(8, dtype=bool)
    results = {}
    results["S1_no_nans_equal"] = np.array_equal(native_indices(finite),
                                                   geode_indices(finite))

    finite = np.ones(8, dtype=bool)
    finite[1] = False
    results["S2_nan_index_1_differs"] = not np.array_equal(
        native_indices(finite), geode_indices(finite))

    finite = np.ones(12, dtype=bool)
    finite[[1, 4, 9]] = False
    expected = np.flatnonzero(finite)[::3]
    results["S3_multiple_nans_exact_compaction"] = np.array_equal(
        geode_indices(finite), expected)

    finite = np.ones(7, dtype=bool)
    finite[0] = False
    results["S4_nan_first_exact_compaction"] = np.array_equal(
        geode_indices(finite), np.array([1, 4], dtype=np.int64))

    finite = np.ones(7, dtype=bool)
    finite[-1] = False
    results["S5_nan_last_exact_compaction"] = np.array_equal(
        geode_indices(finite), np.array([0, 3], dtype=np.int64))

    dtype = np.dtype([("x", "f8"), ("y", "f8"), ("z", "f8")])
    points = np.zeros(8, dtype=dtype)
    points[1]["x"] = 0.1
    finite = np.ones(8, dtype=bool)
    native = blind_indices(points, native_indices(finite), 2.0)
    geode = blind_indices(points, geode_indices(finite), 2.0)
    results["S6_blind_does_not_change_stride_phase"] = np.array_equal(
        native, geode)
    return results
