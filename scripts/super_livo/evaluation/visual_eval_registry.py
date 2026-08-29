#!/usr/bin/env python3
"""Canonical Visual Eval registry generator (Prompt75 §50/§51/§52).

The registry TSV is GENERATED from canonical scorecard JSONs — it is never
manually authoritative. Rows are built by mapping scorecard fields; the
stage-parent map (CANONICAL_STAGE_PARENTS) is enforced exactly; "CURRENT"
semantic labels, dirty-source canonical rows, non-zero duplicate geometry
use events, and missing GT stats are rejected.

Usage:
  visual_eval_registry.py --stage A2_D_CAMERA_EPOCH_SHADOW --scorecard a.json
                          [--stage ... --scorecard ...] --out registry.tsv
"""

import argparse
import json
import pathlib
import re
import sys

from visual_eval_score import CANONICAL_STAGE_PARENTS

REPO = pathlib.Path(__file__).resolve().parents[3]

# Prompt75 §17/§44/§45/§46/§47/§48/§49: canonical typed schema (single source).
REGISTRY_SCHEMA = {
    "Stage": "string", "ParentStage": "string", "HEAD": "sha40/null",
    "GitDirty": "int/null", "ConfigHash": "sha64/null", "Dataset": "string",
    "Sequence": "string", "VisualEvent": "enum/string", "VisualApply": "bool",
    "VisualMapPolicy": "string", "NormalizePolicy": "string",
    "ExposurePolicy": "string", "NormalPolicy": "string", "PatchPolicy": "string",
    "ResidualPolicy": "string", "IterationPolicy": "string",
    "APE_RMSE_m": "float/null", "APE_Mean_m": "float/null",
    "APE_Median_m": "float/null", "APE_Max_m": "float/null",
    "CompletionRatio": "float/null", "AssociatedSamples": "int/null",
    "EstimateSamples": "int/null",
    "InitialMeasuredFrames": "int/null", "InitialQueryAttempts": "int/null",
    "InitialQueryHits": "int/null", "InitialCandidates": "int/null",
    "InitialValidObservations": "int/null", "InitialRejectedObservations": "int/null",
    "InitialValidObservationRatio": "float/null",
    "InitialResidualSamplesTotal": "int/null",
    "InitialResidualsPerFrameMean": "float/null",
    "InitialResidualsPerFrameP10": "float/null",
    "InitialResidualsPerFrameP50": "float/null",
    "InitialResidualsPerFrameP90": "float/null",
    "InitialResidualsPerFrameP99": "float/null",
    "InitialLambdaMinNormP50": "float/null", "InitialLambdaMaxNormP50": "float/null",
    "InitialTraceNormP50": "float/null", "InitialConditionP50": "float/null",
    "InitialDegenerateFrames": "int/null", "InitialMetricInvalidFrames": "int/null",
    "SolverApplyCount": "int/null", "SolverIterationCount": "int/null",
    "SolverCallbackInvocations": "int/null", "SolverResidualSamplesTotal": "int/null",
    "SolverIterationsPerApplyP50": "float/null",
    "ApplyEligibleFrames": "int/null", "ApplySuccess": "int/null",
    "ApplyFailures": "int/null", "ApplySkipZeroCandidate": "int/null",
    "ApplySkipZeroValidResidual": "int/null",
    "RawLidarInputScans": "int/null", "PreObserveExcludedScans": "int/null",
    "EligibleRawScans": "int/null", "UniqueGeometryUsedScans": "int/null",
    "GeometryUpdateEvents": "int/null", "DuplicateGeometryUseEvents": "int/null",
    "EligibleNeverUsedScans": "int/null",
    "CameraEventVisualCount": "int/null", "LidarCallbackVisualCount": "int/null",
    "DuplicateVisualEventCount": "int/null", "PayloadMissing": "int/null",
    "PayloadReleasedBeforeMeasurement": "int/null",
    "VisualCPU_P50_ms": "float/null", "PeakRSS_MB": "float/null",
    "Classification": "enum", "EvidencePath": "string",
}

NULL_MARKERS = {"NOT_AVAILABLE", "NOT_APPLICABLE", "PENDING", "EVIDENCE_MISSING",
                "UNREGISTERED_STAGE"}
CLASSIFICATIONS = {"VALID", "MIGRATED_HISTORICAL", "NONCANONICAL",
                   "NONCANONICAL_DIRTY_SOURCE"}
STAGE_ORDER = ("A0_D_LEGACY_PLACEMENT_SHADOW", "A1_D_SCHEDULER_BASE",
               "A2_D_CAMERA_EPOCH_SHADOW", "B0_D_CAMERA_EPOCH_APPLY_CORRECTED")


def _fmt(v):
    if v is None:
        return "NOT_AVAILABLE"
    if isinstance(v, bool):
        return "true" if v else "false"
    return str(v) if not isinstance(v, str) else ("true" if v == "True" else ("false" if v == "False" else v))


