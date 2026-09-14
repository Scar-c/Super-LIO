#!/usr/bin/env python3
"""Prompt12R fixed-contract full-6DoF and shadow-diagnostic analysis."""

import argparse
import csv
import hashlib
import json
import math
import pathlib
import re
import statistics
import sys

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3

MAX_DIFF = 0.05
HORIZONS = (1.0, 5.0, 10.0)


def finite(values):
    return [float(value) for value in values if math.isfinite(float(value))]


def stats(values):
    values = np.asarray(finite(values), dtype=float)
    if values.size == 0:
        return {"count": 0, "rmse": None, "mean": None, "median": None,
                "p90": None, "p95": None, "max": None}
    return {
        "count": int(values.size),
        "rmse": float(np.sqrt(np.mean(values ** 2))),
        "mean": float(np.mean(values)),
        "median": float(np.median(values)),
        "p90": float(np.percentile(values, 90)),
        "p95": float(np.percentile(values, 95)),
        "max": float(np.max(values)),
    }


def rotation_matrix(quaternion):
    x, y, z, w = np.asarray(quaternion, dtype=float)
    norm = np.linalg.norm([x, y, z, w])
    if not math.isfinite(norm) or norm <= 0.0:
        raise ValueError("invalid quaternion")
    x, y, z, w = np.asarray([x, y, z, w]) / norm
    return np.asarray([
        [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
        [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
        [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
    ])


def so3_log(matrix):
    cosine = np.clip((np.trace(matrix) - 1.0) * 0.5, -1.0, 1.0)
    angle = float(np.arccos(cosine))
    vee = np.asarray([matrix[2, 1] - matrix[1, 2],
                      matrix[0, 2] - matrix[2, 0],
                      matrix[1, 0] - matrix[0, 1]])
    if angle < 1e-8:
        return 0.5 * vee
    sine = math.sin(angle)
    if abs(sine) < 1e-8:
        return angle * vee / max(np.linalg.norm(vee), 1e-12)
    return angle * vee / (2.0 * sine)


def nearest_index(times, target):
    index = int(np.searchsorted(times, target))
    candidates = [candidate for candidate in (index - 1, index) if 0 <= candidate < len(times)]
    if not candidates:
        return None
    return min(candidates, key=lambda candidate: abs(float(times[candidate] - target)))


def load_diagnostics(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise ValueError(f"empty diagnostics: {path}")
    return rows


def number(row, key):
    try:
        return float(row.get(key, "nan"))
    except (TypeError, ValueError):
        return math.nan


def numeric_stats(rows, key):
    return stats([number(row, key) for row in rows])


def raw_characterization(path):
    rows = load_diagnostics(path)
    valid = [row for row in rows if row.get("valid") == "1"]
    result = {
        "input": str(pathlib.Path(path).resolve()),
        "sha256": hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest(),
        "rows": len(rows),
        "valid_rows": len(valid),
        "invalid_rows": len(rows) - len(valid),
        "cond_R": numeric_stats(valid, "cond_rot"),
        "cond_t": numeric_stats(valid, "cond_trans"),
        "weak_rank_R": {},
        "weak_rank_t": {},
        "projector_angle_R": numeric_stats(valid, "principal_angle_max_rot"),
        "projector_angle_t": numeric_stats(valid, "principal_angle_max_trans"),
    }
    for output_key, input_key in (("weak_rank_R", "weak_rank_rot"),
                                  ("weak_rank_t", "weak_rank_trans")):
        values = [int(number(row, input_key)) for row in valid
                  if math.isfinite(number(row, input_key))]
        result[output_key] = {str(value): values.count(value) for value in sorted(set(values))}
    return result


def paired_characterization(path):
    rows = load_diagnostics(path)
    valid = [row for row in rows if row.get("control_valid") == "1"]
    active = [row for row in rows if row.get("control_applied") == "1"]
    return {
        "input": str(pathlib.Path(path).resolve()),
        "sha256": hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest(),
        "rows": len(rows),
        "valid_fraction": len(valid) / len(rows) if rows else 0.0,
        "active_fraction": len(active) / len(rows) if rows else 0.0,
        "gamma": numeric_stats(valid, "gamma_w"),
        "trace_ratio": numeric_stats(valid, "trace_ratio"),
        "cond_R": numeric_stats(valid, "cond_R"),
        "cond_t": numeric_stats(valid, "cond_t"),
        "weak_rank_R": {
            str(rank): sum(int(number(row, "weak_rank_R")) == rank for row in valid)
            for rank in sorted({int(number(row, "weak_rank_R")) for row in valid
                                if math.isfinite(number(row, "weak_rank_R"))})
        },
    }


def prepare_trajectory(path, ground_truth):
    et, ep, eq = load_tum(path)
    gt, gp, gq = load_tum(ground_truth, allow_unsorted=True)
    pairs = associate(et, gt, MAX_DIFF)
    if len(pairs) < 3:
        raise ValueError(f"insufficient matches: {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    alignment, translation = umeyama_se3(ep[ei], gp[gi])
    est_pos = (alignment @ ep[ei].T).T + translation
    est_rot = np.asarray([alignment @ rotation_matrix(eq[index]) for index in ei])
    gt_rot = np.asarray([rotation_matrix(gq[index]) for index in gi])
    return {
        "path": str(pathlib.Path(path).resolve()),
        "times": et[ei],
        "position": est_pos,
        "rotation": est_rot,
        "gt_position": gp[gi],
        "gt_rotation": gt_rot,
        "matched": len(pairs),
        "overlap_s": float(min(et[-1], gt[-1]) - max(et[0], gt[0])),
        "alignment_rotation": alignment,
        "alignment_translation": translation,
    }


def absolute_metrics(trajectory):
    position_error = np.linalg.norm(trajectory["position"] - trajectory["gt_position"], axis=1)
    rotation_error = []
    for estimate, truth in zip(trajectory["rotation"], trajectory["gt_rotation"]):
        rotation_error.append(np.degrees(np.linalg.norm(so3_log(truth.T @ estimate))))
    return {"translation": stats(position_error), "rotation_deg": stats(rotation_error)}


def interval_records(trajectory, horizon):
    times = trajectory["times"]
    records = []
    for start in range(len(times)):
        stop = nearest_index(times, times[start] + horizon)
        if stop is None or stop <= start or abs(float(times[stop] - times[start] - horizon)) > MAX_DIFF:
            continue
        est_translation = trajectory["rotation"][start].T @ (
            trajectory["position"][stop] - trajectory["position"][start])
        gt_translation = trajectory["gt_rotation"][start].T @ (
            trajectory["gt_position"][stop] - trajectory["gt_position"][start])
        est_relative = trajectory["rotation"][start].T @ trajectory["rotation"][stop]
        gt_relative = trajectory["gt_rotation"][start].T @ trajectory["gt_rotation"][stop]
        attitude_error = so3_log(gt_relative.T @ est_relative)
        records.append({
            "start_time": float(times[start]),
            "translation_error": float(np.linalg.norm(est_translation - gt_translation)),
            "rotation_error_deg": float(np.degrees(np.linalg.norm(attitude_error))),
            "attitude_error": attitude_error,
        })
    return records


def rpe_metrics(trajectory):
    result = {}
    for horizon in HORIZONS:
        records = interval_records(trajectory, horizon)
        result[str(int(horizon))] = {
            "intervals": len(records),
            "translation": stats([row["translation_error"] for row in records]),
            "rotation_deg": stats([row["rotation_error_deg"] for row in records]),
        }
    return result


def projector_index(rows):
    usable = []
    for row in rows:
        value = number(row, "timestamp")
        rank = number(row, "weak_rank_rot")
        entries = [number(row, f"P_weak_rot_{index}") for index in range(9)]
        if math.isfinite(value) and math.isfinite(rank) and all(math.isfinite(item) for item in entries):
            usable.append((value, int(rank), np.asarray(entries).reshape(3, 3)))
    return sorted(usable, key=lambda item: item[0])


def projector_at(index, timestamp):
    if not index:
        return None
    times = np.asarray([item[0] for item in index])
    position = nearest_index(times, timestamp)
    if position is None:
        return None
    value, rank, projector = index[position]
    if abs(value - timestamp) > 0.15:
        return None
    return rank, projector


def weak_reference(trajectory, diagnostics, n_reference=None):
    projector_rows = projector_index(load_diagnostics(diagnostics))
    output = {}
    for horizon in HORIZONS:
        records = interval_records(trajectory, horizon)
        weak, complement, fractions, ranks = [], [], [], []
        for row in records:
            selected = projector_at(projector_rows, row["start_time"])
            if selected is None:
                continue
            rank, projector = selected
            attitude = row["attitude_error"]
            weak_norm = float(np.linalg.norm(projector @ attitude))
            complement_norm = float(np.linalg.norm((np.eye(3) - projector) @ attitude))
            total = float(np.linalg.norm(attitude))
            weak.append(np.degrees(weak_norm))
            complement.append(np.degrees(complement_norm))
            fractions.append(weak_norm ** 2 / total ** 2 if total > 1e-12 else 0.0)
            ranks.append(rank)
        output[str(int(horizon))] = {
            "intervals": len(weak),
            "weak_error_deg": stats(weak),
            "complement_error_deg": stats(complement),
            "f_weak": stats(fractions),
            "rank_distribution": {str(rank): ranks.count(rank) for rank in sorted(set(ranks))},
        }
    return output


def normal_vs_weak(trajectory, diagnostics):
    projector_rows = projector_index(load_diagnostics(diagnostics))
    output = {}
    for horizon in (1.0, 5.0):
        records = interval_records(trajectory, horizon)
        grouped = {"rank_0": [], "rank_gt_0": []}
        for row in records:
            selected = projector_at(projector_rows, row["start_time"])
            if selected is None:
                continue
            rank, _ = selected
            grouped["rank_0" if rank == 0 else "rank_gt_0"].append(row)
        output[str(int(horizon))] = {
            name: {"intervals": len(values),
                   "translation": stats([item["translation_error"] for item in values]),
                   "rotation_deg": stats([item["rotation_error_deg"] for item in values])}
            for name, values in grouped.items()
        }
    return output


def overhead(path):
    text = pathlib.Path(path).read_text(encoding="utf-8")
    result = {"input": str(pathlib.Path(path).resolve())}
    for key in ("sensor_duration_s", "wall_processing_s", "speed_factor"):
        match = re.search(rf"{key}=([0-9.eE+-]+)", text)
        if match:
            result[key] = float(match.group(1))
    return result


def analyze(args):
    native = prepare_trajectory(args.native, args.ground_truth)
    p1 = prepare_trajectory(args.p1, args.ground_truth)
    native_absolute = absolute_metrics(native)
    p1_absolute = absolute_metrics(p1)

    def improvement(native_value, p1_value):
        if native_value in (None, 0.0):
            return None
        return 100.0 * (native_value - p1_value) / native_value

    result = {
        "contract": {
            "association_tolerance_s": MAX_DIFF,
            "alignment": "single global SE(3), no scale, no crop",
            "gt_time_offset_optimization": False,
        },
        "scene": args.scene,
        "native": {
            "trajectory": native["path"],
            "matched": native["matched"],
            "overlap_s": native["overlap_s"],
            "absolute": native_absolute,
            "rpe": rpe_metrics(native),
            "weak_reference": weak_reference(native, args.n_dcreg),
            "normal_vs_weak": normal_vs_weak(native, args.n_dcreg),
        },
        "p1": {
            "trajectory": p1["path"],
            "matched": p1["matched"],
            "overlap_s": p1["overlap_s"],
            "absolute": p1_absolute,
            "rpe": rpe_metrics(p1),
            "weak_reference": weak_reference(p1, args.n_dcreg),
            "normal_vs_weak": normal_vs_weak(p1, args.n_dcreg),
        },
        "raw_dcreg": raw_characterization(args.n_dcreg),
        "p1_exposure": paired_characterization(args.p1_paired),
        "p1_raw_dcreg": raw_characterization(args.p1_dcreg),
        "overhead": {"native": overhead(args.native_meta), "p1": overhead(args.p1_meta)},
    }
    result["improvement_percent"] = {
        "translation_rmse": improvement(
            native_absolute["translation"]["rmse"], p1_absolute["translation"]["rmse"]),
        "rotation_rmse": improvement(
            native_absolute["rotation_deg"]["rmse"], p1_absolute["rotation_deg"]["rmse"]),
    }
    return result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--scene", required=True)
    parser.add_argument("--ground-truth", required=True, type=pathlib.Path)
    parser.add_argument("--native", required=True, type=pathlib.Path)
    parser.add_argument("--p1", required=True, type=pathlib.Path)
    parser.add_argument("--n-dcreg", required=True, type=pathlib.Path)
    parser.add_argument("--p1-dcreg", required=True, type=pathlib.Path)
    parser.add_argument("--p1-paired", required=True, type=pathlib.Path)
    parser.add_argument("--native-meta", required=True, type=pathlib.Path)
    parser.add_argument("--p1-meta", required=True, type=pathlib.Path)
    parser.add_argument("--out", required=True, type=pathlib.Path)
    args = parser.parse_args(argv)
    result = analyze(args)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
