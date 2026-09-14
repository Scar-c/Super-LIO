#!/usr/bin/env python3
"""Prompt07R canonical evaluation and exact preprocessing comparison."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import sys
from collections import Counter

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio import prompt07_analysis as p07
from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3


ONSET = (1706584541.828, 1706584579.030)
SCENES = ("stairs", "tunnel2")
ARMS = ("N", "V", "B", "R", "A_old", "S", "A*")
FIELDS = ("weak_chi_max_R", "weak_psi_max_R", "Psi_weak_R", "A_weak_R",
          "C_L", "G_per_used")


def sha(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_ape(estimate: pathlib.Path, ground_truth: pathlib.Path) -> dict:
    et, ep, eq = load_tum(estimate)
    gt, gp, _ = load_tum(ground_truth, allow_unsorted=True)
    pairs = associate(et, gt, 0.10)
    if len(pairs) < 3:
        raise ValueError(f"insufficient canonical matches: {estimate} {len(pairs)}")
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    R, t = umeyama_se3(ep[ei], gp[gi])
    errors = np.linalg.norm((R @ ep[ei].T).T + t - gp[gi], axis=1)
    return {
        "matched": int(len(pairs)),
        "max_diff_s": 0.10,
        "alignment": "single global SE(3), no scale, no crop, no per-window realignment",
        "rmse_m": float(np.sqrt(np.mean(errors ** 2))),
        "mean_m": float(np.mean(errors)),
        "median_m": float(np.median(errors)),
        "p90_m": float(np.percentile(errors, 90)),
        "p95_m": float(np.percentile(errors, 95)),
        "max_m": float(np.max(errors)),
    }


def integer(row, field):
    try:
        value = float(row.get(field, ""))
    except (TypeError, ValueError):
        return -1
    return int(value) if math.isfinite(value) else -1


def num(row, field):
    try:
        return float(row.get(field, ""))
    except (TypeError, ValueError):
        return math.nan


def stat(values):
    values = np.asarray([float(value) for value in values if math.isfinite(float(value))],
                        dtype=float)
    if len(values) == 0:
        return {"n": 0, "median": None, "p90": None, "p95": None,
                "p99": None, "min": None, "max": None}
    return {"n": int(len(values)), "median": float(np.median(values)),
            "p90": float(np.percentile(values, 90)),
            "p95": float(np.percentile(values, 95)),
            "p99": float(np.percentile(values, 99)),
            "min": float(np.min(values)), "max": float(np.max(values))}


def load_csv(path):
    return p07.load_csv(path)


def row_by_frame(rows):
    return {integer(row, "frame"): row for row in rows}


def consecutive(values: list[bool], timestamps: list[float]) -> dict:
    best = {"length": 0, "start_index": None, "end_index": None,
            "duration_s": 0.0}
    start = None
    for index, value in enumerate(values + [False]):
        if value and start is None:
            start = index
        if not value and start is not None:
            end = index - 1
            duration = timestamps[end] - timestamps[start] if end >= start else 0.0
            if end - start + 1 > best["length"]:
                best = {"length": end - start + 1,
                        "start_index": start, "end_index": end,
                        "duration_s": float(duration)}
            start = None
    return best


def persistence(tunnel_rows, stairs_rows) -> dict:
    start, end = ONSET
    onset = sorted((row for row in tunnel_rows
                    if start <= num(row, "timestamp") <= end),
                   key=lambda row: num(row, "timestamp"))
    stairs_thresholds = {field: stat(num(row, field) for row in stairs_rows)["p95"]
                         for field in FIELDS}
    result = {"window": list(ONSET), "onset_rows": len(onset),
              "negative_control": "Stairs A* full-run P95", "fields": {}}
    timestamps = [num(row, "timestamp") for row in onset]
    for field in FIELDS:
        values = [num(row, field) for row in onset]
        threshold = stairs_thresholds[field]
        above = [math.isfinite(value) and value > threshold for value in values]
        result["fields"][field] = {
            "stairs_astar_p95": threshold,
            "onset": stat(values),
            "above_stairs_astar_p95_count": int(sum(above)),
            "above_stairs_astar_p95_fraction": float(sum(above) / len(above)) if above else None,
            "longest_exact_consecutive_run": consecutive(above, timestamps),
        }
    all_above = [all(math.isfinite(num(row, field)) and
                     num(row, field) > stairs_thresholds[field] for field in FIELDS)
                 for row in onset]
    result["all_fields_above_negative_control"] = {
        "fraction": float(sum(all_above) / len(all_above)) if all_above else None,
        "longest_exact_consecutive_run": consecutive(all_above, timestamps),
    }
    return result


def arm_specs(prompt07: pathlib.Path, prompt07r: pathlib.Path, scene: str) -> dict:
    old_scene = scene
    new_names = {"stairs": "S_stairs", "tunnel2": "S_tunnel2"}
    astar_names = {"stairs": "Astar_stairs_1", "tunnel2": "Astar_tunnel2_1"}
    return {
        "N": prompt07 / old_scene / "N",
        "V": prompt07 / old_scene / "V",
        "B": prompt07 / old_scene / "B",
        "R": prompt07 / old_scene / "R",
        "A_old": prompt07 / old_scene / "A1",
        "S": prompt07r / new_names[scene],
        "A*": prompt07r / astar_names[scene],
    }


def report_arm(path: pathlib.Path, scene: str, arm: str,
               ground_truth: pathlib.Path, native_axis, native_physical_rows) -> dict:
    stage = load_csv(path / "observation_stage.csv")
    d2 = load_csv(path / "d2_frame_summary.csv")
    consistency = load_csv(path / "consistency_frame_summary.csv")
    axis = load_csv(path / "axis_frame_summary.csv")
    dcreg = load_csv(path / "dcreg_shadow.csv")
    trajectory = path / "trajectory.tum"
    physical, physical_rows = p07.physical_axis(
        trajectory, ground_truth, path / "axis_frame_summary.csv", scene)
    consistency_report = p07.consistency_summary(consistency)
    consistency_report["Psi_weak_R"] = p07.frame_stat(physical_rows, "Psi_weak_R")
    consistency_report["A_weak_R"] = p07.frame_stat(physical_rows, "A_weak_R")
    if arm in {"S", "A*"}:
        semantics = "FINITE_COMPACTED"
    else:
        semantics = "RAW_STRIDE"
    return {
        "arm": arm, "path": str(path.resolve()),
        "finite_stride_semantics": semantics,
        "stage_rows": len(stage), "stage": p07.stage_summary(stage),
        "d2": p07.d2_summary(d2, dcreg, consistency),
        "consistency": consistency_report,
        "h": p07.h_summary(dcreg),
        "physical_axis": physical,
        "trajectory": canonical_ape(trajectory, ground_truth),
        "trajectory_sha256": sha(trajectory),
        "trajectory_rows": sum(1 for line in trajectory.read_text().splitlines()
                                if line.strip()),
        "axis_rows": len(axis), "d2_rows": len(d2), "dcreg_rows": len(dcreg),
        "N_vs_arm_projector": p07.matched_projectors(native_axis, axis),
        "full_H_trace_normalization": "FULL_H_SHAPE_NORMALIZATION_NOT_AVAILABLE; retained Schur normalized-spectrum analysis",
        "physical_rows": physical_rows,
    }


def compact_report(report):
    if isinstance(report, dict):
        return {key: compact_report(value) for key, value in report.items()
                if key != "physical_rows"}
    if isinstance(report, list):
        return [compact_report(value) for value in report]
    if isinstance(report, np.generic):
        return compact_report(report.item())
    if isinstance(report, float) and not math.isfinite(report):
        return None
    return report


def markdown_table(reports, scene):
    lines = [f"## {scene.title()}", "",
             "| arm | finite/stride | blind | voxel | upper range | N_after_voxel | N_used | lambda_R_min | kappa_R median/P95 | weak rank | O_P vs N | angle | O_yaw | O_course | C_L | G/N | APE RMSE @0.10s |",
             "|---|---|---:|---:|---|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|"]
    config = {
        "N": ("RAW_STRIDE", 2.0, .5, "150"),
        "V": ("RAW_STRIDE", 2.0, .3, "150"),
        "B": ("RAW_STRIDE", 1.5, .5, "150"),
        "R": ("RAW_STRIDE", 2.0, .5, "1000/no effect"),
        "A_old": ("RAW_STRIDE", 1.5, .3, "1000/no effect"),
        "S": ("FINITE_COMPACTED", 2.0, .5, "150"),
        "A*": ("FINITE_COMPACTED", 1.5, .3, "1000/no effect"),
    }
    for arm in ARMS:
        r = reports[arm]
        d, c, a = r["d2"], r["consistency"], r["physical_axis"]
        overlap = r["N_vs_arm_projector"]["overlap"]["median"]
        angle = r["N_vs_arm_projector"]["rank1_principal_angle_deg"]["median"]
        lines.append("| %s | %s | %.1f | %.1f | %s | %.1f | %.1f | %.6g | %.6g/%.6g | %s | %.6g | %.6g | %.6g | %.6g | %.6g | %.6g | %.6g |" % (
            arm, config[arm][0], config[arm][1], config[arm][2], config[arm][3],
            r["stage"]["N_after_voxel"]["median"] or 0,
            r["stage"]["N_used"]["median"] or 0,
            d["lambda_R_min"]["median"] or 0,
            d["kappa_R"]["median"] or 0, d["kappa_R"]["p95"] or 0,
            str(d["weak_rank_R_distribution"]), overlap or 0, angle or 0,
            a["O_yaw"]["median"] or 0, a["O_long"]["median"] or 0,
            c["C_L"]["median"] or 0, c["G_per_used"]["median"] or 0,
            r["trajectory"]["rmse_m"]))
    return "\n".join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--prompt07-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--prompt07r-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--audit", type=pathlib.Path, required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    parser.add_argument("--evidence-dir", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    gt = {"stairs": pathlib.Path("/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt"),
          "tunnel2": pathlib.Path("/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt")}
    combined = {"schema_version": 1, "association_max_diff_s": 0.10,
                "arms": ARMS, "scenes": {}, "input_audit": json.loads(
                    args.audit.read_text(encoding="utf-8"))}
    for scene in SCENES:
        specs = arm_specs(args.prompt07_runtime, args.prompt07r_runtime, scene)
        native_axis = load_csv(specs["N"] / "axis_frame_summary.csv")
        _, native_physical_rows = p07.physical_axis(
            specs["N"] / "trajectory.tum", gt[scene],
            specs["N"] / "axis_frame_summary.csv", scene)
        reports = {}
        for arm in ARMS:
            reports[arm] = report_arm(specs[arm], scene, arm, gt[scene],
                                      native_axis, native_physical_rows)
        combined["scenes"][scene] = reports

    stairs_rows = combined["scenes"]["stairs"]["A*"]["physical_rows"]
    tunnel_rows = combined["scenes"]["tunnel2"]["A*"]["physical_rows"]
    combined["tunnel2_astar_persistence"] = persistence(tunnel_rows, stairs_rows)
    combined["canonical_evaluator"] = {
        "association_max_diff_s": 0.10,
        "one_global_se3": True, "scale": "none", "crop": "none",
        "per_window_realign": False,
        "old_prompt07_0p05_superseded": True,
    }
    combined["classification"] = {
        "primary": "B — FINITE_STRIDE_ORDER_MATERIALLY_CHANGES_WEAK_SUBSPACE",
        "supporting": "The S stride-order-only isolation materially changes the Stairs weak-subspace geometry; the complete A* arm remains close to A_old in physical-axis interpretation.",
    }
    combined["d2_authority"] = {
        "designation": "D2-OBS-R2",
        "text": "Weak-subspace direction itself depends materially on preprocessing; magnitude and thresholds remain preprocessing-dependent.",
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(compact_report(combined), indent=2, sort_keys=True) + "\n",
                        encoding="utf-8")
    args.evidence_dir.mkdir(parents=True, exist_ok=True)
    tables = ["# Prompt07R exact alignment primary comparison", ""]
    for scene in SCENES:
        tables.append(markdown_table(combined["scenes"][scene], scene))
        tables.append("")
    (args.evidence_dir / "EXACT_ALIGNMENT_ARM_TABLES.md").write_text(
        "\n".join(tables).rstrip() + "\n", encoding="utf-8")
    print("PROMPT07R_ANALYSIS_PASS scenes=stairs,tunnel2 arms=" + ",".join(ARMS))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
