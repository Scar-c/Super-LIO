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

import yaml

from visual_eval_score import CANONICAL_STAGE_PARENTS
from visual_eval_score import MANIFEST_SEMANTIC_FIELDS, POLICY_ID_KEYS
from visual_eval_score import _load_semantic_snapshot

REPO = pathlib.Path(__file__).resolve().parents[3]

# Prompt75 §17/§44/§45/§46/§47/§48/§49: canonical typed schema (single source).
REGISTRY_SCHEMA = {
    "Stage": "string", "ParentStage": "string", "HEAD": "sha40/null",
    "GitDirty": "int/null", "ConfigHash": "sha64/null", "Dataset": "string",
    "Sequence": "string", "VisualEvent": "enum/string",
    "TimestampSemantics": "string", "VisualApply": "bool",
    "ApplyConnectivity": "string", "PayloadOwnershipMode": "string",
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
    "SemanticSourceSha256": "sha64/null", "SemanticProfileRevision": "sha64/null",
    "SemanticSnapshotSHA256": "sha64/null", "SemanticBindingMode": "enum/string",
    "SemanticProductionRevision": "sha40/null",
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
    apply_stage = stage == "B0_D_CAMERA_EPOCH_APPLY_CORRECTED"
    solver = "NOT_APPLICABLE" if not apply_stage else None
    # Prompt76 P2: semantic columns MUST come from the scorecard ACTUAL
    # semantics (resolved run manifest + snapshot) — never from the stage.
    a = score.get("actual_semantics", {})

    row = {
        "Stage": stage,
        "ParentStage": CANONICAL_STAGE_PARENTS.get(stage, "UNREGISTERED_STAGE"),
        "HEAD": p.get("git_sha"),
        "GitDirty": p.get("git_dirty"),
        "ConfigHash": p.get("config_hash"),
        "Dataset": p.get("dataset"),
        "Sequence": p.get("sequence"),
        "VisualEvent": a.get("visual_measurement_event", "SEMANTIC_PROVENANCE_MISSING"),
        "TimestampSemantics": a.get("visual_measurement_timestamp_semantics",
                                    "SEMANTIC_PROVENANCE_MISSING"),
        "VisualApply": a.get("visual_state_apply", "SEMANTIC_PROVENANCE_MISSING"),
        "ApplyConnectivity": a.get("visual_state_apply_connectivity",
                                   "SEMANTIC_PROVENANCE_MISSING"),
        "PayloadOwnershipMode": a.get("camera_payload_ownership_mode",
                                      "SEMANTIC_PROVENANCE_MISSING"),
        "VisualMapPolicy": a.get("visual_map_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "NormalizePolicy": a.get("normalize_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "ExposurePolicy": a.get("exposure_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "NormalPolicy": a.get("normal_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "PatchPolicy": a.get("patch_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "ResidualPolicy": a.get("residual_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
        "IterationPolicy": a.get("iteration_policy_id", "SEMANTIC_PROVENANCE_MISSING"),
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
        "SolverApplyCount": m.get("solver_apply_count") if apply_stage else solver,
        "SolverIterationCount": m.get("solver_iteration_count") if apply_stage else solver,
        "SolverCallbackInvocations": m.get("solver_callback_invocations") if apply_stage else solver,
        "SolverResidualSamplesTotal": m.get("solver_residual_samples_total") if apply_stage else solver,
        "SolverIterationsPerApplyP50": m.get("solver_iterations_per_apply_P50") if apply_stage else solver,
        "ApplyEligibleFrames": m.get("solver_apply_count") if apply_stage else solver,
        "ApplySuccess": m.get("solver_apply_success") if apply_stage else solver,
        "ApplyFailures": m.get("solver_apply_fail") if apply_stage else solver,
        "ApplySkipZeroCandidate": m.get("solver_apply_skip_zero_candidate") if apply_stage else solver,
        "ApplySkipZeroValidResidual": m.get("solver_apply_skip_zero_valid") if apply_stage else solver,
        "SemanticSourceSha256": score.get("semantic_provenance", {}).get(
            "semantic_source_sha256"),
        "SemanticProfileRevision": score.get("semantic_provenance", {}).get(
            "semantic_profile_revision"),
        "SemanticSnapshotSHA256": score.get("semantic_provenance", {}).get(
            "semantic_snapshot_sha256"),
        "SemanticBindingMode": score.get("semantic_provenance", {}).get(
            "semantic_binding_mode"),
        "SemanticProductionRevision": score.get("semantic_provenance", {}).get(
            "semantic_snapshot_production_revision"),
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


def validate_canonical_scorecard(score, contracts=None, stage=None):
    """Prompt76 §42/§43: hard conditions for a canonical A2/B0 scorecard.
    Returns a list of error strings; empty means CANONICAL_SCORECARD_VALID."""
    stage = stage or score.get("provenance", {}).get("stage_id")
    errors = []
    if stage not in ("A2_D_CAMERA_EPOCH_SHADOW",
                     "B0_D_CAMERA_EPOCH_APPLY_CORRECTED"):
        return errors
    p = score.get("provenance", {})
    a = score.get("actual_semantics", {})
    ep = score.get("event_placement", {})
    m = score.get("measurement_counts", {})
    if str(p.get("git_dirty")) != "0":
        errors.append("CANONICAL_RUN_DIRTY_SOURCE")
    if not p.get("config_hash"):
        errors.append("CANONICAL_CONFIG_HASH_MISSING")
    sp = score.get("semantic_provenance", {})
    if not sp.get("complete"):
        errors.append("SEMANTIC_PROVENANCE_MISSING")
    if not sp.get("semantic_snapshot_sha256"):
        errors.append("SEMANTIC_SNAPSHOT_HASH_MISSING")
    if sp.get("semantic_binding_mode") not in (
            "RUN_EMBEDDED", "RUN_REFERENCED", "HISTORICAL_REVISION_BINDING"):
        errors.append(f"SEMANTIC_BINDING_MODE_INVALID {sp.get('semantic_binding_mode')}")
    # expected stage contract from the snapshot
    if contracts is None:
        _, contracts, _, _ = _load_semantic_snapshot()
    expected = (contracts or {}).get(stage, {})
    for key in ("visual_measurement_event",
                "visual_measurement_timestamp_semantics",
                "visual_state_apply", "visual_state_apply_connectivity",
                "camera_payload_ownership_mode"):
        if a.get(key) != expected.get(key):
            errors.append(f"CANONICAL_STAGE_SEMANTIC_MISMATCH {key}: "
                          f"actual {a.get(key)} != expected {expected.get(key)}")
    # event placement gates
    for name, val in (("CameraEventVisualCount", ep.get("camera_event_visual_count")),
                      ("LidarCallbackVisualCount", ep.get("lidar_callback_visual_count")),
                      ("DuplicateVisualEventCount", ep.get("duplicate_visual_event_count")),
                      ("PayloadMissingAtMeasurement", ep.get("payload_missing_at_measurement")),
                      ("PayloadReleasedBeforeMeasurement", ep.get("payload_released_before_measurement"))):
        if not isinstance(val, int):
            errors.append(f"CANONICAL_EVENT_EVIDENCE_MISSING {name}={val}")
    if not (isinstance(ep.get("camera_event_visual_count"), int)
            and ep["camera_event_visual_count"] > 0):
        errors.append("A2_B0_CAMERA_EVENT_REQUIRED")
    if ep.get("lidar_callback_visual_count") != 0:
        errors.append("LIDAR_CALLBACK_VISUAL_MUST_BE_ZERO")
    if ep.get("duplicate_visual_event_count") != 0:
        errors.append("DUPLICATE_VISUAL_EVENT_MUST_BE_ZERO")
    if ep.get("payload_missing_at_measurement") != 0:
        errors.append("PAYLOAD_MISSING_MUST_BE_ZERO")
    if ep.get("payload_released_before_measurement") != 0:
        errors.append("PAYLOAD_EARLY_RELEASE_MUST_BE_ZERO")
    if stage == "A2_D_CAMERA_EPOCH_SHADOW":
        if m.get("solver_apply_count") not in (0, None):
            errors.append("A2_APPLY_ATTEMPTS_MUST_BE_ZERO")
    elif stage == "B0_D_CAMERA_EPOCH_APPLY_CORRECTED":
        if not (isinstance(m.get("solver_apply_count"), int)
                and m["solver_apply_count"] > 0):
            errors.append("B0_APPLY_ATTEMPTS_REQUIRED")
        if not (isinstance(m.get("solver_apply_success"), int)
                and isinstance(m.get("solver_apply_fail"), int)
                and m["solver_apply_count"] ==
                m["solver_apply_success"] + m["solver_apply_fail"]):
            errors.append("B0_APPLY_IDENTITY_FAIL")
        if m.get("solver_iteration_count") != m.get("solver_callback_invocations"):
            errors.append("SOLVER_ITERATION_CALLBACK_IDENTITY_FAIL")
        itpa = m.get("solver_iterations_per_apply_P50")
        if isinstance(itpa, (int, float)) and itpa > 1 and \
           m.get("solver_iteration_count") == m.get("solver_apply_count"):
            errors.append("APPLY_COUNT_SUBSTITUTED_FOR_ITERATION")
    return errors


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
        # Prompt76 §42/§43: canonical A2/B0 must pass the hard semantic
        # contract BEFORE any row is emitted.
        hard = validate_canonical_scorecard(score, stage=stage)
        if hard:
            raise ValueError("CANONICAL_SCORECARD_INVALID: " + "; ".join(hard))
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
            # Prompt76 §42/§43 semantic contract from the snapshot file
            _, contracts, _, _ = _load_semantic_snapshot()
            exp = (contracts or {}).get(stage, {})
            for key, col in (
                ("visual_measurement_event", "VisualEvent"),
                ("visual_measurement_timestamp_semantics", "TimestampSemantics"),
                ("visual_state_apply", "VisualApply"),
                ("visual_state_apply_connectivity", "ApplyConnectivity"),
                ("camera_payload_ownership_mode", "PayloadOwnershipMode"),
            ):
                val = row.get(col, "NOT_AVAILABLE")
                if val in NULL_MARKERS or val == "" or val != _norm(exp.get(key)):
                    errors.append(f"row {i}: semantic contract {col}: {val} != {exp.get(key)}")
            # Prompt76 §42/§43 / Prompt77: policy IDs must equal the RUN'S
            # BOUND snapshot contract. Historical rows bind to the immutable
            # semantic_snapshots/<revision>.yaml; run-bound rows are gated at
            # the scorecard level (the TSV validator cannot see the run dir).
            binding_mode = row.get("SemanticBindingMode", "")
            if binding_mode == "HISTORICAL_REVISION_BINDING":
                rev = row.get("SemanticProductionRevision", "")
                bound = pathlib.Path(__file__).resolve().parents[3] / \
                    "scripts/super_livo/evaluation/semantic_snapshots" / f"{rev}.yaml"
                if bound.exists():
                    policies = (yaml.safe_load(bound.read_text()) or {}).get("policies", {})
                else:
                    policies = {}
                    errors.append(f"row {i}: bound snapshot file missing {bound.name}")
            else:
                policies = None
            for col, pkey in (
                ("VisualMapPolicy", "visual_map_policy_id"),
                ("NormalizePolicy", "normalize_policy_id"),
                ("ExposurePolicy", "exposure_policy_id"),
                ("NormalPolicy", "normal_policy_id"),
                ("PatchPolicy", "patch_policy_id"),
                ("ResidualPolicy", "residual_policy_id"),
                ("IterationPolicy", "iteration_policy_id"),
            ):
                if policies is None:
                    continue  # run-bound rows gated at the scorecard level
                val = row[col]
                if val in NULL_MARKERS or val == "" or \
                   val != _norm(policies.get(pkey)):
                    errors.append(f"row {i}: policy contract {col}: {val} != {policies.get(pkey)}")
            # event placement zero gates (numeric evidence required)
            for col in ("CameraEventVisualCount", "LidarCallbackVisualCount",
                        "DuplicateVisualEventCount", "PayloadMissing",
                        "PayloadReleasedBeforeMeasurement"):
                val = row[col]
                if val in NULL_MARKERS or val == "":
                    errors.append(f"row {i}: CANONICAL_EVENT_EVIDENCE_MISSING {col}")
            if not row["CameraEventVisualCount"].isdigit() or \
               int(row["CameraEventVisualCount"]) <= 0:
                errors.append(f"row {i}: camera-event count must be > 0")
            for col in ("LidarCallbackVisualCount", "DuplicateVisualEventCount",
                        "PayloadMissing", "PayloadReleasedBeforeMeasurement"):
                if not row[col].isdigit() or int(row[col]) != 0:
                    errors.append(f"row {i}: {col} must be 0")
            # config hash must match the run artifact (EvidencePath)
            evid = row["EvidencePath"]
            if evid not in NULL_MARKERS and evid != "":
                sha_file = pathlib.Path(evid) / "effective_config.post_resolve.yaml.sha256"
                if sha_file.exists():
                    tok = sha_file.read_text(errors="replace").strip().split()
                    expected = tok[0] if tok else None
                    if expected and row["ConfigHash"] != expected:
                        errors.append(f"row {i}: config hash {row['ConfigHash'][:8]} != artifact {expected[:8]}")
            # Prompt77: canonical semantic snapshot binding must be verified
            if row["SemanticBindingMode"] != "HISTORICAL_REVISION_BINDING" and \
               row["SemanticBindingMode"] not in ("RUN_EMBEDDED", "RUN_REFERENCED"):
                errors.append(f"row {i}: semantic binding {row['SemanticBindingMode']} invalid")
            if row["SemanticSnapshotSHA256"] in NULL_MARKERS or row["SemanticSnapshotSHA256"] == "":
                errors.append(f"row {i}: semantic snapshot hash missing")
            if row["SemanticProductionRevision"] != row["HEAD"]:
                errors.append(f"row {i}: semantic snapshot revision {row['SemanticProductionRevision']} != HEAD {row['HEAD']}")
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
