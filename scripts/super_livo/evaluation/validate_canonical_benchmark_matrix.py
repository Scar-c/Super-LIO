#!/usr/bin/env python3
"""Fail-closed schema validation for the canonical benchmark ledger."""

import argparse
import pathlib
import sys

import yaml


REFERENCE_FIELDS = {
    "algorithm", "dataset", "sequence", "value", "metric", "alignment",
    "source_type", "source_owner", "source_title", "paper_or_repository",
    "revision_or_version", "location", "comparability",
}
REPRODUCTION_FIELDS = {
    "value", "metric", "alignment", "revision", "config_provenance",
    "snapshot", "evaluator_provenance", "validity",
}
PARENTS = ("super_lio", "fast_livo2")
VARIANTS = ("B0", "C0", "A0", "A1", "D", "Visual")


def validate(data):
    errors = []
    if data.get("schema") != "super_livo.canonical_benchmark_matrix.v2":
        errors.append("schema must be super_livo.canonical_benchmark_matrix.v2")
    references = data.get("reference_catalog", {})
    configs = data.get("config_catalog", {})
    reproductions = data.get("reproduction_catalog", {})
    historical = data.get("historical_result_catalog", {})
    for name, item in references.items():
        if not isinstance(item, dict):
            errors.append(f"reference_catalog.{name} must be a mapping")
            continue
        for field in sorted(REFERENCE_FIELDS - set(item)):
            errors.append(f"reference_catalog.{name}: missing {field}")
    for name, item in reproductions.items():
        if not isinstance(item, dict):
            errors.append(f"reproduction_catalog.{name} must be a mapping")
            continue
        for field in sorted(REPRODUCTION_FIELDS - set(item)):
            errors.append(f"reproduction_catalog.{name}: missing {field}")
        if item.get("validity") == "CANONICAL_VALID" and item.get("snapshot") in (None, "", "NOT_AVAILABLE"):
            errors.append(f"reproduction_catalog.{name}: canonical run missing snapshot")
    for name, item in historical.items():
        if not isinstance(item, dict):
            errors.append(f"historical_result_catalog.{name} must be a mapping")
            continue
        for field in sorted(REPRODUCTION_FIELDS - set(item)):
            errors.append(f"historical_result_catalog.{name}: missing {field}")
    rows = data.get("rows")
    if not isinstance(rows, list) or not rows:
        return errors + ["rows must be a non-empty list"]
    seen = set()
    for i, row in enumerate(rows):
        where = f"rows[{i}]"
        for field in ("dataset", "sequence", "local_bag", "gt_eval", "parents", "super_livo_results"):
            if field not in row:
                errors.append(f"{where}: missing {field}")
        key = (row.get("dataset"), row.get("sequence"))
        if key in seen:
            errors.append(f"{where}: duplicate dataset/sequence {key}")
        seen.add(key)
        parents = row.get("parents", {})
        for parent in PARENTS:
            p = parents.get(parent)
            if not isinstance(p, dict):
                errors.append(f"{where}.parents: missing {parent}")
                continue
            for group in ("method_author_references", "dataset_author_references", "config_sources", "reproductions"):
                values = p.get(group)
                if not isinstance(values, list):
                    errors.append(f"{where}.parents.{parent}: {group} must be a list")
                    continue
                required = REFERENCE_FIELDS if group.endswith("references") else REPRODUCTION_FIELDS if group == "reproductions" else set()
                for j, item in enumerate(values):
                    if isinstance(item, str):
                        catalog = references if group.endswith("references") else reproductions if group == "reproductions" else configs
                        if item not in catalog:
                            errors.append(f"{where}.parents.{parent}.{group}[{j}]: unknown catalog id {item}")
                        continue
                    if not isinstance(item, dict):
                        errors.append(f"{where}.parents.{parent}.{group}[{j}] must be a mapping")
                        continue
                    if required and isinstance(item.get("value"), (int, float)):
                        for field in sorted(required - set(item)):
                            errors.append(f"{where}.parents.{parent}.{group}[{j}]: missing {field}")
                    if group == "reproductions" and item.get("validity") == "CANONICAL_VALID":
                        if item.get("snapshot") in (None, "", "NOT_AVAILABLE"):
                            errors.append(f"{where}.parents.{parent}.{group}[{j}]: canonical run missing snapshot")
        variants = row.get("super_livo_results", {})
        for variant in VARIANTS:
            if variant not in variants:
                errors.append(f"{where}.super_livo_results: missing {variant}")
            elif isinstance(variants[variant], str) and variants[variant] in historical:
                pass
    return errors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ledger", type=pathlib.Path)
    args = parser.parse_args()
    try:
        data = yaml.safe_load(args.ledger.read_text())
    except (OSError, yaml.YAMLError) as exc:
        print(f"ledger unreadable: {exc}")
        return 2
    errors = validate(data or {})
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        print("LEDGER_SCHEMA_VALIDATION = FAIL")
        return 1
    print(f"rows={len(data['rows'])}")
    print("LEDGER_SCHEMA_VALIDATION = PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
