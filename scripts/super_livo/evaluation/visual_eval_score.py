#!/usr/bin/env python3
"""Canonical Visual Eval Scorecard builder (Prompt75 eval final seal).

Every metric has ONE authoritative producer definition; no fallback
inference, no hard-coded dataset semantics, no stage-dependent field
meanings. Semantic labels come from the resolved manifest snapshot mapped to
immutable IDs (no "CURRENT"). Provenance (git SHA/dirty/config hash) is read
from run artifacts. Completion reference rows come from explicit
--expected-rows metadata, never a hard-coded constant.

Usage:
  visual_eval_score.py --stage <id> --run-dir <run> --manifest <yaml>
      [--trajectory x.tum] [--gt leica.csv] [--expected-rows 3981]
      [--ate-evaluator ntu_viral_official_ate.py]
"""

import argparse
import json
import pathlib
import re
import subprocess
import sys

import yaml

REPO = pathlib.Path(__file__).resolve().parents[3]

# Prompt74: explicit canonical stage-parent map (single source of truth).
# Prompt75 F6: A1 -> A0 exactly; registry validator enforces this map.
CANONICAL_STAGE_PARENTS = {
    "A0_D_LEGACY_PLACEMENT_SHADOW": "-",
    "A1_D_SCHEDULER_BASE": "A0_D_LEGACY_PLACEMENT_SHADOW",
    "A2_D_CAMERA_EPOCH_SHADOW": "A1_D_SCHEDULER_BASE",
    "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": "A2_D_CAMERA_EPOCH_SHADOW",
}

# Prompt75 F12/§64: immutable semantic checkpoint IDs (Phase C/D parent
# semantics). "CURRENT" is forbidden in the canonical registry; later phases
# change exactly one ID per semantic-family checkpoint.
SEMANTIC_SNAPSHOT_IDS = {
    "visual_map_policy": "S3_SPATIAL_BALANCED_V0",
    "normalize_semantics": "NOT_IMPLEMENTED",
    "exposure_semantics": "NOT_IMPLEMENTED",
    "normal_semantics": "NOT_IMPLEMENTED",
    "patch_semantics": "SUPER_LIVO_PRE_PHASEC_PATCH_V0",
    "residual_semantics": "SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0",
    "iteration_semantics": "SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0",
}

EVENT_PLACEMENT_FIELDS = (
    "camera_event_visual_count",
    "lidar_callback_visual_count",
    "duplicate_visual_event_count",
    "payload_missing_at_measurement",
    "payload_released_before_measurement",
    "payload_release_after_measurement",
)


def _int(text, pattern):
    m = re.search(pattern, text)
    return int(m.group(1)) if m else None


def _float(text, pattern):
    m = re.search(pattern, text)
    return float(m.group(1)) if m else None


def _load_run_provenance(run):
    """git HEAD/dirty from run_provenance.yaml (F13 / §37)."""
    p = run / "run_provenance.yaml"
    if not p.exists():
        return {}, "EVIDENCE_MISSING"
    try:
        data = yaml.safe_load(p.read_text()) or {}
    except Exception:
        return {}, "EVIDENCE_MISSING"
    run_data = data.get("run", {})
    return {
        "git_sha": run_data.get("git_head"),
        "git_dirty": (1 if run_data.get("git_dirty") else 0),
    }, "OK"


def _load_config_sha256(run):
    """config hash from effective_config.post_resolve.yaml.sha256 (§37)."""
    p = run / "effective_config.post_resolve.yaml.sha256"
    if not p.exists():
        return None
    tok = p.read_text(errors="replace").strip().split()
    return tok[0] if tok else None