def _norm(v):
    """registry cell from a scorecard value; None -> NOT_AVAILABLE."""
    if v is None:
        return "NOT_AVAILABLE"
    if isinstance(v, str):
        return v
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, float):
        return f"{v:.9g}"
    return str(v)


def _row_from_scorecard(score):
    """Map a canonical scorecard JSON to a registry row dict (full schema)."""
    p = score.get("provenance", {})
    c = score.get("completion", {})
    m = score.get("measurement_counts", {})
    rpf = score.get("residual_density_per_frame", {})
    info = score.get("information", {})
    ep = score.get("event_placement", {})
    acc = score.get("accuracy", {})
    comp = score.get("compute", {})
    rat = score.get("ratios", {})
    stage = p.get("stage_id", "UNREGISTERED_STAGE")
    apply_stage = stage.startswith("B0")
    solver = "NOT_APPLICABLE"
    if apply_stage:
        solver = None

    row = {
        "Stage": stage,
        "ParentStage": CANONICAL_STAGE_PARENTS.get(stage, "UNREGISTERED_STAGE"),
        "HEAD": p.get("git_sha"),
        "GitDirty": p.get("git_dirty"),
        "ConfigHash": p.get("config_hash"),
        "Dataset": p.get("dataset"),
        "Sequence": p.get("sequence"),
        "VisualEvent": ({"B0_D_CAMERA_EPOCH_APPLY_CORRECTED": "CAMERA_EPOCH"}.get(stage)
                        if stage in CANONICAL_STAGE_PARENTS
                        else "UNREGISTERED_STAGE"),
        "VisualApply": stage == "B0_D_CAMERA_EPOCH_APPLY_CORRECTED",
        "VisualMapPolicy": p.get("visual_map_policy"),
        "NormalizePolicy": p.get("normalize_semantics"),
        "ExposurePolicy": p.get("exposure_semantics"),
        "NormalPolicy": p.get("normal_semantics"),
        "PatchPolicy": p.get("patch_semantics"),
        "ResidualPolicy": p.get("residual_semantics"),
        "IterationPolicy": p.get("iteration_semantics"),
        "APE_RMSE_m": acc.get("ape_translation_rmse_m"),
        "APE_Mean_m": acc.get("ape_translation_mean_m"),
        "APE_Median_m": acc.get("ape_translation_median_m"),
        "APE_Max_m": acc.get("ape_translation_max_m"),
        "CompletionRatio": c.get("completion_ratio"),
        "AssociatedSamples": acc.get("associated_samples"),
        "EstimateSamples": acc.get("estimate_samples"),
        "InitialMeasuredFrames": m.get("initial_measured_frames"),
        "InitialQueryAttempts": m.get("initial_query_attempts_total"),
        "InitialQueryHits": m.get("initial_query_hits_total"),
        "InitialCandidates": m.get("initial_candidate_observations_total"),
        "InitialValidObservations": m.get("initial_valid_observations_total"),
        "InitialRejectedObservations": m.get("initial_rejected_observations_total"),
        "InitialValidObservationRatio": rat.get("valid_observation_ratio"),
        "InitialResidualSamplesTotal": m.get("initial_residual_samples_total"),
        "InitialResidualsPerFrameMean": rpf.get("mean"),
        "InitialResidualsPerFrameP10": rpf.get("P10"),
        "InitialResidualsPerFrameP50": rpf.get("P50"),
        "InitialResidualsPerFrameP90": rpf.get("P90"),
        "InitialResidualsPerFrameP99": rpf.get("P99"),
        "InitialLambdaMinNormP50": info.get("lambda_min_norm_P50"),
        "InitialLambdaMaxNormP50": info.get("lambda_max_norm_P50"),
        "InitialTraceNormP50": info.get("trace_norm_P50"),
        "InitialConditionP50": info.get("condition_P50"),
        "InitialDegenerateFrames": info.get("degenerate_frames"),
        "InitialMetricInvalidFrames": info.get("metric_invalid_frames"),
        "SolverApplyCount": m.get("solver_apply_count", solver),
        "SolverIterationCount": m.get("solver_iteration_count", solver),
        "SolverCallbackInvocations": m.get("solver_callback_invocations", solver),
        "SolverResidualSamplesTotal": m.get("solver_residual_samples_total", solver),
        "SolverIterationsPerApplyP50": m.get("solver_iterations_per_apply_P50", solver),
        "ApplyEligibleFrames": m.get("solver_apply_count", solver),
        "ApplySuccess": m.get("solver_apply_success", solver),
        "ApplyFailures": m.get("solver_apply_fail", solver),
        "ApplySkipZeroCandidate": m.get("solver_apply_skip_zero_candidate", solver),
        "ApplySkipZeroValidResidual": m.get("solver_apply_skip_zero_valid", solver),
        "RawLidarInputScans": c.get("raw_lidar_input_scans"),
        "PreObserveExcludedScans": c.get("preobserve_excluded_scans"),
        "EligibleRawScans": c.get("eligible_raw_scans"),
        "UniqueGeometryUsedScans": c.get("unique_geometry_used_scans"),
        "GeometryUpdateEvents": c.get("geometry_update_events"),
        "DuplicateGeometryUseEvents": c.get("duplicate_geometry_use_events"),
        "EligibleNeverUsedScans": c.get("eligible_never_used_scans"),
        "CameraEventVisualCount": ep.get("camera_event_visual_count"),
        "LidarCallbackVisualCount": ep.get("lidar_callback_visual_count"),
        "DuplicateVisualEventCount": ep.get("duplicate_visual_event_count"),
        "PayloadMissing": ep.get("payload_missing_at_measurement"),
        "PayloadReleasedBeforeMeasurement": ep.get("payload_released_before_measurement"),
        "VisualCPU_P50_ms": comp.get("visual_lifecycle_residual_ms_P50"),
        "PeakRSS_MB": comp.get("peak_rss_mb"),
        "Classification": "VALID",
        "EvidencePath": p.get("result_path", "NOT_AVAILABLE"),
    }
    return row


