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
import hashlib
import json
import pathlib
import re
import subprocess
import sys

import yaml

REPO = pathlib.Path(__file__).resolve().parents[3]

DEFAULT_SEMANTIC_SNAPSHOT = REPO / "scripts/super_livo/evaluation/semantic_snapshot_v0.yaml"
HISTORICAL_SNAPSHOT_DIR = REPO / "scripts/super_livo/evaluation/semantic_snapshots"
RUN_BINDING_DIR = REPO / "docs/super_livo/evidence/run_semantic_bindings"

# Prompt77 §12: canonical semantic binding modes (no other canonical mode).
BINDING_RUN_EMBEDDED = "RUN_EMBEDDED"
BINDING_RUN_REFERENCED = "RUN_REFERENCED"
BINDING_HISTORICAL = "HISTORICAL_REVISION_BINDING"
BINDING_NONE = "SEMANTIC_PROVENANCE_MISSING"

# Prompt76 §7/§19: EXPECTED stage semantic contract (validator metadata
# only). Loaded from the current template's stage_contracts — it describes
# the STAGE, never the run; actual semantics always come from the run.
import contextlib


@contextlib.contextmanager
def _semantic_env(snap_dir=None, template=None, binding_dir=None):
    """Test-only injection for the semantic resolution directories."""
    global HISTORICAL_SNAPSHOT_DIR, DEFAULT_SEMANTIC_SNAPSHOT, RUN_BINDING_DIR
    old = (HISTORICAL_SNAPSHOT_DIR, DEFAULT_SEMANTIC_SNAPSHOT, RUN_BINDING_DIR)
    if snap_dir is not None:
        HISTORICAL_SNAPSHOT_DIR = pathlib.Path(snap_dir)
    if template is not None:
        DEFAULT_SEMANTIC_SNAPSHOT = pathlib.Path(template)
    if binding_dir is not None:
        RUN_BINDING_DIR = pathlib.Path(binding_dir)
    try:
        yield
    finally:
        HISTORICAL_SNAPSHOT_DIR, DEFAULT_SEMANTIC_SNAPSHOT, RUN_BINDING_DIR = old


def _load_stage_contracts():
    if not DEFAULT_SEMANTIC_SNAPSHOT.exists():
        return {}
    data = yaml.safe_load(DEFAULT_SEMANTIC_SNAPSHOT.read_text()) or {}
    return data.get("stage_contracts", {})


SEMANTIC_STAGE_CONTRACTS = _load_stage_contracts()

# Prompt74: explicit canonical stage-parent map (single source of truth).
# Prompt75 F6: A1 -> A0 exactly; registry validator enforces this map.
CANONICAL_STAGE_PARENTS = {
    "A0_D_LEGACY_PLACEMENT_SHADOW": "-",
    "A1_D_SCHEDULER_BASE": "A0_D_LEGACY_PLACEMENT_SHADOW",
    "A2_D_CAMERA_EPOCH_SHADOW": "A1_D_SCHEDULER_BASE",
    "B0_D_CAMERA_EPOCH_APPLY_CORRECTED": "A2_D_CAMERA_EPOCH_SHADOW",
}

# Prompt76 P1: semantic authority is the RUN. Policy IDs and the expected
# stage contract live in the explicit machine-readable semantic snapshot
# file (semantic_snapshot_v0.yaml) — NEVER as evaluator constants. Missing
# snapshot/provenance for canonical stages is SEMANTIC_PROVENANCE_MISSING,
# never a default.
MANIFEST_SEMANTIC_FIELDS = (
    "semantic_profile",
    "visual_measurement_enabled",
    "visual_measurement_event",
    "visual_measurement_timestamp_semantics",
    "visual_measurement_exact_once",
    "camera_payload_ownership_mode",
    "visual_state_apply",
    "visual_state_apply_connectivity",
)

POLICY_ID_KEYS = (
    "visual_map_policy_id",
    "normalize_policy_id",
    "exposure_policy_id",
    "normal_policy_id",
    "patch_policy_id",
    "residual_policy_id",
    "iteration_policy_id",
)

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


