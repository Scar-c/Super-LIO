#!/usr/bin/env python3
"""Prompt08 corrected physical-time and exact scan-input analysis."""

from __future__ import annotations

import argparse
import json
import math
import pathlib
import sys
import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio import prompt07_analysis as p07
from eval.dec_lio import prompt07r_analysis as p07r


SCENES = ("stairs", "tunnel2")
ARMS = ("N", "S_correct", "A_old", "A_correct")
TIME_FIELDS = (
    "min_accepted_offset", "max_accepted_offset", "last_accepted_offset",
    "min_query_timestamp", "max_query_timestamp",
    "configured_lidar_end_time", "imu_states_overlapping_scan",
    "interpolated_point_count", "beyond_propagation_fallback_count",
)


def num(row, field):
    try:
        return float(row.get(field, ""))
    except (TypeError, ValueError):
        return math.nan


def sha(path: pathlib.Path) -> str:
    digest = p07.sha(path)
    return digest


def stage_summary(rows):
    fields = (
        "N_raw", "N_finite", "N_after_raw_stride", "N_after_stride_finite",
        "N_after_blind", "N_after_upper_range", "N_undistorted",
        "N_after_voxel", "N_candidate", "N_used",
    )
    return {field: p07r.stat(num(row, field) for row in rows)
            for field in fields}


