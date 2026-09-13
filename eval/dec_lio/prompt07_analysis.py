#!/usr/bin/env python3
"""Prompt07 scan-aligned observation-construction audit.

This consumer compares only the declared Super-LIO observation arms.  It does
not modify estimator state, crop trajectories, or perform per-window pose
realignment.  Prompt06's offline course reconstruction is reused only as an
evaluation-side physical-axis diagnostic.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import pathlib
import statistics
import sys
from collections import Counter

import numpy as np

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from eval.dec_lio import prompt06_analysis as p06
from eval.dec_lio.eval_tum_translation import associate, load_tum, umeyama_se3


ONSET = (1706584541.828, 1706584579.030)
ARMS = ("N", "V", "B", "R", "A1", "A2")
PRIMARY_A = "A1"


def finite(values):
    return [float(value) for value in values
            if value is not None and math.isfinite(float(value))]


def stat(values):
    values = finite(values)
    if not values:
        return {"n": 0, "median": None, "p95": None, "min": None, "max": None}
    return {"n": len(values), "median": float(np.median(values)),
            "p95": float(np.percentile(values, 95)), "min": min(values),
            "max": max(values)}


def num(row, field):
    try:
        value = float(row.get(field, ""))
    except (TypeError, ValueError):
        return math.nan
    return value


def integer(row, field):
    value = num(row, field)
    return int(value) if math.isfinite(value) else -1


def load_csv(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def sha(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def vector(row, prefix, count):
    return np.asarray([num(row, f"{prefix}_{index}") for index in range(count)],
                      dtype=float)


def matrix(row, prefix, size=3):
    return vector(row, prefix, size * size).reshape(size, size)


def row_by_frame(rows):
    return {integer(row, "frame"): row for row in rows}


def frame_stat(rows, field):
    return stat([num(row, field) for row in rows])


def stage_summary(rows):
    fields = ("N_raw", "N_finite", "N_after_raw_stride", "N_after_blind",
              "N_after_upper_range", "N_undistorted", "N_after_voxel",
              "N_candidate", "N_used")
    return {field: frame_stat(rows, field) for field in fields}


def d2_summary(rows, dcreg_rows, consistency_rows):
    first = [row for row in dcreg_rows if integer(row, "iteration") == 0]
    result = {
        "rows": len(rows),
        "lambda_R_min": stat([min(vector(row, "lambda_rot", 3)) for row in first]),
        "lambda_t_min": stat([min(vector(row, "lambda_trans", 3)) for row in first]),
        "kappa_R": frame_stat(consistency_rows, "dcreg_schur_kappa_R"),
        "kappa_t": frame_stat(consistency_rows, "dcreg_schur_kappa_t"),
        "weak_rank_R_distribution": dict(Counter(integer(row, "weak_rank_R")
                                                    for row in consistency_rows)),
        "weak_rank_t_distribution": dict(Counter(integer(row, "weak_rank_t")
                                                    for row in consistency_rows)),
    }
    return result


def consistency_summary(rows):
    fields = ("C_L", "C_F", "G_per_used", "N_used", "weak_chi_max_R",
              "weak_psi_max_R", "Psi_weak_R", "A_weak_R")
    result = {field: frame_stat(rows, field) for field in fields}
    result["valid_fraction"] = (sum(integer(row, "valid") == 1 for row in rows) /
                                 len(rows) if rows else 0.0)
    return result


def h_summary(rows):
    # dcreg_shadow contains the first-native-iteration H_L/H_t eigenvalues in
    # the estimator's native 1000-weighted scale.  Ratios are scale-free.
    rows = [row for row in rows if integer(row, "iteration") == 0]
    out = {}
    for block, prefix in (("R", "lambda_rot"), ("t", "lambda_trans")):
        traces = [float(np.sum(vector(row, prefix, 3))) for row in rows]
        frobenius = [float(np.linalg.norm(vector(row, prefix, 3))) for row in rows]
        minimum = [float(np.min(vector(row, prefix, 3))) for row in rows]
        normalized = [(
            vector(row, "normalized_lambda_rot" if block == "R"
                   else "normalized_lambda_trans", 3)).tolist()
            for row in rows]
        out[block] = {"raw_trace": stat(traces), "raw_frobenius": stat(frobenius),
                      "raw_lambda_min": stat(minimum),
                      "shape_eigenvalues": normalized}
    return out


def projector_overlap(P, Q, rank_p, rank_q):
    if rank_p <= 0 or rank_q <= 0 or not np.isfinite(P).all() or not np.isfinite(Q).all():
        return None, None, None
    denominator = float(min(rank_p, rank_q))
    overlap = float(np.trace(P @ Q) / denominator)
    distance = float(np.linalg.norm(P - Q, ord="fro"))
    angle = None
    if rank_p == 1 and rank_q == 1:
        eigenvalues, eigenvectors = np.linalg.eigh(P)
        direction = eigenvectors[:, int(np.argmax(eigenvalues))]
        cosine_squared = float(direction.T @ Q @ direction)
        angle = float(np.degrees(np.arccos(np.sqrt(np.clip(cosine_squared, 0.0, 1.0)))))
    return overlap, distance, angle


def matched_projectors(native_rows, arm_rows):
    native = row_by_frame(native_rows)
    arm = row_by_frame(arm_rows)
    overlaps, distances, angles = [], [], []
    timestamp_deltas = []
    for frame, nrow in native.items():
        arow = arm.get(frame)
        if arow is None:
            continue
        timestamp_deltas.append(abs(num(nrow, "timestamp") - num(arow, "timestamp")))
        P = matrix(nrow, "weak_projector_R")
        Q = matrix(arow, "weak_projector_R")
        overlap, distance, angle = projector_overlap(
            P, Q, integer(nrow, "weak_rank_R"), integer(arow, "weak_rank_R"))
        if overlap is not None:
            overlaps.append(overlap)
            distances.append(distance)
        if angle is not None:
            angles.append(angle)
    return {"matched_frames": len(overlaps),
            "timestamp_delta_s": stat(timestamp_deltas),
            "overlap": stat(overlaps), "frobenius_distance": stat(distances),
            "rank1_principal_angle_deg": stat(angles),
            "formula": "tr(P_N P_A)/min(rank_N,rank_A); no eigenvector sign/component comparison"}


def ape(estimate, ground_truth):
    et, ep, eq = load_tum(estimate)
    gt, gp, _ = load_tum(ground_truth, allow_unsorted=True)
    pairs = associate(et, gt, 0.05)
    ei = np.asarray([pair[0] for pair in pairs], dtype=int)
    gi = np.asarray([pair[1] for pair in pairs], dtype=int)
    R, t = umeyama_se3(ep[ei], gp[gi])
    errors = np.linalg.norm((R @ ep[ei].T).T + t - gp[gi], axis=1)
    return {"matched": len(pairs), "alignment": "single global SE(3), no scale, no crop",
            "rmse_m": float(np.sqrt(np.mean(errors ** 2))),
            "mean_m": float(np.mean(errors)), "median_m": float(np.median(errors)),
            "p95_m": float(np.percentile(errors, 95)), "max_m": float(np.max(errors))}


def physical_axis(estimate, ground_truth, axis_path, scene):
    report = p06.analyze_scene(scene, pathlib.Path(estimate), pathlib.Path(ground_truth),
                               pathlib.Path(axis_path))
    rows = report["rows"]
    fields = ("O_yaw", "O_long", "O_lat")
    result = {field: frame_stat(rows, field) for field in fields}
    result["semantic_label"] = "COURSE_AXIS_ROTATION_LIKE"
    result["old_label_rejected"] = "LONGITUDINAL_ROLL_LIKE"
    result["forcing"] = report["forcing_robustness"]
    result["yaw_gate"] = report["yaw_gate"]
    result["course_proxy"] = report["course_proxy"]
    result["rows"] = len(rows)
    if scene == "tunnel2":
        result["onset"] = report["physical_axis"].get("onset", {})
    if scene == "stairs":
        result["rotation_error"] = report["stairs_orientation_control"]
    return result, rows


def onset_summary(rows, reference_rows):
    start, end = ONSET
    onset = [row for row in rows if start <= num(row, "timestamp") <= end]
    pre5 = [row for row in rows if start - 5.0 <= num(row, "timestamp") < start]
    result = {"window": list(ONSET), "rows": len(onset),
              "reference_p95_from_stairs_or_native": {}}
    for field in ("weak_chi_max_R", "weak_psi_max_R", "Psi_weak_R", "A_weak_R",
                  "C_L", "G_per_used"):
        result[field] = {"onset": frame_stat(onset, field),
                         "pre5": frame_stat(pre5, field),
                         "above_pre5_p95": sum(
                             num(row, field) > frame_stat(pre5, field)["p95"]
                             for row in onset if math.isfinite(num(row, field)))}
        result["reference_p95_from_stairs_or_native"][field] = frame_stat(reference_rows, field)["p95"]
    return result


def scene_arm(runtime, scene, arm, gt, native_rows, reference_rows):
    directory = runtime / scene / arm
    stage = load_csv(directory / "observation_stage.csv")
    d2 = load_csv(directory / "d2_frame_summary.csv")
    consistency = load_csv(directory / "consistency_frame_summary.csv")
    axis = load_csv(directory / "axis_frame_summary.csv")
    dcreg = load_csv(directory / "dcreg_shadow.csv")
    traj = directory / "trajectory.tum"
    physical, physical_rows = physical_axis(traj, gt, directory / "axis_frame_summary.csv", scene)
    consistency_report = consistency_summary(consistency)
    # Psi/A are Prompt06 physical-axis quantities carried by the merged
    # evaluation rows, while C_L/G/N/chi/psi are native consistency fields.
    consistency_report["Psi_weak_R"] = frame_stat(physical_rows, "Psi_weak_R")
    consistency_report["A_weak_R"] = frame_stat(physical_rows, "A_weak_R")
    result = {
        "arm": arm, "directory": str(directory.resolve()),
        "stage_rows": len(stage), "stage": stage_summary(stage),
        "d2": d2_summary(d2, dcreg, consistency), "consistency": consistency_report,
        "h": h_summary(dcreg), "physical_axis": physical,
        "trajectory": ape(traj, gt),
        "trajectory_sha256": sha(traj),
        "trajectory_rows": sum(1 for line in traj.read_text().splitlines() if line.strip()),
        "axis_rows": len(axis), "d2_rows": len(d2),
        "N_vs_arm_projector": matched_projectors(native_rows, axis),
    }
    if scene == "tunnel2":
        result["aligned_onset"] = onset_summary(physical_rows, reference_rows)
    return result, consistency, physical_rows


def ratios(native, aligned):
    output = {}
    metrics = {
        "N_used": ("stage", "N_used"),
        "lambda_R_min": ("d2", "lambda_R_min"),
        "C_L": ("consistency", "C_L"),
        "weak_chi_R": ("consistency", "weak_chi_max_R"),
        "Psi_weak_R": ("consistency", "Psi_weak_R"),
    }
    for name, (group, field) in metrics.items():
        n = native[group][field]
        a = aligned[group][field]
        output[name] = {
            "median_A_over_N": (
                a["median"] / n["median"]
                if n.get("median") not in (None, 0) and a.get("median") is not None
                else None),
            "p95_A_over_N": (
                a["p95"] / n["p95"]
                if n.get("p95") not in (None, 0) and a.get("p95") is not None
                else None),
        }
    return output


def clean(value):
    if isinstance(value, dict):
        return {key: clean(item) for key, item in value.items()}
    if isinstance(value, list):
        return [clean(item) for item in value]
    if isinstance(value, np.generic):
        return clean(value.item())
    if isinstance(value, float) and not math.isfinite(value):
        return None
    return value


def write(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.rstrip() + "\n", encoding="utf-8")


def table(scene_reports):
    lines = ["| arm | blind | stride | voxel | upper cutoff | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |",
             "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|"]
    config = {"N": (2.0, 3, 0.5, 150), "V": (2.0, 3, 0.3, 150),
              "B": (1.5, 3, 0.5, 150), "R": (2.0, 3, 0.5, "none"),
              "A1": (1.5, 3, 0.3, "none"), "A2": (1.5, 3, 0.3, "none")}
    for arm, report in scene_reports.items():
        s, d, c, a = report["stage"], report["d2"], report["consistency"], report["physical_axis"]
        overlap = report["N_vs_arm_projector"]["overlap"]["median"]
        angle = report["N_vs_arm_projector"]["rank1_principal_angle_deg"]["median"]
        kr = d["kappa_R"]
        kt = d["kappa_t"]
        lines.append("| %s | %g | %d | %g | %s | %.3f | %.3f | %.6g | %.6g | %.6g / %.6g | %.6g / %.6g | %s | %s | %s | %.6g | %.6g | %.6g | %.6g | %.6g |" % (
            arm, config[arm][0], config[arm][1], config[arm][2], config[arm][3],
            s["N_after_voxel"]["median"] or 0, s["N_used"]["median"] or 0,
            d["lambda_R_min"]["median"] or 0, d["lambda_t_min"]["median"] or 0,
            kr["median"] or 0, kr["p95"] or 0, kt["median"] or 0, kt["p95"] or 0,
            str(report["d2"]["weak_rank_R_distribution"]),
            "%.6g" % overlap if overlap is not None else "NA",
            "%.6g" % angle if angle is not None else "NA",
            a["O_yaw"]["median"] or 0, a["O_long"]["median"] or 0,
            c["C_L"]["median"] or 0, c["G_per_used"]["median"] or 0,
            report["trajectory"]["rmse_m"]))
    return "\n".join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--runtime", type=pathlib.Path, required=True)
    parser.add_argument("--audit", type=pathlib.Path, required=True)
    parser.add_argument("--out-dir", type=pathlib.Path, required=True)
    parser.add_argument("--combined-out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    gt = {"stairs": pathlib.Path("/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt"),
          "tunnel2": pathlib.Path("/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt")}
    reports = {}
    consistency_rows = {}
    physical_rows = {}
    for scene in ("stairs", "tunnel2"):
        native_axis = load_csv(args.runtime / scene / "N" / "axis_frame_summary.csv")
        _, native_physical_rows = physical_axis(
            args.runtime / scene / "N" / "trajectory.tum", gt[scene],
            args.runtime / scene / "N" / "axis_frame_summary.csv", scene)
        scene_reports = {}
        for arm in ARMS:
            report, consistency, axis = scene_arm(
                args.runtime, scene, arm, gt[scene], native_axis, native_physical_rows)
            scene_reports[arm] = report
            consistency_rows[(scene, arm)] = consistency
            physical_rows[(scene, arm)] = axis
        reports[scene] = scene_reports
        for arm in ARMS:
            scene_reports[arm]["threshold_shift_vs_N"] = ratios(
                scene_reports["N"], scene_reports[arm])

    audit = json.loads(args.audit.read_text(encoding="utf-8"))
    combined = {"schema_version": 1, "analysis": "Prompt07 GEODE observation-semantics authority and scan-aligned ablation",
                "onset": ONSET, "arms": ARMS, "scenes": reports, "input_audit": audit}
    args.combined_out.parent.mkdir(parents=True, exist_ok=True)
    args.combined_out.write_text(json.dumps(clean(combined), indent=2, sort_keys=True) + "\n",
                                 encoding="utf-8")

    evidence = args.out_dir
    write(evidence / "OBSERVATION_ARM_TABLES.md",
          "# Prompt07 arm tables\n\n## Stairs\n\n" + table(reports["stairs"]) +
          "\n\n## Tunnel2\n\n" + table(reports["tunnel2"]))
    for scene in ("stairs", "tunnel2"):
        for arm in ARMS:
            path = evidence / f"{scene}_{arm}_SUMMARY.json"
            path.write_text(json.dumps(clean(reports[scene][arm]), indent=2, sort_keys=True) + "\n",
                            encoding="utf-8")
    print(f"PROMPT07_ANALYSIS_PASS scenes=stairs,tunnel2 arms={','.join(ARMS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
