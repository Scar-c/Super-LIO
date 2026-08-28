#!/usr/bin/env python3
"""Fail-closed canonical experiment semantic-profile resolver/validator."""
import argparse
import copy
import pathlib
import subprocess
import sys

import yaml

PROTECTED_FIELDS = (
    "scheduler_family", "camera_input_enabled", "camera_epoch_enabled",
    "visual_frontend_enabled", "visual_map_producer_enabled",
    "visual_measurement_enabled", "visual_state_apply", "raw_lidar_policy",
    "full_lidar_observe_per_raw_scan", "camera_stride",
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
}
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
    },
    "D_VISUAL_SHADOW": _SHADOW,
    "D_VISUAL_APPLY": {**_SHADOW, "visual_state_apply": True},
}

class SemanticProfileError(ValueError):
    pass

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
                **copy.deepcopy(PROFILES[profile]), "camera_stride": stride,
                "dataset": dataset, "sequence": sequence,
                "config_provenance": dict(provenance or {})}
    manifest.update(dict(revisions or {}))
    manifest.update(overrides)
    validate_manifest(manifest)
    return manifest

def validate_manifest(manifest):
    profile = manifest.get("semantic_profile")
    if profile not in PROFILES:
        raise SemanticProfileError("semantic_profile missing or unknown")
    missing = [key for key in PROTECTED_FIELDS if key not in manifest]
    if missing:
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
        else:
            for key, value in rosparams_for(load_manifest(args.manifest), args.out_dir):
                print(f"{key}\t{value}")
    except SemanticProfileError as exc:
        print(f"SEMANTIC_PROFILE_FAIL: {exc}", file=sys.stderr)
        return 2
    return 0

if __name__ == "__main__": sys.exit(main())
