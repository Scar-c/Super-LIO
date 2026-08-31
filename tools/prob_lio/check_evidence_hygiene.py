#!/usr/bin/env python3
"""Check Prompt12's compact evidence and tracked-artifact policy."""

import argparse
import hashlib
import pathlib
import re
import subprocess
import sys

import yaml

from export_evidence import is_compact


FORBIDDEN_RE = re.compile(
    r"(?:\.bag$|/(?:node\.log|roscore[^/]*\.log|roslaunch[^/]*\.log|"
    r"stdout\.log|stderr\.log|play\.log|record\.log|trajectory\.tum|"
    r"ground_truth\.tum|assoc_shadow_frames\.csv)$|(?:^|/)build(?:/|$)|"
    r"(?:^|/)devel(?:/|$)|(?:^|/)raw_dump)", re.IGNORECASE
)


def sha256(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def check_manifest(runtime_dir, evidence_dir):
    manifest_path = pathlib.Path(runtime_dir) / "run_manifest.yaml"
    document = yaml.safe_load(manifest_path.read_text(encoding="utf-8")) or {}
    trajectory = document.get("trajectory") or {}
    required = ["sha256", "path"]
    missing = [key for key in required if not trajectory.get(key)]
    missing += [key for key in ("rows", "completion", "metric") if key not in document]
    if missing:
        return [f"manifest missing required fields: {', '.join(missing)}"]
    trajectory_path = pathlib.Path(trajectory["path"])
    errors = []
    if not trajectory_path.is_file():
        errors.append(f"manifest trajectory path missing: {trajectory_path}")
    elif sha256(trajectory_path) != trajectory["sha256"]:
        errors.append("manifest trajectory sha256 does not match local runtime file")
    evidence_manifest = pathlib.Path(evidence_dir) / "run_manifest.yaml"
    if not evidence_manifest.is_file():
        errors.append(f"evidence manifest missing: {evidence_manifest}")
    return errors


def git_forbidden_paths(repo_root):
    result = subprocess.run(
        ["git", "-C", str(repo_root), "ls-files", "-z"],
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        return ["unable to inspect tracked files"]
    paths = result.stdout.decode().split("\0")
    return [path for path in paths if path and FORBIDDEN_RE.search("/" + path)]


def check_evidence(runtime_dir, evidence_dir, repo_root=None, skip_git=False):
    runtime_dir = pathlib.Path(runtime_dir).resolve()
    evidence_dir = pathlib.Path(evidence_dir).resolve()
    errors = []
    if not runtime_dir.is_dir():
        errors.append(f"runtime directory missing: {runtime_dir}")
    if not evidence_dir.is_dir():
        errors.append(f"evidence directory missing: {evidence_dir}")
    if errors:
        return errors
    for path in evidence_dir.rglob("*"):
        if path.is_file() and not is_compact(path):
            errors.append(f"non-compact evidence file: {path.relative_to(evidence_dir)}")
    errors.extend(check_manifest(runtime_dir, evidence_dir))
    if not skip_git:
        errors.extend(
            f"forbidden tracked artifact: {path}"
            for path in git_forbidden_paths(pathlib.Path(repo_root).resolve())
        )
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime-dir", required=True)
    parser.add_argument("--evidence-dir", required=True)
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--skip-git", action="store_true")
    args = parser.parse_args(argv)
    errors = check_evidence(
        args.runtime_dir, args.evidence_dir, args.repo_root, args.skip_git
    )
    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("Prompt12 evidence hygiene: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
