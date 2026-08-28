#!/usr/bin/env python3
"""Fail-closed post-run gate for canonical D_VISUAL_SHADOW evidence."""
import argparse
import json
import pathlib
import re
import sys
import yaml


def value(text, pattern):
    match = re.search(pattern, text)
    return int(match.group(1)) if match else None


def nested(document, *keys):
    for key in keys:
        if not isinstance(document, dict):
            return None
        document = document.get(key)
    return document


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", required=True)
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--state", help="strict terminal transaction/cleanup check")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    text = pathlib.Path(args.log).read_text(errors="replace")
    manifest = yaml.safe_load(pathlib.Path(args.manifest).read_text()) or {}
    metrics = {
        "visual_map_create_attempts": value(text, r"V-0 VisualMap:.* attempts=(\d+)"),
        "landmark_inserted": value(text, r"V-0 VisualMap:.* landmarks=(\d+)"),
        "landmark_query_attempts": value(text, r"VISUAL_MEASUREMENT query: attempts=(\d+)"),
        "landmark_query_hits": value(text, r"VISUAL_MEASUREMENT query:.* hits=(\d+)"),
        "landmark_query_misses": value(text, r"VISUAL_MEASUREMENT query:.* misses=(\d+)"),
        "landmark_query_rejected_explicit": value(text, r"VISUAL_MEASUREMENT query:.* rejected_explicit=(\d+)"),
        "visual_candidates": value(text, r"VISUAL_MEASUREMENT observation:.* candidates=(\d+)"),
        "visual_valid_observations": value(text, r"VISUAL_MEASUREMENT observation:.* valid=(\d+)"),
        "visual_rejected_observations": value(text, r"VISUAL_MEASUREMENT observation:.* rejected=(\d+)"),
        "visual_residual_samples": value(text, r"VISUAL_MEASUREMENT observation:.* residual_samples=(\d+)"),
        "visual_H_nonzero": value(text, r"VISUAL_MEASUREMENT H:.* nonzero=(\d+)"),
        "visual_H_nonfinite": value(text, r"VISUAL_MEASUREMENT H:.* nonfinite=(\d+)"),
        "visual_b_nonzero": value(text, r"VISUAL_MEASUREMENT b:.* nonzero=(\d+)"),
        "visual_b_nonfinite": value(text, r"VISUAL_MEASUREMENT b:.* nonfinite=(\d+)"),
        "visual_apply_count": value(text, r"VISUAL_MEASUREMENT proposed_correction=NOT_COMPUTED_BY_SHADOW_PROFILE state_apply_count=(\d+)"),
        "duplicate_use": value(text, r"fullscan ownership:.* duplicate_use=(\d+)"),
        "never_used": value(text, r"fullscan ownership:.* never_used=(\d+)"),
    }
    missing = [key for key, metric in metrics.items() if metric is None]
    query_conserved = (metrics["landmark_query_attempts"] is not None and
                       metrics["landmark_query_attempts"] ==
                       sum(metrics[k] or 0 for k in ("landmark_query_hits", "landmark_query_misses", "landmark_query_rejected_explicit")))
    positive = all((metrics[key] or 0) > 0 for key in (
        "visual_map_create_attempts", "landmark_inserted", "landmark_query_attempts",
        "landmark_query_hits", "visual_candidates", "visual_valid_observations",
        "visual_residual_samples", "visual_H_nonzero", "visual_b_nonzero"))
    finite = metrics["visual_H_nonfinite"] == 0 and metrics["visual_b_nonfinite"] == 0
    apply_off = metrics["visual_apply_count"] == 0
    duplicate_zero = metrics["duplicate_use"] == 0
    profile = manifest.get("semantic_profile") or nested(manifest, "experiment", "semantic_profile")
    apply_manifest = manifest.get("visual_state_apply")
    manifest_ok = profile == "D_VISUAL_SHADOW" and apply_manifest is False
    transaction_ok = None
    if args.state:
        state = json.loads(pathlib.Path(args.state).read_text())
        transaction_ok = state.get("state") == "SUCCESS" and state.get("cleanup_verified") is True
    passed = positive and finite and apply_off and duplicate_zero and query_conserved and manifest_ok and not missing
    if args.state:
        passed = passed and transaction_ok
    report = {
        "semantic_profile": profile, "hard_gate_pass": passed,
        "classification": "ESTABLISHED" if passed else "EVIDENCE_INCOMPLETE_NOT_CANONICAL",
        "metrics": metrics,
        "checks": {"mandatory_present": not missing, "positive_measurement_path": positive,
                   "query_conservation": query_conserved, "finite_normal_equations": finite,
                   "visual_state_apply_zero": apply_off, "duplicate_use_zero": duplicate_zero,
                   "manifest_shadow_profile_apply_off": manifest_ok,
                   "terminal_success_cleanup_verified": transaction_ok},
        "missing_mandatory_counters": missing,
        "proposed_correction": "NOT_COMPUTED_BY_SHADOW_PROFILE",
        "notes": ["never_used requires explicit legal-exclusion attribution and is not silently accepted",
                  "real Shadow evidence reports accumulated normal equations but does not solve or apply them"],
    }
    pathlib.Path(args.out).write_text(yaml.safe_dump(report, sort_keys=False))
    print(report["classification"])
    return 0 if passed else 2


if __name__ == "__main__":
    sys.exit(main())
