#!/usr/bin/env python3
"""Canonical Visual Eval Scorecard builder (Phase A.1 corrected).

Metric semantics are defined in docs/super_livo/visual_eval_metric_dictionary.md.
No semantics inferred from field names; every metric has a source producer +
aggregation rule. full_lidar_observe_count comes from the actual geometry
update event count (Round11X cadence line), NEVER from used_once (a geometry
point ownership count).

Usage:
  visual_eval_score.py --stage <id> --run-dir <run> --manifest <yaml>
      [--trajectory x.tum] [--gt leica.csv] [--ate-evaluator ntu_viral_official_ate.py]
"""

import argparse
import json
import math
import pathlib
import re
import subprocess
import sys

import yaml

REPO = pathlib.Path(__file__).resolve().parents[3]


def _int(text, pattern):
    m = re.search(pattern, text)
    return int(m.group(1)) if m else None


def _float(text, pattern):
    m = re.search(pattern, text)
    return float(m.group(1)) if m else None


def _pct_of(vectors, label):
    return {k: v for k, v in vectors.items() if label in k}


def build_scorecard(stage, run_dir, manifest, trajectory=None, gt=None,
                    ate_evaluator=None):
    run = pathlib.Path(run_dir)
    node_log = run / "node_stdout.log"
    text = node_log.read_text(errors="replace") if node_log.exists() else ""

    manifest_data = {}
    if manifest and pathlib.Path(manifest).exists():
        manifest_data = yaml.safe_load(pathlib.Path(manifest).read_text()) or {}

    not_impl = "NOT_IMPLEMENTED"
    score = {
        "provenance": {
            "git_sha": manifest_data.get("production_revision"),
            "stage_id": stage,
            "parent_stage": "A0_D_LEGACY_PLACEMENT_SHADOW" if stage.startswith("A2")
                           else ("-" if stage.endswith("_BASE") or stage.endswith("_LEGACY") else "A2_D_CAMERA_EPOCH_SHADOW"),
            "dataset": manifest_data.get("dataset"),
            "sequence": manifest_data.get("sequence"),
            "config_path": manifest_data.get("config_provenance", {}).get("dataset_calibration"),
            "config_hash": None,
            "semantic_profile": manifest_data.get("semantic_profile"),
            "visual_map_policy": "S3_SPATIAL_BALANCED",
            "normalize_semantics": not_impl,
            "exposure_semantics": not_impl,
            "normal_semantics": not_impl,
            "patch_semantics": "CURRENT",
            "residual_semantics": "CURRENT",
            "iteration_semantics": "CURRENT",
            "result_path": str(run),
        },
        "completion": {
            "experiment_valid": None,
            "cleanup_verified": None,
            "trajectory_rows": None,
            "expected_or_reference_rows": None,
            "completion_ratio": None,
            "camera_received": None,
            "camera_processable": None,
            "camera_stale": None,
            "camera_eof": None,
            "raw_lidar_scans": None,
            "processable_raw_lidar_scans": None,
            "full_lidar_observe_count": None,
            "duplicate_full_lidar_observe_count": None,
            "geometry_points_used_once": None,
            "geometry_points_duplicate": None,
            "geometry_points_never_used": None,
        },
        "measurement_counts": {},
        "residual_density_per_frame": {},
        "ratios": {},
        "event_placement": {},
        "timestamp": {},
        "information": {},
        "spatial_coverage": {"status": "NOT_AVAILABLE_CURRENT_INSTRUMENTATION",
                             "reason": "per-frame landmark/voxel/grid occupancy requires intrusive map traversal; deferred to Phase E/F map ablation"},
        "compute": {},
        "accuracy": {},
    }

    # completion / validity (supervisor state at the run root)
    st = run / "state.json"
    if not st.exists() and run.parent.exists():
        st = run.parent / "state.json"
    if st.exists():
        data = json.loads(st.read_text())
        score["completion"]["experiment_valid"] = (
            data.get("state") == "SUCCESS" and data.get("cleanup_verified") is True)
        score["completion"]["cleanup_verified"] = data.get("cleanup_verified") is True
    if trajectory and pathlib.Path(trajectory).exists():
        score["completion"]["trajectory_rows"] = sum(1 for _ in open(trajectory))

    # camera / lidar accounting
    score["completion"]["camera_received"] = _int(
        text, r"images read/dispatched/skipped:\s*(\d+)/")
    score["completion"]["camera_processable"] = _int(
        text, r"S-0 camera-epoch: epochs=(\d+)")
    score["completion"]["camera_stale"] = _int(
        text, r"stale_image_drop=(\d+)")
    # full LiDAR Observe = actual geometry update count (Round11X cadence)
    score["completion"]["raw_lidar_scans"] = _int(
        text, r"cadence: policy=\S+ raw_scans=(\d+)")
    score["completion"]["processable_raw_lidar_scans"] = _int(
        text, r"cadence: policy=\S+ raw_scans=(\d+)")
    score["completion"]["full_lidar_observe_count"] = _int(
        text, r"cadence: policy=\S+ raw_scans=\d+ geometry_updates=(\d+)")
    # duplicate full Observe = observes beyond the processable scan count
    # (mechanically zero when updates-per-scan <= 1, as the cadence shows)
    obs = score["completion"]["full_lidar_observe_count"]
    proc = score["completion"]["processable_raw_lidar_scans"]
    score["completion"]["duplicate_full_lidar_observe_count"] = (
        max(0, obs - proc) if obs is not None and proc is not None else None)
    # geometry point ownership (separate fields; NEVER the observe count)
    score["completion"]["geometry_points_used_once"] = _int(
        text, r"fullscan ownership:.* used_once=(\d+)")
    score["completion"]["geometry_points_duplicate"] = _int(
        text, r"fullscan ownership:.* duplicate_use=(\d+)")
    score["completion"]["geometry_points_never_used"] = _int(
        text, r"fullscan ownership:.* never_used=(\d+)")

    rows = score["completion"]["trajectory_rows"]
    score["completion"]["expected_or_reference_rows"] = 3981  # canonical A0 rows
    if rows:
        score["completion"]["completion_ratio"] = rows / 3981.0

    # measurement counts
    score["measurement_counts"]["visual_query_attempts_total"] = _int(
        text, r"VISUAL_MEASUREMENT query: attempts=(\d+)")
    score["measurement_counts"]["visual_query_hits_total"] = _int(
        text, r"VISUAL_MEASUREMENT query:.* hits=(\d+)")
    score["measurement_counts"]["candidate_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* candidates=(\d+)")
    score["measurement_counts"]["valid_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* valid=(\d+)")
    score["measurement_counts"]["rejected_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* rejected=(\d+)")
    score["measurement_counts"]["visual_measured_frames"] = _int(
        text, r"VISUAL_MEASUREMENT observation: frames=(\d+)")
    score["measurement_counts"]["nonzero_H_frames"] = _int(
        text, r"VISUAL_MEASUREMENT H:.* nonzero=(\d+)")
    score["measurement_counts"]["nonzero_b_frames"] = _int(
        text, r"VISUAL_MEASUREMENT b:.* nonzero=(\d+)")
    score["measurement_counts"]["residual_samples_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* residual_samples=(\d+)")

    # residual density per frame (R14 per-frame vector percentiles)
    rpf = text
    for lab, pat in [
        ("P10", r"R14 residuals_per_frame P10=([-0-9.e+]+)"),
        ("P50", r"R14 residuals_per_frame P10=[-0-9.e+]+ P50=([-0-9.e+]+)"),
        ("P90", r"R14 residuals_per_frame P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=([-0-9.e+]+)"),
        ("P99", r"R14 residuals_per_frame P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=[-0-9.e+]+ P99=([-0-9.e+]+)"),
    ]:
        score["residual_density_per_frame"][lab] = _float(rpf, pat)
    tot = score["measurement_counts"]["residual_samples_total"]
    frames = score["measurement_counts"]["visual_measured_frames"]
    if tot is not None and frames:
        score["residual_density_per_frame"]["mean"] = tot / frames

    # ratios (explicit denominators; div-by-zero -> None)
    qa = score["measurement_counts"]["visual_query_attempts_total"]
    qh = score["measurement_counts"]["visual_query_hits_total"]
    vo = score["measurement_counts"]["valid_observations_total"]
    co = score["measurement_counts"]["candidate_observations_total"]
    cp = score["completion"]["camera_processable"]
    score["ratios"]["query_hit_ratio"] = (qh / qa) if qa else None
    score["ratios"]["valid_observation_ratio"] = (vo / co) if co else None
    score["ratios"]["measured_camera_ratio"] = (frames / cp) if (cp and frames) else None

    # event placement
    score["event_placement"]["camera_event_visual_count"] = _int(
        text, r"R14 camera-epoch Visual: executions=(\d+)")
    score["event_placement"]["lidar_callback_visual_count"] = _int(
        text, r"R14 LiDAR-callback Visual: executions=(\d+)")
    score["event_placement"]["duplicate_visual_event_count"] = _int(
        text, r"R14 duplicate Visual: count=(\d+)")
    score["event_placement"]["payload_missing_at_measurement"] = _int(
        text, r"R14 payload missing: (\d+)")
    score["event_placement"]["payload_released_before_measurement"] = _int(
        text, r"R14 payload released-before: (\d+)")
    score["event_placement"]["payload_release_after_measurement"] = _int(
        text, r"R14 payload release-after: (\d+)")
    if score["event_placement"]["camera_event_visual_count"] is None:
        frames2 = score["measurement_counts"]["visual_measured_frames"]
        score["event_placement"]["camera_event_visual_count"] = 0
        score["event_placement"]["lidar_callback_visual_count"] = frames2 or 0

    # timestamp
    for lab, pat in [
        ("mean_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=([-0-9.e+]+)"),
        ("P50_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=[-0-9.e+]+ P50=([-0-9.e+]+)"),
        ("P99_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=[-0-9.e+]+ P50=[-0-9.e+]+ P99=([-0-9.e+]+)"),
        ("max_abs_dt", r"R14 dt_visual max=([-0-9.e+]+)"),
    ]:
        score["timestamp"][lab] = _float(text, pat)

    # information (frame-level P10/P50/P90; fields are contiguous per line)
    for base, prefix in [
        ("lambda_min_norm", "lambda_min"),
        ("trace_norm", "trace"),
        ("condition", "cond"),
    ]:
        head = "R14 I cond" if base == "condition" else f"R14 I_norm {prefix}"
        score["information"][f"{base}_P10"] = _float(
            text, rf"{head} P10=([-0-9.e+]+)")
        score["information"][f"{base}_P50"] = _float(
            text, rf"{head} P10=[-0-9.e+]+ P50=([-0-9.e+]+)")
        score["information"][f"{base}_P90"] = _float(
            text, rf"{head} P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=([-0-9.e+]+)")
    # effective rank: NOT_AVAILABLE (no robust threshold rule documented yet)
    score["information"]["effective_rank_P10"] = "NOT_AVAILABLE"
    score["information"]["effective_rank_P50"] = "NOT_AVAILABLE"
    score["information"]["effective_rank_P90"] = "NOT_AVAILABLE"

    # compute
    for lab, pat in [
        ("P10", r"R14 visual cpu P10=([-0-9.e+]+)"),
        ("P50", r"R14 visual cpu P10=[-0-9.e+]+ P50=([-0-9.e+]+)"),
        ("P90", r"R14 visual cpu P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=([-0-9.e+]+)"),
    ]:
        score["compute"][f"visual_lifecycle_residual_ms_{lab}"] = _float(text, pat)
    score["compute"]["peak_rss_kb"] = _float(text, r"rss=(\d+)KB")
    score["compute"]["peak_rss_mb"] = (score["compute"]["peak_rss_kb"] / 1024.0
                                       if score["compute"]["peak_rss_kb"] else None)
    score["compute"]["visual_map_memory_estimate_mb"] = "NOT_AVAILABLE_CURRENT_INSTRUMENTATION"

    # shadow / apply side effects (Phase B adds apply metrics)
    score["shadow_side_effect"] = {
        "visual_apply_attempts_total": _int(
            text, r"R14 Shadow apply_attempts=(\d+)") or _int(
            text, r"R14 Apply attempts=(\d+)"),
        "visual_state_writes": _int(text, r"R14 Shadow state_writes=(\d+)"),
        "visual_covariance_writes": _int(text, r"R14 Shadow cov_writes=(\d+)"),
    }

    # accuracy: run the canonical NTU evaluator if trajectory + GT available
    if ate_evaluator and gt and trajectory and pathlib.Path(trajectory).exists():
        ev = subprocess.run(
            [sys.executable, ate_evaluator, "--est", trajectory, "--gt", gt],
            capture_output=True, text=True, timeout=300)
        def _find(label):
            m = re.search(label + r":\s*([-0-9.e+]+)", ev.stdout)
            return float(m.group(1)) if m else "NOT_AVAILABLE"
        for key, label in [
            ("ape_translation_rmse_m", r"ATE \(m\)"),
            ("ape_translation_mean_m", r"mean"),
            ("ape_translation_median_m", r"median"),
            ("ape_translation_max_m", r"max"),
        ]:
            score["accuracy"][key] = _find(label)
    else:
        for key in ("ape_translation_rmse_m", "ape_translation_mean_m",
                    "ape_translation_median_m", "ape_translation_max_m"):
            score["accuracy"][key] = (
                "NOT_AVAILABLE" if not (gt and trajectory) else "NOT_AVAILABLE")
    score["accuracy"]["trajectory_completion_ratio"] = score["completion"]["completion_ratio"]

    return score


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", required=True)
    ap.add_argument("--run-dir", required=True)
    ap.add_argument("--manifest")
    ap.add_argument("--trajectory")
    ap.add_argument("--gt")
    ap.add_argument("--ate-evaluator",
                    default=str(REPO / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"))
    args = ap.parse_args(argv)
    score = build_scorecard(args.stage, args.run_dir, args.manifest,
                            args.trajectory, args.gt, args.ate_evaluator)
    out_json = pathlib.Path(args.run_dir) / "visual_eval_score.json"
    out_tsv = pathlib.Path(args.run_dir) / "visual_eval_score.tsv"
    out_json.write_text(json.dumps(score, indent=2, sort_keys=False))
    with open(out_tsv, "w") as f:
        f.write("field\tvalue\n")
        for section, fields in score.items():
            if isinstance(fields, dict):
                for k, v in fields.items():
                    f.write(f"{section}.{k}\t{v}\n")
            else:
                f.write(f"{section}\t{fields}\n")
    print(f"scorecard written: {out_json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
