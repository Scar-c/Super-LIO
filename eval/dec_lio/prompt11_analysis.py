#!/usr/bin/env python3
"""Prompt11 fixed-contract trajectory and paired-control analysis."""

import argparse
import csv
import hashlib
import json
import math
import pathlib
import statistics
import sys

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3

MAX_DIFF = 0.10
TUNNEL_ONSET = (1706584541.828, 1706584579.030)


def stats(values):
    values = np.asarray([value for value in values if math.isfinite(value)], dtype=float)
    if values.size == 0:
        return {"count": 0, "rmse": None, "median": None, "p95": None, "max": None}
    return {"count": int(values.size), "rmse": float(np.sqrt(np.mean(values ** 2))),
            "median": float(np.median(values)), "p95": float(np.percentile(values, 95)),
            "max": float(np.max(values))}


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


def trajectory_report(estimate_path, ground_truth_path, position_only=False):
    est_time, est_pos, est_quat = load_tum(estimate_path)
    gt_time, gt_pos, gt_quat = load_tum(ground_truth_path, allow_unsorted=True)
    pairs = associate(est_time, gt_time, MAX_DIFF)
    if len(pairs) < 3:
        raise ValueError(f"insufficient matches: {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    align_rotation, align_translation = umeyama_se3(est_pos[ei], gt_pos[gi])
    aligned_pos = (align_rotation @ est_pos[ei].T).T + align_translation
    position_error = np.linalg.norm(aligned_pos - gt_pos[gi], axis=1)
    result = {
        "trajectory": str(pathlib.Path(estimate_path).resolve()),
        "matched": len(pairs),
        "overlap_s": float(min(est_time[-1], gt_time[-1]) - max(est_time[0], gt_time[0])),
        "translation": stats(position_error),
        "position_only": position_only,
    }
    if not position_only:
        est_rot = np.asarray([align_rotation @ rotation_matrix(est_quat[index]) for index in ei])
        gt_rot = np.asarray([rotation_matrix(gt_quat[index]) for index in gi])
        rotation_error = []
        for estimate, truth in zip(est_rot, gt_rot):
            relative = truth.T @ estimate
            cosine = np.clip((np.trace(relative) - 1.0) * 0.5, -1.0, 1.0)
            rotation_error.append(float(np.degrees(np.arccos(cosine))))
        result["rotation_deg"] = stats(rotation_error)
    else:
        result["rotation_deg"] = None
    return result, est_time[ei], position_error


def analyze_scene(paths, ground_truth, position_only):
    reports = {}
    local = {}
    for name, path in paths.items():
        report, timestamps, _ = trajectory_report(path, ground_truth, position_only)
        reports[name] = report
        if position_only:
            est_time, est_pos, _ = load_tum(path)
            gt_time, gt_pos, _ = load_tum(ground_truth, allow_unsorted=True)
            pairs = associate(est_time, gt_time, MAX_DIFF)
            ei = np.asarray([pair[0] for pair in pairs], dtype=int)
            gi = np.asarray([pair[1] for pair in pairs], dtype=int)
            rotation, translation = umeyama_se3(est_pos[ei], gt_pos[gi])
            aligned = (rotation @ est_pos[ei].T).T + translation
            errors = aligned - gt_pos[gi]
            start, end = TUNNEL_ONSET
            absolute = np.linalg.norm(errors, axis=1)
            local_onset, local_post = [], []
            for index, timestamp in enumerate(est_time[ei]):
                end_index = int(np.searchsorted(est_time[ei], timestamp + 5.0, side="left"))
                if end_index >= len(ei):
                    continue
                value = np.linalg.norm(errors[end_index] - errors[index])
                if start <= timestamp <= end:
                    local_onset.append(value)
                elif timestamp > end:
                    local_post.append(value)
            local[name] = {
                "onset_absolute": stats(absolute[(est_time[ei] >= start) & (est_time[ei] <= end)]),
                "post_absolute": stats(absolute[est_time[ei] > end]),
                "onset_local_5s": stats(local_onset),
                "post_local_5s": stats(local_post),
            }
    return {"reports": reports, "tunnel_local": local}


def audit_report(path):
    path = pathlib.Path(path)
    with path.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise ValueError(f"empty control audit: {path}")

    def numbers(key):
        return [float(row[key]) for row in rows if row.get(key, "") not in ("", "nan")]

    trace_raw = numbers("trace_raw_H")
    trace_p1 = numbers("trace_att_H")
    trace_control = numbers("trace_control_H")
    trace_ratio = numbers("trace_ratio")
    alpha = numbers("trace_control_ratio")
    gamma = numbers("gamma_w")
    active = [row.get("control_applied") == "1" for row in rows]
    valid = [row.get("control_valid") == "1" for row in rows]
    mismatch = [abs(control - p1) / max(abs(p1), 1.0)
                for control, p1 in zip(trace_control, trace_p1)]
    return {
        "rows": len(rows),
        "mode_values": sorted({int(row["mode"]) for row in rows}),
        "valid_fraction": float(sum(valid) / len(valid)),
        "active_fraction": float(sum(active) / len(active)),
        "trace_raw_median": float(np.median(trace_raw)),
        "trace_p1_median": float(np.median(trace_p1)),
        "trace_control_median": float(np.median(trace_control)),
        "trace_ratio_median": float(np.median(trace_ratio)),
        "trace_control_ratio_median": float(np.median(alpha)),
        "gamma_w_median": float(np.median(gamma)),
        "max_p1_vs_control_trace_relative_error": float(max(mismatch)),
        "audit_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
    }


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--ground-truth", required=True, type=pathlib.Path)
    parser.add_argument("--position-only", action="store_true")
    parser.add_argument("--out", required=True, type=pathlib.Path)
    parser.add_argument("--trajectory", action="append", nargs=2, metavar=("NAME", "PATH"), required=True)
    parser.add_argument("--audit", action="append", nargs=2, metavar=("NAME", "PATH"), required=True)
    args = parser.parse_args(argv)
    paths = {name: pathlib.Path(path) for name, path in args.trajectory}
    audits = {name: pathlib.Path(path) for name, path in args.audit}
    result = analyze_scene(paths, args.ground_truth, args.position_only)
    result["audits"] = {name: audit_report(path) for name, path in audits.items()}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
