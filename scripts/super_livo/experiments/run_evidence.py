#!/usr/bin/env python3
"""Capture and validate canonical-run evidence for Super-LIVO.

The validator is deliberately fail-closed: a run without both effective
configuration snapshots is never presented as fully comparable.
"""

import argparse
import hashlib
import shlex
import subprocess
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable, Optional, Sequence, Tuple

import yaml


INCOMPLETE = "CONFIG_EVIDENCE_INCOMPLETE"
COMPLETE = "CONFIG_EVIDENCE_COMPLETE"


@dataclass(frozen=True)
class ValidationResult:
    complete: bool
    status: str
    reasons: Tuple[str, ...]


def now_utc() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _file_identity(path: Path) -> dict:
    path = Path(path).resolve()
    return {
        "path": str(path),
        "size_bytes": path.stat().st_size,
        "sha256": sha256_file(path),
    }


def _git(repo_root: Path, *args: str) -> str:
    completed = subprocess.run(
        ["git", "-C", str(repo_root), *args],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return completed.stdout.rstrip("\n")


def _write_yaml(path: Path, data: dict) -> None:
    temp = path.with_suffix(path.suffix + ".tmp")
    temp.write_text(yaml.safe_dump(data, sort_keys=False), encoding="utf-8")
    temp.replace(path)


def _write_dirty_artifact(run_dir: Path, repo_root: Path, status: str) -> dict:
    tracked_diff = _git(repo_root, "diff", "--binary", "HEAD")
    lines = ["# git diff --binary HEAD", tracked_diff, "", "# git status --porcelain", status]
    untracked = []
    for line in status.splitlines():
        if not line.startswith("?? "):
            continue
        relative = line[3:].strip()
        if relative.startswith('"'):
            # Quoted porcelain paths can contain C-style escapes; retain the
            # exact status entry rather than guessing a filesystem path.
            untracked.append({"status_entry": relative, "sha256": "UNRESOLVED_QUOTED_PATH"})
            continue
        path = repo_root / relative
        identity = {"status_entry": relative}
        if path.is_file():
            identity.update({"size_bytes": path.stat().st_size, "sha256": sha256_file(path)})
        untracked.append(identity)
    lines.extend(["", "# untracked identities", yaml.safe_dump(untracked, sort_keys=False)])
    artifact = run_dir / "git_diff.patch"
    artifact.write_text("\n".join(lines), encoding="utf-8")
    return {
        "path": str(artifact.resolve()),
        "sha256": sha256_file(artifact),
        "untracked": untracked,
    }


def begin_provenance(
    *,
    run_dir: Path,
    repo_root: Path,
    dataset: str,
    sequence: str,
    variant: str,
    command_line: str,
    source_config: Path,
    bags: Sequence[Path],
    camera_calibration: Optional[Path] = None,
) -> dict:
    run_dir = Path(run_dir)
    repo_root = Path(repo_root).resolve()
    run_dir.mkdir(parents=True, exist_ok=True)
    status = _git(repo_root, "status", "--porcelain")
    dirty_artifact = _write_dirty_artifact(run_dir, repo_root, status)
    manifest = {
        "schema_version": 1,
        "run": {
            "dataset": dataset,
            "sequence": sequence,
            "variant": variant,
            "git_head": _git(repo_root, "rev-parse", "HEAD"),
            "git_dirty": bool(status),
            "git_status_porcelain": status.splitlines(),
            "command_line": command_line,
            "start_utc": now_utc(),
        },
        "source_config": _file_identity(source_config),
        "inputs": [_file_identity(path) for path in bags],
        "camera_calibration": (
            _file_identity(camera_calibration)
            if camera_calibration is not None and Path(camera_calibration).is_file()
            else {"path": "NOT_APPLICABLE", "sha256": "NOT_APPLICABLE"}
        ),
        "dirty_artifact": dirty_artifact,
        "environment": {
            "ros_distro": __import__("os").environ.get("ROS_DISTRO", "unknown"),
            "python": __import__("sys").version.split()[0],
        },
        "evaluator": {"status": "NOT_ATTACHED"},
        "completion": {"status": "RUNNING"},
    }
    _write_yaml(run_dir / "run_provenance.yaml", manifest)
    return manifest


def finalize_provenance(
    *, run_dir: Path, source_config: Path, process_return_code: int
) -> ValidationResult:
    run_dir = Path(run_dir)
    manifest_path = run_dir / "run_provenance.yaml"
    manifest = yaml.safe_load(manifest_path.read_text(encoding="utf-8")) or {}
    result = validate_run_dir(run_dir, source_config=source_config)
    for key, filename in (
        ("effective_rosparams_pre_node", "effective_rosparams.pre_node.yaml"),
        ("effective_config_post_resolve", "effective_config.post_resolve.yaml"),
        ("trajectory", "trajectory.tum"),
    ):
        path = run_dir / filename
        manifest[key] = _file_identity(path) if path.is_file() else {
            "path": str(path.resolve()),
            "status": "MISSING",
        }
    if process_return_code != 0:
        status = "FAILED"
    elif not result.complete:
        status = INCOMPLETE
    elif not (run_dir / "trajectory.tum").is_file():
        status = "FAILED_MISSING_TRAJECTORY"
    else:
        status = "COMPLETE"
    manifest["completion"] = {
        "status": status,
        "process_return_code": int(process_return_code),
        "config_evidence_status": result.status,
        "config_evidence_reasons": list(result.reasons),
        "end_utc": now_utc(),
    }
    _write_yaml(manifest_path, manifest)
    return result


def dump_pre_node(path: Path) -> None:
    """Dump deterministic experiment-relevant ROS parameter roots."""
    snapshot = {}
    for root in ("/camera", "/lio"):
        completed = subprocess.run(
            ["rosparam", "get", root],
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        snapshot[root[1:]] = yaml.safe_load(completed.stdout) or {}
    path = Path(path)
    path.write_text(yaml.safe_dump(snapshot, sort_keys=True), encoding="utf-8")
    Path(str(path) + ".sha256").write_text(
        f"{sha256_file(path)}  {path.name}\n", encoding="utf-8"
    )


def validate_run_dir(
    run_dir: Path, source_config: Optional[Path] = None
) -> ValidationResult:
    run_dir = Path(run_dir)
    required = (
        "effective_rosparams.pre_node.yaml",
        "effective_config.post_resolve.yaml",
    )
    missing = tuple(name for name in required if not (run_dir / name).is_file())
    if missing:
        return ValidationResult(False, INCOMPLETE, missing)
    if source_config is not None:
        source_config = Path(source_config)
        post = run_dir / "effective_config.post_resolve.yaml"
        if source_config.is_file() and source_config.read_bytes() == post.read_bytes():
            return ValidationResult(
                False, INCOMPLETE, ("post_resolve_is_source_yaml_copy",)
            )
    try:
        pre = yaml.safe_load(
            (run_dir / "effective_rosparams.pre_node.yaml").read_text(
                encoding="utf-8"
            )
        ) or {}
        post_data = yaml.safe_load(
            (run_dir / "effective_config.post_resolve.yaml").read_text(
                encoding="utf-8"
            )
        ) or {}
    except (OSError, yaml.YAMLError):
        return ValidationResult(False, INCOMPLETE, ("snapshot_parse_error",))

    pre_stride = (pre.get("camera") or {}).get("temporal_stride")
    post_stride = (
        ((post_data.get("input") or {}).get("camera_temporal_stride") or {})
        .get("value")
    )
    if pre_stride is not None and pre_stride != post_stride:
        return ValidationResult(
            False,
            INCOMPLETE,
            ("post_resolve_mismatch:camera.temporal_stride",),
        )
    pre_camera_enabled = (pre.get("camera") or {}).get("enabled")
    post_camera_enabled = (
        ((post_data.get("input") or {}).get("camera_enabled") or {}).get("value")
    )
    if (
        pre_camera_enabled is not None
        and post_camera_enabled is not None
        and pre_camera_enabled != post_camera_enabled
    ):
        return ValidationResult(
            False, INCOMPLETE, ("post_resolve_mismatch:camera.enabled",)
        )
    frame_capacity = (
        ((post_data.get("estimator") or {}).get("camera_frame_buffer_capacity") or {})
        .get("value")
    )
    if frame_capacity is None:
        return ValidationResult(
            False,
            INCOMPLETE,
            ("missing_resolved_default:camera_frame_buffer_capacity",),
        )
    timestamp_unit = (
        ((post_data.get("lidar") or {}).get("point_timestamp_unit") or {})
    )
    if timestamp_unit.get("source") != "derived" or not timestamp_unit.get("value"):
        return ValidationResult(
            False,
            INCOMPLETE,
            ("derived_source_missing:lidar.point_timestamp_unit",),
        )
    pre_offset = (pre.get("camera") or {}).get("time_offset")
    post_offset = (
        ((post_data.get("time") or {}).get("camera_offset_s") or {}).get("value")
    )
    if pre_offset is not None and pre_offset != post_offset:
        return ValidationResult(
            False, INCOMPLETE, ("post_resolve_mismatch:camera.time_offset",)
        )
    pre_policy = (
        (((pre.get("lio") or {}).get("camera_epoch") or {})
         .get("lidar_update_policy"))
    )
    post_policy = (
        ((post_data.get("estimator") or {}).get("lidar_update_policy") or {})
        .get("value")
    )
    if pre_policy is not None and pre_policy != post_policy:
        return ValidationResult(
            False, INCOMPLETE, ("post_resolve_mismatch:lidar_update_policy",)
        )
    return ValidationResult(True, COMPLETE, ())


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    dump = sub.add_parser("dump-pre")
    dump.add_argument("--out", type=Path, required=True)

    begin = sub.add_parser("begin")
    begin.add_argument("--run-dir", type=Path, required=True)
    begin.add_argument("--repo-root", type=Path, required=True)
    begin.add_argument("--dataset", required=True)
    begin.add_argument("--sequence", required=True)
    begin.add_argument("--variant", required=True)
    begin.add_argument("--command-line", required=True)
    begin.add_argument("--source-config", type=Path, required=True)
    begin.add_argument("--bag", type=Path, action="append", required=True)
    begin.add_argument("--camera-calibration", type=Path)

    final = sub.add_parser("finalize")
    final.add_argument("--run-dir", type=Path, required=True)
    final.add_argument("--source-config", type=Path, required=True)
    final.add_argument("--process-return-code", type=int, required=True)
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = _parser().parse_args(argv)
    if args.command == "dump-pre":
        dump_pre_node(args.out)
        return 0
    if args.command == "begin":
        begin_provenance(
            run_dir=args.run_dir,
            repo_root=args.repo_root,
            dataset=args.dataset,
            sequence=args.sequence,
            variant=args.variant,
            command_line=args.command_line,
            source_config=args.source_config,
            bags=args.bag,
            camera_calibration=args.camera_calibration,
        )
        return 0
    result = finalize_provenance(
        run_dir=args.run_dir,
        source_config=args.source_config,
        process_return_code=args.process_return_code,
    )
    print(result.status)
    return 0 if result.complete and args.process_return_code == 0 else 5


if __name__ == "__main__":
    raise SystemExit(main())
