#!/usr/bin/env python3
"""Fail-closed post-run gate for canonical D_VISUAL_SHADOW evidence."""
import argparse
import pathlib
import re
import sys
import yaml

def value(text, pattern, group=1):
    match = re.search(pattern, text)
    return int(match.group(group)) if match else None

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--log", required=True); ap.add_argument("--out", required=True)
    args = ap.parse_args(); text = pathlib.Path(args.log).read_text(errors="replace")
    metrics = {
        "visual_map_create_attempts": value(text, r"V-0 VisualMap:.* attempts=(\d+)"),
        "landmark_inserted": value(text, r"V-0 VisualMap:.* landmarks=(\d+)"),
        "landmark_query_hits": None,
        "visual_candidates": value(text, r"V-0C coverage:.* cells_with_candidates=(\d+)"),
        "visual_valid_observations": value(text, r"V-2 photometric:.* accepted_landmarks=(\d+)"),
        "visual_residual_samples": value(text, r"V-2 photometric:.* total_samples=(\d+)"),
        "visual_H_nonzero": None,
        "visual_b_nonzero": None,
        "visual_apply_count": value(text, r"V-4 health: apply_count=(\d+)"),
        "duplicate_use": value(text, r"fullscan ownership:.* duplicate_use=(\d+)"),
        "never_used": value(text, r"fullscan ownership:.* never_used=(\d+)"),
    }
    missing = [key for key in ("landmark_query_hits", "visual_H_nonzero", "visual_b_nonzero")
               if metrics[key] is None]
    positive = all((metrics[key] or 0) > 0 for key in (
        "visual_map_create_attempts", "landmark_inserted", "visual_candidates",
        "visual_valid_observations", "visual_residual_samples"))
    apply_off = metrics["visual_apply_count"] == 0
    duplicate_zero = metrics["duplicate_use"] == 0
    passed = positive and apply_off and duplicate_zero and not missing
    report = {
        "semantic_profile": "D_VISUAL_SHADOW", "hard_gate_pass": passed,
        "classification": "ESTABLISHED" if passed else "EVIDENCE_INCOMPLETE_NOT_CANONICAL",
        "metrics": metrics, "missing_mandatory_counters": missing,
        "notes": [
            "never_used requires explicit legal-exclusion attribution and is not silently accepted",
            "nonzero residual samples do not substitute for separately persisted H/b counters",
        ],
    }
    pathlib.Path(args.out).write_text(yaml.safe_dump(report, sort_keys=False))
    print(report["classification"])
    return 0 if passed else 2

if __name__ == "__main__": sys.exit(main())