def _resolve_run_semantics(run_dir, manifest_data, canonical=True):
    """Prompt77 §11-13: canonical semantic policy source resolution.

    Returns a dict with keys: policies, snapshot_source, snapshot_sha256,
    production_revision, binding_mode, runtime (manifest semantics).

    Resolution order (canonical):
      1. run-embedded/run-referenced snapshot (future runs)
      2. validated historical revision binding (canonical 31d677e pair)
      else -> SEMANTIC_PROVENANCE_MISSING (fail-closed).

    The current-checkout template semantic_snapshot_v0.yaml is NEVER a
    canonical source for historical runs.
    """
    run = pathlib.Path(run_dir)
    manifest_rev = manifest_data.get("production_revision")
    # Prompt79 G1: when the manifest CLAIMS a run-bound snapshot, the path is
    # REQUIRED and is the exact authority (no file discovery). Historical
    # runs (no path field) fall through to their revision binding; a run
    # with neither is SEMANTIC_SNAPSHOT_PATH_MISSING (fail-closed).
    manifest_path = manifest_data.get("semantic_snapshot_path")
    if manifest_path:
        resolved_path = pathlib.Path(manifest_path)
        if not resolved_path.is_absolute():
            resolved_path = run / resolved_path
        if not resolved_path.exists():
            raise ValueError("SEMANTIC_SNAPSHOT_MISSING")
        snapshot_file = resolved_path
        mode = (BINDING_RUN_EMBEDDED
                if resolved_path == (run / "semantic_snapshot.yaml").resolve()
                else BINDING_RUN_REFERENCED)
    else:
        # no run-bound claim: historical revision binding or fail-closed
        if canonical:
            binding = _find_historical_binding(manifest_rev, run)
            if binding is not None:
                return _resolve_historical_binding(binding, manifest_rev)
        raise ValueError("SEMANTIC_SNAPSHOT_PATH_MISSING")
    if snapshot_file is not None:
        if not snapshot_file.exists():
            raise ValueError("SEMANTIC_SNAPSHOT_MISSING")
        manifest_sha = manifest_data.get("semantic_snapshot_sha256")
        manifest_schema = manifest_data.get("semantic_snapshot_schema_version")
        if not manifest_sha:
            raise ValueError("SEMANTIC_SNAPSHOT_HASH_MISSING")
        if not manifest_schema:
            raise ValueError("SEMANTIC_SNAPSHOT_SCHEMA_MISSING")
        if not manifest_rev:
            raise ValueError("SEMANTIC_SNAPSHOT_REVISION_MISSING")
        data = yaml.safe_load(snapshot_file.read_text()) or {}
        policies = data.get("policies", {})
        rev = data.get("production_revision")
        schema = data.get("snapshot_schema_version")
        if not rev:
            raise ValueError("SEMANTIC_SNAPSHOT_REVISION_MISSING")
        if not schema:
            raise ValueError("SEMANTIC_SNAPSHOT_SCHEMA_MISSING")
        # §19: production revision must be a full 40-char git SHA
        if not (isinstance(rev, str) and re.fullmatch(r"[0-9a-f]{40}", rev)):
            raise ValueError(f"SEMANTIC_SNAPSHOT_REVISION_MISMATCH: {rev} not full SHA")
        if rev != manifest_rev:
            raise ValueError(f"SEMANTIC_SNAPSHOT_REVISION_MISMATCH: {rev} != {manifest_rev}")
        if str(schema) != str(manifest_schema):
            raise ValueError(
                f"SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH: {schema} != {manifest_schema}")
        actual_sha = hashlib.sha256(snapshot_file.read_bytes()).hexdigest()
        if actual_sha != manifest_sha:
            raise ValueError(
                f"SEMANTIC_SNAPSHOT_HASH_MISMATCH: file {actual_sha[:12]} "
                f"!= manifest {str(manifest_sha)[:12]}")
        return {
            "policies": policies,
            "snapshot_source": str(snapshot_file),
            "snapshot_sha256": actual_sha,
            "production_revision": rev,
            "binding_mode": mode,
            "snapshot_schema_version": schema,
        }
    # 2) historical revision binding (canonical 31d677e pair)
    if canonical:
        binding = _find_historical_binding(manifest_rev, run)
        if binding is not None:
            return _resolve_historical_binding(binding, manifest_rev)
        # fail-closed: no valid canonical source
        raise ValueError("SEMANTIC_PROVENANCE_MISSING: no run-bound or "
                         "historical-revision-bound semantic snapshot")
    return {
        "policies": {}, "snapshot_source": None, "snapshot_sha256": None,
        "production_revision": None, "binding_mode": BINDING_NONE,
        "snapshot_schema_version": None,
    }


