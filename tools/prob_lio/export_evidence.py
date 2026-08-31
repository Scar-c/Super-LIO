#!/usr/bin/env python3
"""Export compact evidence from a local Prompt12 runtime directory.

The runtime directory is allowed to contain full trajectories, derived GT,
logs, and other disposable outputs.  This command copies only the bounded
evidence files that are suitable for the Git-tracked evidence tree.
"""

import argparse
import pathlib
import shutil
import sys


COMPACT_FILES = {
    "run_manifest.yaml",
    "preflight.yaml",
    "requested_effective_config.yaml",
    "effective_rosparams.yaml",
    "evaluation.yaml",
    "evaluation.txt",
    "config_diff.yaml",
    "hygiene_check.yaml",
    "instrumentation_check.yaml",
    "provenance.yaml",
}


def is_compact(path):
    return path.name in COMPACT_FILES or path.name.endswith("_isolation.yaml")


def export_evidence(runtime_dir, evidence_dir, overwrite=False):
    runtime_dir = pathlib.Path(runtime_dir).resolve()
    evidence_dir = pathlib.Path(evidence_dir).resolve()
    manifest = runtime_dir / "run_manifest.yaml"
    if not manifest.is_file():
        raise FileNotFoundError(f"runtime manifest missing: {manifest}")
    if evidence_dir.exists() and any(evidence_dir.iterdir()) and not overwrite:
        raise FileExistsError(f"refusing to overwrite evidence: {evidence_dir}")
    evidence_dir.mkdir(parents=True, exist_ok=True)
    copied = []
    for path in sorted(runtime_dir.iterdir()):
        if path.is_file() and is_compact(path):
            destination = evidence_dir / path.name
            shutil.copy2(path, destination)
            copied.append(path.name)
    if "run_manifest.yaml" not in copied:
        raise RuntimeError("run_manifest.yaml was not exported")
    return copied


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime-dir", required=True)
    parser.add_argument("--evidence-dir", required=True)
    parser.add_argument("--overwrite", action="store_true")
    args = parser.parse_args(argv)
    try:
        copied = export_evidence(args.runtime_dir, args.evidence_dir, args.overwrite)
    except (FileExistsError, FileNotFoundError, RuntimeError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(f"exported {len(copied)} compact files to {pathlib.Path(args.evidence_dir).resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
