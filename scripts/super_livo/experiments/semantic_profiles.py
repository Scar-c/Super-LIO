#!/usr/bin/env python3
"""Fail-closed canonical experiment semantic-profile resolver/validator."""
import argparse
import copy
import pathlib
import subprocess
import sys

import yaml

SCHEMA_VERSION = "2"
# Prompt64 event-placement schema: WHAT/WHERE/WHEN/OWNERSHIP/EXACT-ONCE.
# visual_measurement_event:      estimator event where Visual producer/query/
#                                residual/H-b construction executes
# visual_measurement_timestamp_semantics: the estimator state/covariance epoch
#                                around which that measurement is linearized
# visual_measurement_exact_once: one eligible logical camera measurement is
#                                executed once, never duplicated at another
#                                estimator callback
# camera_payload_ownership_mode: lifecycle contract governing the camera
#                                payload through the declared measurement event
PROTECTED_FIELDS = (
    "scheduler_family", "camera_input_enabled", "camera_epoch_enabled",
    "visual_frontend_enabled", "visual_map_producer_enabled",
    "visual_measurement_enabled", "visual_state_apply", "raw_lidar_policy",
    "full_lidar_observe_per_raw_scan", "camera_stride",
    "visual_measurement_event", "visual_measurement_timestamp_semantics",
    "visual_measurement_exact_once", "camera_payload_ownership_mode",
)
REVISION_FIELDS = (
    "production_revision", "semantic_profile_revision", "dataset_adapter_revision",
    "transaction_revision",
)
PROVENANCE_FIELDS = ("lio", "visual", "dataset_calibration")

_SHADOW = {
    "scheduler_family": "D_CORRECTED",
    "camera_input_enabled": True,
    "camera_epoch_enabled": True,
    "visual_frontend_enabled": True,
    "visual_map_producer_enabled": True,
    "visual_measurement_enabled": True,
    "visual_state_apply": False,
    "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
    "full_lidar_observe_per_raw_scan": 1,
    "visual_measurement_event": "FULL_LIDAR_OBSERVE_CALLBACK",
    "visual_measurement_timestamp_semantics": "LIDAR_OBSERVE_CONVERGENCE_STATE",
    "visual_measurement_exact_once": True,
    "camera_payload_ownership_mode": "POP_AT_CAMERA_EPOCH",
}
# Truthful representation of current Prompt60 production capability (source:
# round13_current_d_event_source_audit.md): the accepted Visual H/b executes
# in the full-LiDAR Observe convergence callback, NOT at the camera epoch.
EFFECTIVE_PRODUCTION_CAPABILITY = {
    "visual_measurement_event": "FULL_LIDAR_OBSERVE_CALLBACK",
    "visual_measurement_timestamp_semantics": "LIDAR_OBSERVE_CONVERGENCE_STATE",
    "camera_payload_ownership_mode": "POP_AT_CAMERA_EPOCH",
    # Prompt60: one measurement event per eligible frame in the callback
    # (measurement frames 823 counted once; no duplicate estimator callback).
    "visual_measurement_exact_once": True,
}
# Requested future intent: camera-event architecture (payload retained through
# the camera epoch measurement). Resolves as a definition; FAILS executability
# until production capability matches (RP-T7, no silent degradation).
PROFILES = {
    "D_SCHEDULER_BASE": {
        "scheduler_family": "D_CORRECTED",
        "camera_input_enabled": None,
        "camera_epoch_enabled": None,
        "visual_frontend_enabled": None,
        "visual_map_producer_enabled": None,
        "visual_measurement_enabled": None,
        "visual_state_apply": None,
        "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
        "full_lidar_observe_per_raw_scan": 1,
        "visual_measurement_event": "NONE",
        "visual_measurement_timestamp_semantics": "NONE",
        "visual_measurement_exact_once": None,
        "camera_payload_ownership_mode": "NONE",
    },
    "D_VISUAL_SHADOW": _SHADOW,
    "D_VISUAL_APPLY": {
        **_SHADOW, "visual_state_apply": True,
        "visual_measurement_event": "CAMERA_EPOCH",
        "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
        "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
    },
}

class SemanticProfileError(ValueError):
    pass

# Repository root anchored to this module's own location: validator path
# resolution is CWD-independent and survives repository relocation.
REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]

VALIDATOR_NAMESPACE = REPO_ROOT / "scripts" / "super_livo" / "experiments"

def resolve_validator_path(relative):
    """Anchor a repository-relative validator contract to REPO_ROOT.
    The manifest stores the portable relative path; runtime resolution is
    absolute and deterministic regardless of caller CWD. Prompt67: the
    resolved path must stay inside the approved repository validator
    namespace; absolute paths, traversal and symlink escapes are rejected."""
    if not relative:
        return ""
    p = pathlib.Path(relative)
    if p.is_absolute():
        raise SemanticProfileError(
            f"validator path must be repository-relative, got absolute: {relative}")
    candidate = (REPO_ROOT / p).resolve()
    try:
        candidate.relative_to(VALIDATOR_NAMESPACE.resolve())
    except ValueError:
        raise SemanticProfileError(
            f"validator path escapes approved namespace: {relative} "
            f"(resolved {candidate})") from None
    return str(candidate)