def _check_canonical_provenance(stage, row, score):
    """Hard gate for canonical (A2/B0) rows: config hash must match the
    run artifact's sha256 file and the source must be clean."""
    if stage not in ("A2_D_CAMERA_EPOCH_SHADOW",
                     "B0_D_CAMERA_EPOCH_APPLY_CORRECTED"):
        return
    if str(row["GitDirty"]) == "1":
        raise ValueError(f"CANONICAL_RUN_DIRTY_SOURCE: {stage}")
    result_path = score.get("provenance", {}).get("result_path")
    if result_path:
        sha = pathlib.Path(result_path) / "effective_config.post_resolve.yaml.sha256"
        if sha.exists():
            tok = sha.read_text(errors="replace").strip().split()
            expected = tok[0] if tok else None
            if expected and str(row["ConfigHash"]) != expected:
                raise ValueError(
                    f"CANONICAL_CONFIG_HASH_MISMATCH: {stage} "
                    f"{row['ConfigHash']} != {expected}")


def _classify(row, score):
    if row["Stage"] in ("A0_D_LEGACY_PLACEMENT_SHADOW", "A1_D_SCHEDULER_BASE"):
        return "MIGRATED_HISTORICAL"
    dirty = row["GitDirty"]
    if dirty is not None and str(dirty) == "1":
        return "NONCANONICAL_DIRTY_SOURCE"
    return "VALID"


def generate_registry(scorecards, out_path, schema=None):
    """scorecards: dict stage -> scorecard dict (or path). Builds the TSV."""
    schema = schema or REGISTRY_SCHEMA
    rows = []
    seen = set()
    for stage in STAGE_ORDER:
        if stage not in scorecards:
            continue
        score = scorecards[stage]
        if isinstance(score, (str, pathlib.Path)):
            score = json.loads(pathlib.Path(score).read_text())
        row = _row_from_scorecard(score)
        row["Classification"] = _classify(row, score)
        # Prompt75 §37/FS-T25/§78: canonical-stage provenance must be
        # mechanically consistent with the run artifact (config hash) and a
        # clean source; mismatch/dirty -> hard rejection, not classification.
        _check_canonical_provenance(stage, row, score)
        rows.append(row)
        seen.add(stage)
    # any extra (non-canonical / historical) stages
    for stage, score in scorecards.items():
        if stage in seen:
            continue
        if isinstance(score, (str, pathlib.Path)):
            score = json.loads(pathlib.Path(score).read_text())
        row = _row_from_scorecard(score)
        row["Classification"] = _classify(row, score)
        rows.append(row)
        seen.add(stage)

    header = list(schema.keys())
    lines = ["\t".join(header)]
    for row in rows:
        lines.append("\t".join(_norm(row.get(h, "NOT_AVAILABLE")) for h in header))
    text = "\n".join(lines) + "\n"
    if out_path:
        pathlib.Path(out_path).write_text(text)
    return text, rows


