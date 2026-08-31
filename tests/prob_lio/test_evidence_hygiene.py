#!/usr/bin/env python3
"""Public-CLI tests for Prompt12's runtime/evidence boundary."""

import hashlib
import json
import pathlib
import subprocess
import sys

import yaml


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
EXPORTER = REPO_ROOT / "tools/prob_lio/export_evidence.py"
CHECKER = REPO_ROOT / "tools/prob_lio/check_evidence_hygiene.py"


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def test_exporter_copies_only_compact_evidence(tmp_path):
    runtime = tmp_path / "runtime" / "sbs_B0"
    evidence = tmp_path / "evidence" / "sbs_B0"
    runtime.mkdir(parents=True)
    trajectory = runtime / "trajectory.tum"
    trajectory.write_text("0.0 0 0 0 0 0 0 1\n", encoding="utf-8")
    manifest = {
        "schema_version": 2,
        "status": "CANONICAL_VALID",
        "variant_id": "B0",
        "completion": True,
        "trajectory": {
            "path": str(trajectory),
            "exists": True,
            "size_bytes": trajectory.stat().st_size,
            "sha256": sha256(trajectory),
        },
        "rows": 1,
        "metric": {"primary_metric": "ATE", "value": 0.1, "unit": "m"},
    }
    (runtime / "run_manifest.yaml").write_text(
        yaml.safe_dump(manifest, sort_keys=False), encoding="utf-8"
    )
    (runtime / "preflight.yaml").write_text("status: PREFLIGHT_OK\n", encoding="utf-8")
    (runtime / "effective_rosparams.yaml").write_text("lio: {}\n", encoding="utf-8")
    (runtime / "evaluation.yaml").write_text("result: {}\n", encoding="utf-8")
    (runtime / "node.log").write_text("runtime only\n", encoding="utf-8")
    (runtime / "ground_truth.tum").write_text("runtime only\n", encoding="utf-8")

    result = subprocess.run(
        [sys.executable, str(EXPORTER), "--runtime-dir", str(runtime),
         "--evidence-dir", str(evidence)],
        check=False, capture_output=True, text=True,
    )
    assert result.returncode == 0, result.stderr
    assert (evidence / "run_manifest.yaml").is_file()
    assert (evidence / "preflight.yaml").is_file()
    assert (evidence / "effective_rosparams.yaml").is_file()
    assert (evidence / "evaluation.yaml").is_file()
    assert not (evidence / "node.log").exists()
    assert not (evidence / "ground_truth.tum").exists()

    copied = yaml.safe_load((evidence / "run_manifest.yaml").read_text())
    assert copied["trajectory"]["sha256"] == sha256(trajectory)
    assert copied["rows"] == 1


def test_checker_accepts_runtime_heavy_files_but_rejects_them_in_evidence(tmp_path):
    runtime = tmp_path / "runtime" / "sbs_B0"
    evidence = tmp_path / "evidence" / "sbs_B0"
    runtime.mkdir(parents=True)
    evidence.mkdir(parents=True)
    trajectory = runtime / "trajectory.tum"
    trajectory.write_text("0.0 0 0 0 0 0 0 1\n", encoding="utf-8")
    manifest = {
        "completion": True,
        "trajectory": {"path": str(trajectory), "sha256": sha256(trajectory)},
        "rows": 1,
        "metric": {"value": 0.1, "unit": "m"},
    }
    (runtime / "run_manifest.yaml").write_text(
        yaml.safe_dump(manifest, sort_keys=False), encoding="utf-8"
    )
    (runtime / "node.log").write_text("allowed local runtime\n", encoding="utf-8")
    (evidence / "run_manifest.yaml").write_text(
        yaml.safe_dump(manifest, sort_keys=False), encoding="utf-8"
    )

    result = subprocess.run(
        [sys.executable, str(CHECKER), "--runtime-dir", str(runtime),
         "--evidence-dir", str(evidence), "--skip-git"],
        check=False, capture_output=True, text=True,
    )
    assert result.returncode == 0, result.stderr

    (evidence / "node.log").write_text("must not be tracked\n", encoding="utf-8")
    result = subprocess.run(
        [sys.executable, str(CHECKER), "--runtime-dir", str(runtime),
         "--evidence-dir", str(evidence), "--skip-git"],
        check=False, capture_output=True, text=True,
    )
    assert result.returncode != 0


if __name__ == "__main__":
    raise SystemExit(pytest.main([__file__]))