# Profile-associated post-run validator contract (Prompt64 §25): selection
# derives from the resolved semantic profile, never from a hardcoded
# transaction-supervisor branch. Validators accept --log/--manifest/--out.
VALIDATOR_CONTRACT = {
    # (validator path, requires_measurement_evidence)
    "D_VISUAL_SHADOW": ("scripts/super_livo/experiments/validate_d_visual_shadow_result.py", True),
}

def validator_contract_for(profile):
    entry = VALIDATOR_CONTRACT.get(profile)
    if not entry:
        return "", False
    return entry[0], entry[1]

def protected_projection(manifest):
    return {key: manifest.get(key) for key in PROTECTED_FIELDS}

def semantic_diff(a, b):
    ignored = {"legacy_alias", "dataset", "sequence", "config_provenance", *REVISION_FIELDS}
    return {key for key in set(a) | set(b) if key not in ignored and a.get(key) != b.get(key)}

def resolve_profile(profile, *, legacy_alias="", dataset="", sequence="", camera_stride=1,
                    revisions=None, provenance=None, adapter_overrides=None):
    if profile not in PROFILES:
        raise SemanticProfileError(f"unknown semantic profile: {profile}")
    overrides = adapter_overrides or {}
    illegal = sorted(set(overrides) & set(PROTECTED_FIELDS))
    if illegal:
        raise SemanticProfileError("dataset adapter overrides protected fields: " + ", ".join(illegal))
    try:
        stride = int(camera_stride)
    except (TypeError, ValueError) as exc:
        raise SemanticProfileError("camera_stride must be an integer") from exc
    if stride < 1:
        raise SemanticProfileError("camera_stride must be >= 1")
    manifest = {"semantic_profile": profile, "legacy_alias": legacy_alias,
                "semantic_schema_version": SCHEMA_VERSION,
                "validator": validator_contract_for(profile)[0],
                "requires_measurement_evidence": validator_contract_for(profile)[1],
                **copy.deepcopy(PROFILES[profile]), "camera_stride": stride,
                "dataset": dataset, "sequence": sequence,
                "config_provenance": dict(provenance or {})}
    manifest.update(dict(revisions or {}))
    manifest.update(overrides)
    validate_manifest(manifest)
    return manifest

def validate_executability(manifest):
    """Requested profile semantics must match the effective production
    capability. No silent degradation (Prompt64 §15/§16)."""
    if manifest.get("semantic_profile") == "D_SCHEDULER_BASE":
        return True  # descriptive only; rosparams_for already rejects it
    if not manifest.get("visual_measurement_enabled"):
        return True
    for key, effective in EFFECTIVE_PRODUCTION_CAPABILITY.items():
        requested = manifest.get(key)
        if requested in (None, "NONE"):
            raise SemanticProfileError(f"{key} unresolved for measurement-enabled profile")
        if requested != effective:
            raise SemanticProfileError(
                f"requested {key}={requested} but effective production "
                f"capability is {effective}; SEMANTIC_PROFILE_FAIL, NO PLAYBACK")
    return True

def validate_manifest(manifest):
    profile = manifest.get("semantic_profile")
    if profile not in PROFILES:
        raise SemanticProfileError("semantic_profile missing or unknown")
    if profile in VALIDATOR_CONTRACT:
        expected_validator, expected_evidence = VALIDATOR_CONTRACT[profile]
        if manifest.get("validator") != expected_validator:
            raise SemanticProfileError(
                f"validator contract integrity: manifest validator={manifest.get('validator')!r} "
                f"!= canonical {expected_validator!r} for {profile}")
        if manifest.get("requires_measurement_evidence") != expected_evidence:
            raise SemanticProfileError(
                f"evidence contract integrity: manifest requires_measurement_evidence="
                f"{manifest.get('requires_measurement_evidence')!r} != canonical "
                f"{expected_evidence!r} for {profile}")
    missing = [key for key in PROTECTED_FIELDS if key not in manifest]
    if missing:
        legacy_missing = [k for k in missing if k in (
            "visual_measurement_event", "visual_measurement_timestamp_semantics",
            "visual_measurement_exact_once", "camera_payload_ownership_mode")]
        if legacy_missing and manifest.get("semantic_schema_version") == "1":
            # Prompt59/60 manifests: SCHEMA_LEGACY, provenance only. Absent
            # event fields are NOT resolved to any assumed value; executability
            # gate below rejects playback.
            return True
        raise SemanticProfileError("missing protected fields: " + ", ".join(missing))
    if profile != "D_SCHEDULER_BASE":
        expected = {**PROFILES[profile], "camera_stride": manifest["camera_stride"]}
        mismatch = [key for key, value in expected.items() if manifest.get(key) != value]
        if mismatch:
            raise SemanticProfileError("semantic profile mismatch: " + ", ".join(mismatch))
    if not isinstance(manifest["camera_stride"], int) or manifest["camera_stride"] < 1:
        raise SemanticProfileError("invalid camera_stride")
    missing_revisions = [key for key in REVISION_FIELDS if not manifest.get(key)]
    if missing_revisions:
        raise SemanticProfileError("missing revisions: " + ", ".join(missing_revisions))
    provenance = manifest.get("config_provenance")
    if not isinstance(provenance, dict):
        raise SemanticProfileError("config_provenance missing")
    missing_provenance = [key for key in PROVENANCE_FIELDS if not provenance.get(key)]
    if missing_provenance:
        raise SemanticProfileError("missing config provenance: " + ", ".join(missing_provenance))
    return True

