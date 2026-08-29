#!/usr/bin/env python3
"""Canonical Visual Eval Scorecard builder (Phase A+).

Aggregates the lightweight frame-level counters emitted by the estimator
(node stdout / evidence files) into visual_eval_score.json + .tsv.
No per-residual dumps required; default-OFF instrumentation.

Usage:
  visual_eval_score.py --stage A2_D_CAMERA_EPOCH_SHADOW --run-dir <run> \
      --manifest <resolved_experiment_semantics.yaml> [--trajectory x.tum]
"""

import argparse
import json
import math
import pathlib
import re
import sys

import yaml


def _int(text, pattern):
    m = re.search(pattern, text)
    return int(m.group(1)) if m else None


def _float(text, pattern):
    m = re.search(pattern, text)
    return float(m.group(1)) if m else None


def build_scorecard(stage, run_dir, manifest, trajectory=None):
    run = pathlib.Path(run_dir)
    node_log = run / "node_stdout.log"
    text = node_log.read_text(errors="replace") if node_log.exists() else ""

    manifest_data = {}
    if manifest and pathlib.Path(manifest).exists():
        manifest_data = yaml.safe_load(pathlib.Path(manifest).read_text()) or {}

    score = {
        "stage_id": stage,
        "provenance": {
            "git_sha": manifest_data.get("production_revision"),
            "semantic_profile": manifest_data.get("semantic_profile"),
            "legacy_alias": manifest_data.get("legacy_alias"),
            "dataset": manifest_data.get("dataset"),
            "sequence": manifest_data.get("sequence"),
            "camera_stride": manifest_data.get("camera_stride"),
            "visual_measurement_event": manifest_data.get("visual_measurement_event"),
            "visual_state_apply": manifest_data.get("visual_state_apply"),
            "camera_payload_ownership_mode": manifest_data.get("camera_payload_ownership_mode"),
            "result_path": str(run),
        },
        "completion": {
            "experiment_valid": None,
            "trajectory_rows": None,
        },
        "camera_event_measurement": {},
        "event_placement": {},
        "timestamp": {},
        "information": {},
        "spatial_coverage": {},
        "shadow_side_effect": {},
        "compute": {},
        "accuracy": {},
    }

    # completion / validity
    st = run / "state.json"
    if not st.exists() and run.parent.exists():
        st = run.parent / "state.json"
    if st.exists():
        data = json.loads(st.read_text())
        score["completion"]["experiment_valid"] = (
            data.get("state") == "SUCCESS" and data.get("cleanup_verified") is True)

    if trajectory and pathlib.Path(trajectory).exists():
        score["completion"]["trajectory_rows"] = sum(
            1 for _ in open(trajectory))

    # camera / lidar accounting
    score["completion"]["camera_received"] = _int(
        text, r"images read/dispatched/skipped:\s*(\d+)/")
    score["completion"]["camera_processable"] = _int(
        text, r"S-0 camera-epoch: epochs=(\d+)")
    score["completion"]["camera_stale"] = _int(
        text, r"stale_image_drop=(\d+)")
    score["completion"]["lidar_full_observe"] = _int(
        text, r"fullscan ownership:.*used_once=(\d+)")

    # camera-event measurement activity (legacy placement fields for A0)
    score["camera_event_measurement"]["query_attempts"] = _int(
        text, r"VISUAL_MEASUREMENT query: attempts=(\d+)")
    score["camera_event_measurement"]["query_hits"] = _int(
        text, r"VISUAL_MEASUREMENT query:.* hits=(\d+)")
    score["camera_event_measurement"]["candidate_observations"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* candidates=(\d+)")
    score["camera_event_measurement"]["valid_observations"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* valid=(\d+)")
    score["camera_event_measurement"]["rejected_observations"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* rejected=(\d+)")
    score["camera_event_measurement"]["frames_with_measurement"] = _int(
        text, r"VISUAL_MEASUREMENT observation: frames=(\d+)")
    score["camera_event_measurement"]["frames_nonzero_H"] = _int(
        text, r"VISUAL_MEASUREMENT H:.* nonzero=(\d+)")
    score["camera_event_measurement"]["frames_nonzero_b"] = _int(
        text, r"VISUAL_MEASUREMENT b:.* nonzero=(\d+)")
    score["camera_event_measurement"]["residual_samples"] = _int(
        text, r"VISUAL_MEASUREMENT observation:.* residual_samples=(\d+)")

    qa = score["camera_event_measurement"]["query_attempts"]
    qh = score["camera_event_measurement"]["query_hits"]
    if qa:
        score["camera_event_measurement"]["query_hit_ratio"] = qh / qa
    vo = score["camera_event_measurement"]["valid_observations"]
    co = score["camera_event_measurement"]["candidate_observations"]
    if co:
        score["camera_event_measurement"]["valid_observation_ratio"] = vo / co

    # event placement (camera-epoch vs LiDAR callback)
    score["event_placement"]["camera_event_visual_count"] = _int(
        text, r"R14 camera-epoch Visual: executions=(\d+)")
    score["event_placement"]["lidar_callback_visual_count"] = _int(
        text, r"R14 LiDAR-callback Visual: executions=(\d+)")
    score["event_placement"]["duplicate_visual_count"] = _int(
        text, r"R14 duplicate Visual: count=(\d+)")
    score["event_placement"]["payload_missing_at_measurement"] = _int(
        text, r"R14 payload missing: (\d+)")
    score["event_placement"]["payload_released_before_measurement"] = _int(
        text, r"R14 payload released-before: (\d+)")
    score["event_placement"]["payload_release_after_measurement"] = _int(
        text, r"R14 payload release-after: (\d+)")
    # legacy placement mapping: no R14 counters -> camera-event=0,
    # LiDAR-callback = measurement frames (the historical placement)
    if (score["event_placement"]["camera_event_visual_count"] is None
            and score["event_placement"]["lidar_callback_visual_count"] is None):
        frames = score["camera_event_measurement"]["frames_with_measurement"]
        score["event_placement"]["camera_event_visual_count"] = 0
        score["event_placement"]["lidar_callback_visual_count"] = frames or 0

    # timestamp semantics
    dt_max = _float(text, r"R14 dt_visual max=([-0-9.e+]+)")
    dt_mean = _float(text, r"R14 dt_visual mean=([-0-9.e+]+)")
    dt_p50 = _float(text, r"R14 dt_visual P50=([-0-9.e+]+)")
    dt_p99 = _float(text, r"R14 dt_visual P99=([-0-9.e+]+)")
    score["timestamp"] = {"max_abs_dt": dt_max, "mean_abs_dt": dt_mean,
                          "P50": dt_p50, "P99": dt_p99}

    # information score
    lm_norm = _float(text, r"R14 I_norm lambda_min P50=([-0-9.e+]+)")
    tr_norm = _float(text, r"R14 I_norm trace P50=([-0-9.e+]+)")
    cond = _float(text, r"cond P50=([-0-9.e+]+)")
    score["information"] = {"lambda_min_norm_P50": lm_norm,
                            "trace_norm_P50": tr_norm, "cond_P50": cond}

    # shadow side effects
    score["shadow_side_effect"] = {
        "visual_apply_attempts": _int(
            text, r"R14 Shadow apply_attempts=(\d+)"),
        "state_writes": _int(text, r"R14 Shadow state_writes=(\d+)"),
        "covariance_writes": _int(text, r"R14 Shadow cov_writes=(\d+)"),
    }

    # compute
    score["compute"] = {
        "visual_cpu_ms_per_frame_P50": _float(
            text, r"R14 visual cpu P50=([-0-9.e+]+)"),
        "peak_rss_kb": _float(text, r"R14 peak rss kb=([-0-9.e+]+)"),
    }

    return score


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", required=True)
    ap.add_argument("--run-dir", required=True)
    ap.add_argument("--manifest")
    ap.add_argument("--trajectory")
    args = ap.parse_args(argv)
    score = build_scorecard(args.stage, args.run_dir, args.manifest,
                            args.trajectory)
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