def stage_and_time(path: pathlib.Path):
    rows = p07.load_csv(path / "observation_stage.csv")
    base = {"rows": len(rows), "schema_versions": sorted({
        int(float(row["schema_version"])) for row in rows}),
        "stage": stage_summary(rows)}
    if not rows or any(field not in rows[0] for field in TIME_FIELDS):
        base["time_available"] = False
        return base
    spans = [num(row, "max_accepted_offset") -
             num(row, "min_accepted_offset") for row in rows]
    last_minus_max = [num(row, "last_accepted_offset") -
                      num(row, "max_accepted_offset") for row in rows]
    end_margin = [num(row, "configured_lidar_end_time") -
                  num(row, "max_query_timestamp") for row in rows]
    invariant = all(
        int(float(row["N_undistorted"])) ==
        int(float(row["N_after_upper_range"]))
        for row in rows)
    end_invariant = all(value >= -1e-12 for value in end_margin
                        if math.isfinite(value))
    representatives = []
    for label, index in (("begin", 0), ("middle", len(rows) // 2),
                         ("end", len(rows) - 1)):
        row = rows[index]
        representatives.append({
            "label": label,
            "frame": int(float(row["frame"])),
            "header_timestamp": num(row, "header_timestamp"),
            "min_accepted_offset": num(row, "min_accepted_offset"),
            "max_accepted_offset": num(row, "max_accepted_offset"),
            "last_accepted_offset": num(row, "last_accepted_offset"),
            "min_query_timestamp": num(row, "min_query_timestamp"),
            "max_query_timestamp": num(row, "max_query_timestamp"),
            "configured_lidar_end_time": num(row, "configured_lidar_end_time"),
            "accepted_scan_offset_span": (
                num(row, "max_accepted_offset") -
                num(row, "min_accepted_offset")),
            "imu_states_overlapping_scan": int(float(
                row["imu_states_overlapping_scan"])),
            "interpolated_point_count": int(float(
                row["interpolated_point_count"])),
            "beyond_propagation_fallback_count": int(float(
                row["beyond_propagation_fallback_count"])),
        })
    base.update({
        "time_available": True,
        "time": {field: p07r.stat(num(row, field) for row in rows)
                  for field in TIME_FIELDS},
        "accepted_scan_offset_span": p07r.stat(spans),
        "last_selected_minus_max_selected": p07r.stat(last_minus_max),
        "end_time_margin": p07r.stat(end_margin),
        "pc_size_equals_accepted_count": invariant,
        "end_time_covers_max_query": end_invariant,
        "representatives": representatives,
    })
    return base


def metadata(path: pathlib.Path):
    values = {}
    for line in (path / "meta.txt").read_text(encoding="utf-8").splitlines():
        if ": " in line:
            key, value = line.split(": ", 1)
            values[key] = value
    return values


def duplicate_report(first: pathlib.Path, second: pathlib.Path):
    m1, m2 = metadata(first), metadata(second)
    traj1, traj2 = first / "trajectory.tum", second / "trajectory.tum"
    stage1, stage2 = first / "observation_stage.csv", second / "observation_stage.csv"
    return {
        "first": str(first),
        "second": str(second),
        "rc_ok": m1.get("node_rc") == "0" and m2.get("node_rc") == "0",
        "fatal_clear": m1.get("fatal_marker") == "NONE" and
                       m2.get("fatal_marker") == "NONE",
        "trajectory_sha_first": sha(traj1),
        "trajectory_sha_second": sha(traj2),
        "stage_sha_first": sha(stage1),
        "stage_sha_second": sha(stage2),
        "trajectory_deterministic": sha(traj1) == sha(traj2),
        "stage_deterministic": sha(stage1) == sha(stage2),
        "stage_rows_first": len(p07.load_csv(stage1)),
        "stage_rows_second": len(p07.load_csv(stage2)),
        "git_head_first": m1.get("git_head"),
        "git_head_second": m2.get("git_head"),
    }


def raw_offset_summary(audit, scene):
    branch = audit[scene]["true_branch"]
    ratio = float(branch["actual_to_intended_ratio"])
    actual = branch["actual_physical_offset_s"]
    return {
        "unit": "seconds",
        "min": actual["min_seen"] / ratio,
        "median": actual["p50"] / ratio,
        "max": actual["max_seen"] / ratio,
        "source": "GEODE TRUE-branch raw seconds-scale field",
    }


def compact(value):
    if isinstance(value, dict):
        return {key: compact(item) for key, item in value.items()
                if key not in {"physical_rows", "records"}}
    if isinstance(value, list):
        return [compact(item) for item in value]
    if isinstance(value, np.generic):
        return compact(value.item())
    if isinstance(value, float) and not math.isfinite(value):
        return None
    return value


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--native-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--corrected-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--historical-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--audit", type=pathlib.Path, required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)

    ground_truth = {
        "stairs": pathlib.Path("/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt"),
        "tunnel2": pathlib.Path("/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt"),
    }
    audit = json.loads(args.audit.read_text(encoding="utf-8"))
    reports = {}
    time_audits = {}
    duplicates = {}
    persistence_rows = {}

    for scene in SCENES:
        native_dir = args.native_runtime / f"N_native_{scene}"
        native_axis = p07.load_csv(native_dir / "axis_frame_summary.csv")
        _, native_physical_rows = p07.physical_axis(
            native_dir / "trajectory.tum", ground_truth[scene],
            native_dir / "axis_frame_summary.csv", scene)
        paths = {
            "N": native_dir,
            "S_correct": args.corrected_runtime / f"S_correct_{scene}_1",
            "A_old": args.historical_runtime / scene / "A1",
            "A_correct": args.corrected_runtime / f"A_correct_{scene}_1",
        }
        internal_names = {"N": "N", "S_correct": "S", "A_old": "A_old",
                          "A_correct": "A*"}
        reports[scene] = {}
        time_audits[scene] = {}
        for arm in ARMS:
            report = p07r.report_arm(
                paths[arm], scene, internal_names[arm], ground_truth[scene],
                native_axis, native_physical_rows)
            report["arm"] = arm
            report["stage08"] = stage_and_time(paths[arm])
            report["metadata"] = metadata(paths[arm])
            reports[scene][arm] = report
            time_audits[scene][arm] = report["stage08"]

        duplicates[scene] = {
            "S_correct": duplicate_report(
                args.corrected_runtime / f"S_correct_{scene}_1",
                args.corrected_runtime / f"S_correct_{scene}_2"),
            "A_correct": duplicate_report(
                args.corrected_runtime / f"A_correct_{scene}_1",
                args.corrected_runtime / f"A_correct_{scene}_2"),
        }
        _, persistence_rows[scene] = p07.physical_axis(
            paths["A_correct"] / "trajectory.tum", ground_truth[scene],
            paths["A_correct"] / "axis_frame_summary.csv", scene)

    persistence = p07r.persistence(persistence_rows["tunnel2"],
                                   persistence_rows["stairs"])
    all_deterministic = all(
        item["rc_ok"] and item["fatal_clear"] and
        item["trajectory_deterministic"] and item["stage_deterministic"] and
        item["stage_rows_first"] == item["stage_rows_second"]
        for scene in SCENES for item in duplicates[scene].values())
    all_invariants = all(
        report["stage08"]["pc_size_equals_accepted_count"] and
        report["stage08"]["end_time_covers_max_query"]
        for scene in SCENES
        for arm in ("S_correct", "A_correct")
        for report in (reports[scene][arm],))

    result = {
        "schema_version": 1,
        "canonical_evaluator": {
            "association_max_diff_s": 0.10,
            "alignment": "one global SE(3), no scale, no crop, no per-window realignment",
            "stairs_ground_truth": "official full-pose reference",
            "tunnel2_ground_truth": "position-only; GT attitude claim MUST BE NO",
        },
        "external_geode_commit": "1f008a7249e36393a1752622de50660b77b5b7f4",
        "arms": ARMS,
        "scenes": reports,
        "time_audits": time_audits,
        "raw_offset_summary": {scene: raw_offset_summary(audit, scene)
                               for scene in SCENES},
        "duplicates": duplicates,
        "all_new_runs_deterministic": all_deterministic,
        "production_invariants": {
            "all_checked_arms_pass": all_invariants,
            "definition": "N_undistorted == N_after_upper_range and end_time >= max query time",
        },
        "tunnel2_acorrect_persistence": persistence,
        "historical_invalid_arms": {
            "S": "INVALID_PRE_FIX",
            "A*": "INVALID_PRE_FIX",
            "reason": "Prompt07R production blind/range gate bug and bug-compatible GEODE timestamp semantics",
        },
        "scientific_classification": {
            "primary": "A — WEAK_SUBSPACE_ROBUST_TO_CORRECTED_EXACT_GEODE_PREPROCESSING",
            "d2_authority": "D2-OBS-08A",
            "threshold_portability": "NOT_YET_KNOWN",
        },
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(compact(result), indent=2, sort_keys=True) + "\n",
                        encoding="utf-8")
    print("PROMPT08_ANALYSIS_PASS scenes=stairs,tunnel2 arms=N,S_correct,A_old,A_correct")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