def write_manifest(manifest, path):
    validate_manifest(manifest)
    target = pathlib.Path(path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(yaml.safe_dump(manifest, sort_keys=False), encoding="utf-8")

def load_manifest(path):
    data = yaml.safe_load(pathlib.Path(path).read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise SemanticProfileError("manifest is not a mapping")
    return data

def git_revision(repo):
    return subprocess.check_output(["git", "-C", str(repo), "rev-parse", "HEAD"], text=True).strip()

def rosparams_for(manifest, out_dir):
    validate_manifest(manifest)
    if manifest["semantic_profile"] == "D_SCHEDULER_BASE":
        raise SemanticProfileError("D_SCHEDULER_BASE is descriptive, not executable")
    boolean = lambda value: "true" if value else "false"
    return [
        ("/camera/enabled", boolean(manifest["camera_input_enabled"])),
        ("/lio/camera_epoch/enabled", boolean(manifest["camera_epoch_enabled"])),
        ("/lio/v0/enabled", boolean(manifest["visual_frontend_enabled"])),
        ("/lio/g0/shadow", boolean(manifest["visual_map_producer_enabled"])),
        ("/lio/g1/enabled", boolean(manifest["visual_map_producer_enabled"])),
        ("/lio/g1/out_dir", str(out_dir)),
        ("/lio/v2/enabled", boolean(manifest["visual_measurement_enabled"])),
        ("/lio/v4/apply", boolean(manifest["visual_state_apply"])),
        # profile-layer default: V-4R0 outlier gate OFF for all normalized
        # profiles (not part of the protected event schema; legacy a1 only).
        ("/lio/v4/outlier_gate", "false"),
        ("/lio/camera_epoch/lidar_update_policy", "imu_fullscan"),
        ("/camera/temporal_stride", str(manifest["camera_stride"])),
    ]

def main(argv=None):
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    resolve = sub.add_parser("resolve")
    for name in ("profile", "legacy-alias", "dataset", "sequence", "camera-stride", "out",
                 "production-revision", "semantic-profile-revision", "dataset-adapter-revision",
                 "transaction-revision", "lio-provenance", "visual-provenance",
                 "dataset-calibration-provenance"):
        resolve.add_argument("--" + name, required=True)
    validate = sub.add_parser("validate"); validate.add_argument("--manifest", required=True)
    executable = sub.add_parser("check-executable"); executable.add_argument("--manifest", required=True)
    validator = sub.add_parser("validator"); validator.add_argument("--manifest", required=True)
    evreq = sub.add_parser("evidence-required"); evreq.add_argument("--manifest", required=True)
    params = sub.add_parser("rosparams"); params.add_argument("--manifest", required=True); params.add_argument("--out-dir", required=True)
    args = parser.parse_args(argv)
    try:
        if args.command == "resolve":
            manifest = resolve_profile(
                args.profile, legacy_alias=args.legacy_alias, dataset=args.dataset,
                sequence=args.sequence, camera_stride=args.camera_stride,
                revisions={key: getattr(args, key) for key in REVISION_FIELDS},
                provenance={"lio": args.lio_provenance, "visual": args.visual_provenance,
                            "dataset_calibration": args.dataset_calibration_provenance})
            write_manifest(manifest, args.out)
        elif args.command == "validate":
            validate_manifest(load_manifest(args.manifest))
        elif args.command == "check-executable":
            validate_executability(load_manifest(args.manifest))
        elif args.command == "evidence-required":
            manifest = load_manifest(args.manifest)
            print("1" if manifest.get("requires_measurement_evidence") else "0")
        elif args.command == "validator":
            manifest = load_manifest(args.manifest)
            contract = manifest.get("validator", "")
            if not contract:
                raise SemanticProfileError("no validator contract for profile "
                                           + manifest.get("semantic_profile", "?"))
            print(resolve_validator_path(contract))
        else:
            for key, value in rosparams_for(load_manifest(args.manifest), args.out_dir):
                print(f"{key}\t{value}")
    except SemanticProfileError as exc:
        print(f"SEMANTIC_PROFILE_FAIL: {exc}", file=sys.stderr)
        return 2
    return 0

if __name__ == "__main__": sys.exit(main())
