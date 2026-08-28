#!/usr/bin/env python3
"""Public-CLI contract tests for the canonical benchmark ledger schema."""

import pathlib
import subprocess
import tempfile

import yaml


ROOT = pathlib.Path(__file__).resolve().parents[3]
VALIDATOR = ROOT / "scripts/super_livo/evaluation/validate_canonical_benchmark_matrix.py"
LEDGER = ROOT / ".scratch/super-livo-v1/reference/canonical_benchmark_matrix.yaml"


def run(data):
    with tempfile.NamedTemporaryFile("w", suffix=".yaml") as f:
        yaml.safe_dump(data, f)
        f.flush()
        return subprocess.run(
            ["python3", str(VALIDATOR), f.name],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )


base = {
    "schema": "super_livo.canonical_benchmark_matrix.v2",
    "rows": [
        {
            "dataset": "Fixture",
            "sequence": "seq",
            "local_bag": "AVAILABLE",
            "gt_eval": {"metric": "translation ATE RMSE", "evaluator_provenance": "DATASET_AUTHOR_BENCHMARK"},
            "parents": {
                "super_lio": {"method_author_references": [], "dataset_author_references": [], "config_sources": [], "reproductions": []},
                "fast_livo2": {"method_author_references": [], "dataset_author_references": [], "config_sources": [], "reproductions": []},
            },
            "super_livo_results": {"B0": "NOT_RUN", "C0": "NOT_RUN", "A0": "NOT_RUN", "A1": "NOT_RUN", "D": "NOT_RUN", "Visual": "NOT_RUN"},
        }
    ],
}

bad = yaml.safe_load(yaml.safe_dump(base))
bad["rows"][0]["parents"]["fast_livo2"]["method_author_references"] = [
    {"value": 0.04, "algorithm": "FAST-LIVO2", "dataset": "Fixture", "sequence": "seq"}
]
result = run(bad)
assert result.returncode != 0
assert "source_type" in result.stdout and "source_owner" in result.stdout and "metric" in result.stdout

bad_id = yaml.safe_load(yaml.safe_dump(base))
bad_id["rows"][0]["parents"]["super_lio"]["reproductions"] = ["missing_run"]
result = run(bad_id)
assert result.returncode != 0
assert "unknown catalog id missing_run" in result.stdout

result = subprocess.run(
    ["python3", str(VALIDATOR), str(LEDGER)],
    text=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    check=False,
)
assert result.returncode == 0, result.stdout
assert "LEDGER_SCHEMA_VALIDATION = PASS" in result.stdout

print("CANONICAL BENCHMARK MATRIX SCHEMA: ALL PASS")