def validate_registry(path, schema=None, canonical_stage_parents=None):
    """Full validation: schema/types/exact parent/CURRENT/classification."""
    schema = schema or REGISTRY_SCHEMA
    canonical_stage_parents = canonical_stage_parents or CANONICAL_STAGE_PARENTS
    errors = []
    lines = pathlib.Path(path).read_text().rstrip("\n").split("\n")
    header = lines[0].split("\t")
    if header != list(schema):
        errors.append(f"columns mismatch: {header}")
        return errors
    seen = set()
    for i, line in enumerate(lines[1:], start=2):
        if not line.strip():
            continue
        cols = line.split("\t")
        if len(cols) != len(schema):
            errors.append(f"row {i}: {len(cols)} columns")
            continue
        row = dict(zip(header, cols))
        stage = row["Stage"]
        if stage in seen:
            errors.append(f"row {i}: duplicate stage {stage}")
        seen.add(stage)
        # Prompt75 FS-T29: only canonical stages may be VALID (a historical
        # or invented stage can never claim canonical status or a parent).
        if row["Classification"] == "VALID" and stage not in STAGE_ORDER:
            errors.append(f"row {i}: non-canonical stage {stage} marked VALID")
        # Prompt75 F6: exact parent enforcement — no exception.
        expected_parent = canonical_stage_parents.get(stage, "UNREGISTERED_STAGE")
        if row["ParentStage"] != expected_parent:
            errors.append(f"row {i}: {stage} parent {row['ParentStage']} != {expected_parent}")
        if "CURRENT" in row.values():
            errors.append(f"row {i}: CURRENT semantic label present")
        if row["HEAD"] not in NULL_MARKERS and not re.fullmatch(r"[0-9a-f]{40}", row["HEAD"]):
            errors.append(f"row {i}: HEAD {row['HEAD']}")
        if row["Classification"] not in CLASSIFICATIONS:
            errors.append(f"row {i}: classification {row['Classification']}")
        if stage in ("A2_D_CAMERA_EPOCH_SHADOW", "B0_D_CAMERA_EPOCH_APPLY_CORRECTED"):
            if row["GitDirty"] == "1":
                errors.append(f"row {i}: canonical {stage} from dirty source")
            if row["Classification"] != "VALID":
                errors.append(f"row {i}: canonical {stage} not VALID")
            for f in ("APE_RMSE_m", "APE_Mean_m", "APE_Median_m", "APE_Max_m"):
                if row[f] in NULL_MARKERS or row[f] == "":
                    errors.append(f"row {i}: {f} not numeric for canonical {stage}")
            if row["DuplicateGeometryUseEvents"] not in ("0", "NOT_AVAILABLE") and row["DuplicateGeometryUseEvents"] != "NOT_APPLICABLE":
                errors.append(f"row {i}: duplicate geometry use {row['DuplicateGeometryUseEvents']}")
            if row["GeometryUpdateEvents"] != "NOT_AVAILABLE" and \
               row["UniqueGeometryUsedScans"] != "NOT_AVAILABLE" and \
               row["GeometryUpdateEvents"] != row["UniqueGeometryUsedScans"]:
                errors.append(f"row {i}: update events != unique used scans")
            if row["CameraEventVisualCount"] in NULL_MARKERS or row["CameraEventVisualCount"] == "":
                errors.append(f"row {i}: camera-event evidence missing for {stage}")
            if stage == "B0_D_CAMERA_EPOCH_APPLY_CORRECTED":
                # Prompt75 §78: Apply count must not masquerade as residual
                # total or as the iteration count.
                if row["SolverIterationCount"] not in NULL_MARKERS and \
                   row["SolverCallbackInvocations"] not in NULL_MARKERS and \
                   row["SolverIterationCount"] != row["SolverCallbackInvocations"]:
                    errors.append(f"row {i}: iteration {row['SolverIterationCount']} != callbacks {row['SolverCallbackInvocations']}")
                if row["InitialResidualSamplesTotal"] not in NULL_MARKERS and \
                   row["SolverApplyCount"] not in NULL_MARKERS and \
                   row["InitialResidualSamplesTotal"] == row["SolverApplyCount"]:
                    errors.append(f"row {i}: initial residual total equals Apply count")
        # type check
        for col, typ in schema.items():
            val = row[col]
            if val in NULL_MARKERS or val == "":
                continue
            if typ.startswith("int"):
                if not val.isdigit():
                    errors.append(f"row {i}: {col}={val} not int")
            elif typ.startswith("float"):
                try:
                    float(val)
                except ValueError:
                    errors.append(f"row {i}: {col}={val} not float")
            elif typ.startswith("bool"):
                if val not in ("true", "false"):
                    errors.append(f"row {i}: {col}={val} not bool")
    return errors


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", action="append", required=True)
    ap.add_argument("--scorecard", action="append", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--validate-only", action="store_true")
    args = ap.parse_args(argv)
    if len(args.stage) != len(args.scorecard):
        print("stage/scorecard count mismatch", file=sys.stderr)
        return 2
    scorecards = dict(zip(args.stage, args.scorecard))
    text, rows = generate_registry(scorecards, args.out)
    errors = validate_registry(args.out)
    if errors:
        print("registry validation FAILED:")
        for e in errors:
            print(" ", e)
        return 1
    print(f"registry generated: {args.out} ({len(rows)} rows, validation PASS)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