def _resolve_historical_binding(binding, manifest_rev):
    """Resolve a validated historical revision binding (Prompt77/79)."""
    snap_path = REPO / binding.get("semantic_snapshot_source", "")
    if not snap_path.exists():
        raise ValueError("SEMANTIC_BINDING_SNAPSHOT_MISSING")
    data = yaml.safe_load(snap_path.read_text()) or {}
    snap_rev = data.get("production_revision")
    if snap_rev != manifest_rev:
        raise ValueError(
            f"SEMANTIC_SNAPSHOT_REVISION_MISMATCH: snapshot {snap_rev} "
            f"!= run {manifest_rev}")
    derived = data.get("derived_from_revision")
    if derived and derived != manifest_rev:
        raise ValueError(
            f"SEMANTIC_SNAPSHOT_REVISION_MISMATCH: derived {derived} "
            f"!= run {manifest_rev}")
    actual_sha = hashlib.sha256(snap_path.read_bytes()).hexdigest()
    bound_sha = binding.get("semantic_snapshot_sha256")
    if bound_sha and bound_sha != actual_sha:
        raise ValueError(
            f"SEMANTIC_SNAPSHOT_HASH_MISMATCH: binding {bound_sha} "
            f"!= file {actual_sha}")
    return {
        "policies": data.get("policies", {}),
        "snapshot_source": str(snap_path),
        "snapshot_sha256": hashlib.sha256(snap_path.read_bytes()).hexdigest(),
        "production_revision": snap_rev,
        "binding_mode": BINDING_HISTORICAL,
        "snapshot_schema_version": data.get("snapshot_schema_version"),
    }


def _find_historical_binding(production_revision, run_dir):
    """Locate a validated historical binding whose run matches the artifact
    (run_id and production_revision) — never stage-name generic."""
    if not RUN_BINDING_DIR.exists():
        return None
    for bfile in sorted(RUN_BINDING_DIR.glob("*.yaml")):
        data = yaml.safe_load(bfile.read_text()) or {}
        runs = data.get("runs", {})
        run_root = pathlib.Path(run_dir).resolve()
        candidates = {run_root, run_root.parent}
        for entry in runs.values():
            if (entry.get("production_revision") == production_revision
                    and entry.get("result_path")
                    and pathlib.Path(entry["result_path"]).resolve() in candidates):
                return data
    return None


def _load_semantic_snapshot(path=None):
    """Read the explicit machine-readable semantic snapshot (P1). Returns
    (policies dict, contracts dict, source_path, source_sha256) or None
    entries when the file is absent."""
    path = pathlib.Path(path) if path else DEFAULT_SEMANTIC_SNAPSHOT
    if not path.exists():
        return None, None, str(path), None
    data = yaml.safe_load(path.read_text()) or {}
    sha = hashlib.sha256(path.read_bytes()).hexdigest()
    return (data.get("policies", {}), data.get("stage_contracts", {}),
            str(path), sha)