def build_scorecard(stage, run_dir, manifest, trajectory=None, gt=None,
                    ate_evaluator=None, expected_rows=None):
    run = pathlib.Path(run_dir)
    node_log = run / "node_stdout.log"
    text = node_log.read_text(errors="replace") if node_log.exists() else ""

    manifest_data = {}
    if manifest and pathlib.Path(manifest).exists():
        manifest_data = yaml.safe_load(pathlib.Path(manifest).read_text()) or {}

    prov, prov_status = _load_run_provenance(run)
    not_impl = "NOT_IMPLEMENTED"
    score = {
        "provenance": {
            "git_sha": manifest_data.get("production_revision") or prov.get("git_sha"),
            "git_dirty": prov.get("git_dirty", "EVIDENCE_MISSING"),
            "provenance_status": prov_status,
            "stage_id": stage,
            "parent_stage": CANONICAL_STAGE_PARENTS.get(stage, "UNREGISTERED_STAGE"),
            "dataset": manifest_data.get("dataset"),
            "sequence": manifest_data.get("sequence"),
            "config_path": manifest_data.get("config_provenance", {}).get("dataset_calibration"),
            "config_hash": _load_config_sha256(run),
            "semantic_profile": manifest_data.get("semantic_profile"),
            # Prompt75 F12: immutable IDs from the semantic snapshot.
            "visual_map_policy": SEMANTIC_SNAPSHOT_IDS["visual_map_policy"],
            "normalize_semantics": SEMANTIC_SNAPSHOT_IDS["normalize_semantics"],
            "exposure_semantics": SEMANTIC_SNAPSHOT_IDS["exposure_semantics"],
            "normal_semantics": SEMANTIC_SNAPSHOT_IDS["normal_semantics"],
            "patch_semantics": SEMANTIC_SNAPSHOT_IDS["patch_semantics"],
            "residual_semantics": SEMANTIC_SNAPSHOT_IDS["residual_semantics"],
            "iteration_semantics": SEMANTIC_SNAPSHOT_IDS["iteration_semantics"],
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
            # Prompt75 F8: explicit scan fields (no raw/processable conflation).
            "raw_lidar_input_scans": None,
            "preobserve_excluded_scans": None,
            "eligible_raw_scans": None,
            "unique_geometry_used_scans": None,
            "geometry_update_events": None,
            "duplicate_geometry_use_events": None,
            "eligible_never_used_scans": None,
            "full_lidar_observe_count": None,
            "geometry_points_used_once": None,
            "geometry_points_duplicate": None,
            "geometry_points_never_used": None,
        },
        "measurement_counts": {},
        "residual_density_per_frame": {},
        "solver_residual_density_per_frame": {},
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

    # camera accounting
    score["completion"]["camera_received"] = _int(
        text, r"images read/dispatched/skipped:\s*(\d+)/")
    score["completion"]["camera_processable"] = _int(
        text, r"S-0 camera-epoch: epochs=(\d+)")
    score["completion"]["camera_stale"] = _int(
        text, r"stale_image_drop=(\d+)")
    # Prompt75 F8: explicit LiDAR scan fields from the producer ownership
    # line (raw / excluded / eligible / unique used / dup events / never used).
    score["completion"]["raw_lidar_input_scans"] = _int(
        text, r"Round11X fullscan ownership:.* raw_input_scans=(\d+)")
    score["completion"]["preobserve_excluded_scans"] = _int(
        text, r"Round11X fullscan ownership:.* pre_observe_excluded_scans=(\d+)")
    score["completion"]["eligible_raw_scans"] = _int(
        text, r"Round11X fullscan ownership:.* eligible_scans=(\d+)")
    score["completion"]["unique_geometry_used_scans"] = _int(
        text, r"Round11X fullscan ownership:.* unique_used_scans=(\d+)")
    score["completion"]["geometry_update_events"] = _int(
        text, r"Round11X fullscan ownership:.* geometry_update_events=(\d+)") or _int(
        text, r"cadence: policy=\S+ raw_scans=\d+ geometry_updates=(\d+)")
    score["completion"]["duplicate_geometry_use_events"] = _int(
        text, r"Round11X fullscan ownership:.* duplicate_scan_use_events=(\d+)")
    score["completion"]["eligible_never_used_scans"] = _int(
        text, r"Round11X fullscan ownership:.* eligible_never_used_scans=(\d+)")
    score["completion"]["full_lidar_observe_count"] = score["completion"]["geometry_update_events"]
    score["completion"]["geometry_points_used_once"] = _int(
        text, r"fullscan ownership:.* used_once=(\d+)")
    score["completion"]["geometry_points_duplicate"] = _int(
        text, r"fullscan ownership:.* duplicate_use=(\d+)")
    score["completion"]["geometry_points_never_used"] = _int(
        text, r"fullscan ownership:.* never_used=(\d+)")

    # Prompt75 F11: completion reference from EXPLICIT metadata/input only.
    rows = score["completion"]["trajectory_rows"]
    score["completion"]["expected_or_reference_rows"] = expected_rows
    if rows and expected_rows:
        score["completion"]["completion_ratio"] = rows / float(expected_rows)

    # measurement counts: global cumulative (existing) + Prompt75 F3
    # context-separated initial/solver accounting.
    score["measurement_counts"]["visual_query_attempts_total"] = _int(
        text, r"VISUAL_MEASUREMENT query: attempts=(\d+)")
    score["measurement_counts"]["visual_query_hits_total"] = _int(
        text, r"VISUAL_MEASUREMENT query:.* hits=(\d+)")
    score["measurement_counts"]["visual_measured_frames"] = _int(
        text, r"VISUAL_MEASUREMENT observation: frames=(\d+)")
    score["measurement_counts"]["candidate_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* candidates=(\d+)")
    score["measurement_counts"]["valid_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* valid=(\d+)")
    score["measurement_counts"]["rejected_observations_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* rejected=(\d+)")
    score["measurement_counts"]["residual_samples_total"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* residual_samples=(\d+)")
    score["measurement_counts"]["nonzero_H_frames"] = _int(
        text, r"VISUAL_MEASUREMENT H:.* nonzero=(\d+)")
    score["measurement_counts"]["nonzero_b_frames"] = _int(
        text, r"VISUAL_MEASUREMENT b:.* nonzero=(\d+)")

    # initial-linearization only (Prompt75 F3/F14: excludes solver callbacks).
    score["measurement_counts"]["initial_query_attempts_total"] = _int(
        text, r"R14 initial measurement query: attempts=(\d+)")
    score["measurement_counts"]["initial_query_hits_total"] = _int(
        text, r"R14 initial measurement query:.* hits=(\d+)")
    score["measurement_counts"]["initial_measured_frames"] = _int(
        text, r"R14 initial measurement observation: frames=(\d+)")
    score["measurement_counts"]["initial_candidate_observations_total"] = _int(
        text, r"R14 initial measurement observation:.* candidates=(\d+)")
    score["measurement_counts"]["initial_valid_observations_total"] = _int(
        text, r"R14 initial measurement observation:.* valid=(\d+)")
    score["measurement_counts"]["initial_rejected_observations_total"] = _int(
        text, r"R14 initial measurement observation:.* rejected=(\d+)")
    score["measurement_counts"]["initial_residual_samples_total"] = _int(
        text, r"R14 initial measurement observation:.* residual_samples=(\d+)") or _int(
        text, r"R14 initial_residual_samples_total=(\d+)")

    # solver iterative callbacks only (Prompt75 F3/F15).
    score["measurement_counts"]["solver_query_attempts_total"] = _int(
        text, r"R14 solver measurement query: attempts=(\d+)")
    score["measurement_counts"]["solver_query_hits_total"] = _int(
        text, r"R14 solver measurement query:.* hits=(\d+)")
    score["measurement_counts"]["solver_measured_frames"] = _int(
        text, r"R14 solver measurement observation: frames=(\d+)")
    score["measurement_counts"]["solver_candidate_observations_total"] = _int(
        text, r"R14 solver measurement observation:.* candidates=(\d+)")
    score["measurement_counts"]["solver_valid_observations_total"] = _int(
        text, r"R14 solver measurement observation:.* valid=(\d+)")
    score["measurement_counts"]["solver_rejected_observations_total"] = _int(
        text, r"R14 solver measurement observation:.* rejected=(\d+)")
    score["measurement_counts"]["solver_residual_samples_total"] = _int(
        text, r"R14 solver measurement observation:.* residual_samples=(\d+)")
    score["measurement_counts"]["solver_apply_count"] = _int(
        text, r"R14 Apply attempts=(\d+)")
    score["measurement_counts"]["solver_callback_invocations"] = _int(
        text, r"R14 solver callbacks=(\d+)")
    score["measurement_counts"]["solver_iteration_count"] = _int(
        text, r"R14 solver callbacks=\d+ completed_iterations=(\d+)")
    score["measurement_counts"]["solver_apply_success"] = _int(
        text, r"R14 Apply attempts=\d+ success=(\d+)")
    score["measurement_counts"]["solver_apply_fail"] = _int(
        text, r"R14 Apply attempts=\d+ success=\d+ fail=(\d+)")
    score["measurement_counts"]["solver_apply_skip_zero_candidate"] = _int(
        text, r"R14 Apply skip zero_candidate=(\d+)")
    score["measurement_counts"]["solver_apply_skip_zero_valid"] = _int(
        text, r"R14 Apply skip zero_candidate=\d+ zero_valid_residual=(\d+)")
    for lab, pat in [
        ("solver_iterations_per_apply_mean", r"R14 solver iterations_per_apply mean=([-0-9.e+]+)"),
        ("solver_iterations_per_apply_P10", r"R14 solver iterations_per_apply mean=[-0-9.e+]+ P10=([-0-9.e+]+)"),
        ("solver_iterations_per_apply_P50", r"R14 solver iterations_per_apply mean=[-0-9.e+]+ P10=[-0-9.e+]+ P50=([-0-9.e+]+)"),
        ("solver_iterations_per_apply_P90", r"R14 solver iterations_per_apply mean=[-0-9.e+]+ P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=([-0-9.e+]+)"),
        ("solver_iterations_per_apply_P99", r"R14 solver iterations_per_apply mean=[-0-9.e+]+ P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=[-0-9.e+]+ P99=([-0-9.e+]+)"),
        ("solver_iterations_per_apply_max", r"R14 solver iterations_per_apply mean=[-0-9.e+]+ P10=[-0-9.e+]+ P50=[-0-9.e+]+ P90=[-0-9.e+]+ P99=[-0-9.e+]+ max=([-0-9.e+]+)"),
        ("solver_callbacks_per_apply_P50", r"R14 solver callbacks_per_apply P10=[-0-9.e+]+ P50=([-0-9.e+]+)"),
    ]:
        score["measurement_counts"][lab] = _float(text, pat)

    # initial residual/frame density (Prompt75 F4: initial-only vector).
    init_frames = score["measurement_counts"]["initial_measured_frames"]
    init_total = score["measurement_counts"]["initial_residual_samples_total"]
    rpf = []
    for m in re.finditer(r"R14 residuals_per_frame P10=([-0-9.e+]+) P50=([-0-9.e+]+) "
                         r"P90=([-0-9.e+]+) P99=([-0-9.e+]+) n=(\d+)", text):
        p10, p50, p90, p99, n = (float(m.group(i)) for i in range(1, 6))
        rpf = [("P10", p10), ("P50", p50), ("P90", p90), ("P99", p99)]
        break
    for lab, val in rpf:
        score["residual_density_per_frame"][lab] = val
    if rpf:
        score["residual_density_per_frame"]["n_frames"] = int(n)
    if init_frames and init_total:
        score["residual_density_per_frame"]["mean"] = init_total / float(init_frames)
    else:
        score["residual_density_per_frame"]["mean"] = None
    solver_total = score["measurement_counts"]["solver_residual_samples_total"]
    solver_frames = score["measurement_counts"]["solver_measured_frames"]
    if solver_frames and solver_total:
        score["solver_residual_density_per_frame"]["mean"] = solver_total / float(solver_frames)

    # ratios (explicit denominators; div-by-zero -> None)
    qa = score["measurement_counts"]["initial_query_attempts_total"]
    qh = score["measurement_counts"]["initial_query_hits_total"]
    vo = score["measurement_counts"]["initial_valid_observations_total"]
    co = score["measurement_counts"]["initial_candidate_observations_total"]
    cp = score["completion"]["camera_processable"]
    frames = score["measurement_counts"]["visual_measured_frames"]
    score["ratios"]["query_hit_ratio"] = (qh / qa) if qa else None
    score["ratios"]["valid_observation_ratio"] = (vo / co) if co else None
    score["ratios"]["measured_camera_ratio"] = (frames / cp) if (cp and frames) else None

    # event placement (Prompt75 F10: NO inference; missing producer data is
    # EVIDENCE_MISSING).
    for field, pat in [
        ("camera_event_visual_count", r"R14 camera-epoch Visual: executions=(\d+)"),
        ("lidar_callback_visual_count", r"R14 LiDAR-callback Visual: executions=(\d+)"),
        ("duplicate_visual_event_count", r"R14 duplicate Visual: count=(\d+)"),
        ("payload_missing_at_measurement", r"R14 payload missing: (\d+)"),
        ("payload_released_before_measurement", r"R14 payload released-before: (\d+)"),
        ("payload_release_after_measurement", r"R14 payload release-after: (\d+)"),
    ]:
        val = _int(text, pat)
        score["event_placement"][field] = (val if val is not None else "EVIDENCE_MISSING")

    # timestamp
    for lab, pat in [
        ("mean_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=([-0-9.e+]+)"),
        ("P50_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=[-0-9.e+]+ P50=([-0-9.e+]+)"),
        ("P99_abs_dt", r"R14 dt_visual max=[-0-9.e+]+ mean=[-0-9.e+]+ P50=[-0-9.e+]+ P99=([-0-9.e+]+)"),
        ("max_abs_dt", r"R14 dt_visual max=([-0-9.e+]+)"),
    ]:
        score["timestamp"][lab] = _float(text, pat)

    # information (Prompt75 F1: ONE shared helper; evaluator only parses the
    # producer output — lambda_min/lambda_max/trace/condition P10/P50/P90 +
    # degenerate/invalid frame counts).
    for base, prefix in [
        ("lambda_min_norm", "lambda_min"),
        ("lambda_max_norm", "lambda_max"),
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
    score["information"]["degenerate_frames"] = _int(
        text, r"R14 I degenerate_frames=(\d+)")
    score["information"]["metric_invalid_frames"] = _int(
        text, r"R14 I degenerate_frames=\d+ invalid_frames=(\d+)")
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

    # shadow / apply side effects
    score["shadow_side_effect"] = {
        "visual_apply_attempts_total": _int(
            text, r"R14 Shadow apply_attempts=(\d+)") or _int(
            text, r"R14 Apply attempts=(\d+)"),
        "visual_state_writes": _int(text, r"R14 Shadow state_writes=(\d+)"),
        "visual_covariance_writes": _int(text, r"R14 Shadow cov_writes=(\d+)"),
    }

    # apply correction (delta pos/rot + cov trace)
    score["apply_correction"] = {
        "delta_position_P50": _float(text, r"R14 Apply delta_pos_m P50=([-0-9.e+]+)"),
        "delta_position_P90": _float(text, r"R14 Apply delta_pos_m P50=[-0-9.e+]+ P90=([-0-9.e+]+)"),
        "delta_rotation_P50": _float(text, r"delta_rot_rad P50=([-0-9.e+]+)"),
    }

    # accuracy: run the canonical NTU evaluator if trajectory + GT available
    # (Prompt75 F5: RMSE + mean/median/max + sample counts from the SAME
    # aligned sample set).
    if ate_evaluator and gt and trajectory and pathlib.Path(trajectory).exists():
        ev = subprocess.run(
            [sys.executable, ate_evaluator, "--est", trajectory, "--gt", gt],
            capture_output=True, text=True, timeout=300)
        def _find(label):
            m = re.search(label + r":\s*([-0-9.e+]+)", ev.stdout)
            return float(m.group(1)) if m else "NOT_AVAILABLE"
        def _find_int(label):
            m = re.search(label + r":\s*(\d+)", ev.stdout)
            return int(m.group(1)) if m else "NOT_AVAILABLE"
        for key, label in [
            ("ape_translation_rmse_m", r"ATE \(m\)"),
            ("ape_translation_mean_m", r"mean"),
            ("ape_translation_median_m", r"median"),
            ("ape_translation_max_m", r"max"),
        ]:
            score["accuracy"][key] = _find(label)
        score["accuracy"]["associated_samples"] = _find_int(r"associated_samples")
        score["accuracy"]["estimate_samples"] = _find_int(r"estimate_samples")
    else:
        for key in ("ape_translation_rmse_m", "ape_translation_mean_m",
                    "ape_translation_median_m", "ape_translation_max_m"):
            score["accuracy"][key] = "NOT_AVAILABLE"
        score["accuracy"]["associated_samples"] = "NOT_AVAILABLE"
        score["accuracy"]["estimate_samples"] = "NOT_AVAILABLE"
    score["accuracy"]["trajectory_completion_ratio"] = score["completion"]["completion_ratio"]

    return score


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", required=True)
    ap.add_argument("--run-dir", required=True)
    ap.add_argument("--manifest")
    ap.add_argument("--trajectory")
    ap.add_argument("--gt")
    ap.add_argument("--expected-rows", type=int,
                    help="explicit completion reference (Prompt75 F11)")
    ap.add_argument("--ate-evaluator",
                    default=str(REPO / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"))
    args = ap.parse_args(argv)
    score = build_scorecard(args.stage, args.run_dir, args.manifest,
                            args.trajectory, args.gt, args.ate_evaluator,
                            args.expected_rows)
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