def _resolve_actual_semantics(manifest_data, policies, snapshot_path,
                              snapshot_sha):
    """actual semantics: resolved run manifest fields + snapshot policy IDs.
    No stage-name inference, no evaluator constants, no fallback defaults."""
    actual = {}
    for key in MANIFEST_SEMANTIC_FIELDS:
        actual[key] = manifest_data.get(key, "SEMANTIC_PROVENANCE_MISSING")
    for key in POLICY_ID_KEYS:
        actual[key] = policies.get(key, "SEMANTIC_PROVENANCE_MISSING")
    actual["semantic_source_path"] = snapshot_path
    actual["semantic_source_sha256"] = snapshot_sha
    actual["semantic_profile_revision"] = manifest_data.get(
        "semantic_profile_revision", "SEMANTIC_PROVENANCE_MISSING")
    # Prompt77 §18: per-semantic-group provenance (run manifest vs snapshot).
    actual["_provenance"] = {
        "runtime_fields": {k: "run_manifest" for k in MANIFEST_SEMANTIC_FIELDS},
        "policy_fields": {k: "semantic_snapshot" for k in POLICY_ID_KEYS},
        "semantic_snapshot_sha256": snapshot_sha,
    }
    return actual


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
                    ate_evaluator=None, expected_rows=None,
                    semantic_snapshot=None, legacy_mode=False):
    run = pathlib.Path(run_dir)
    node_log = run / "node_stdout.log"
    text = node_log.read_text(errors="replace") if node_log.exists() else ""

    manifest_data = {}
    if manifest and pathlib.Path(manifest).exists():
        manifest_data = yaml.safe_load(pathlib.Path(manifest).read_text()) or {}

    prov, prov_status = _load_run_provenance(run)
    # Prompt77: canonical semantic policy source resolution — the
    # current-checkout template is NEVER a canonical historical source.
    try:
        bound = _resolve_run_semantics(run, manifest_data, canonical=True)
    except ValueError as exc:
        if not legacy_mode:
            raise
        bound = {
            "policies": {}, "snapshot_source": None, "snapshot_sha256": None,
            "production_revision": None, "binding_mode": BINDING_NONE,
            "snapshot_schema_version": None,
        }
        bound["_legacy_error"] = str(exc)
    # Prompt76: actual semantics from the RUN (manifest + bound snapshot).
    actual = _resolve_actual_semantics(manifest_data, bound.get("policies", {}),
                                       bound.get("snapshot_source"),
                                       bound.get("snapshot_sha256"))
    expected = (SEMANTIC_STAGE_CONTRACTS or {}).get(stage, {})
    missing_semantics = any(
        actual[k] == "SEMANTIC_PROVENANCE_MISSING" for k in
        MANIFEST_SEMANTIC_FIELDS + POLICY_ID_KEYS)
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
            "result_path": str(run),
        },
        # Prompt76 §5/§21: ACTUAL run semantics (authoritative) vs the
        # expected stage contract (validator metadata) kept separately.
        "actual_semantics": actual,
        "expected_stage_semantics": expected,
        "semantic_provenance": {
            "semantic_binding_mode": bound.get("binding_mode"),
            "semantic_snapshot_source": bound.get("snapshot_source"),
            "semantic_snapshot_sha256": bound.get("snapshot_sha256"),
            "semantic_snapshot_production_revision": bound.get("production_revision"),
            "semantic_snapshot_schema_version": bound.get("snapshot_schema_version"),
            "manifest_semantic_profile": manifest_data.get("semantic_profile"),
            "semantic_profile_revision": manifest_data.get(
                "semantic_profile_revision"),
            "manifest_path": str(pathlib.Path(manifest).resolve()) if manifest
                            and pathlib.Path(manifest).exists() else "EVIDENCE_MISSING",
            "complete": (not missing_semantics
                         and bound.get("snapshot_sha256") is not None
                         and bound.get("binding_mode") in
                         (BINDING_RUN_EMBEDDED, BINDING_RUN_REFERENCED,
                          BINDING_HISTORICAL)
                         and manifest_data.get("semantic_profile") is not None),
            "mode": "LEGACY" if legacy_mode else "CANONICAL",
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
    ap.add_argument("--semantic-snapshot",
                    default=str(DEFAULT_SEMANTIC_SNAPSHOT),
                    help="explicit machine-readable semantic snapshot (P1)")
    ap.add_argument("--legacy-mode", action="store_true",
                    help="legacy mode: missing historical fields allowed as "
                         "LEGACY_NOT_CAPTURED (A0/A1)")
    ap.add_argument("--ate-evaluator",
                    default=str(REPO / "scripts/super_livo/evaluation/ntu_viral_official_ate.py"))
    args = ap.parse_args(argv)
    score = build_scorecard(args.stage, args.run_dir, args.manifest,
                            args.trajectory, args.gt, args.ate_evaluator,
                            args.expected_rows, args.semantic_snapshot,
                            args.legacy_mode)
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
