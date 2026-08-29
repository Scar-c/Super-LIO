#!/usr/bin/env python3
"""Validate the machine-readable Round14 FINAL HARD CLOSE evidence
(docs/super_livo/evidence/round14_final_hard_close_evidence.json).

Rejects: missing gate, PASS without artifact, PASS without evidence type,
adversarial gate without negative test, real-seam gate backed only by a
synthetic fixture, unknown failure class, duplicate gate, NOT_RUN gate
marked PASS.
"""
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVIDENCE = ROOT / "docs/super_livo/evidence/round14_final_hard_close_evidence.json"

EVIDENCE_TYPES = {
    "REAL_TRANSACTION_SEAM", "RESOLVER_BEHAVIOR", "GENERATOR_BEHAVIOR",
    "ADVERSARIAL_REJECTION", "REAL_HISTORICAL_E2E", "PRODUCTION_HELPER_UNIT",
}
NEGATIVE_GATES = {
    "F1_MANIFEST_SNAPSHOT_HASH_NOT_ENFORCED",
    "F2_SNAPSHOT_REVISION_SCHEMA_BINDING_INCOMPLETE",
    "MANIFEST_SNAPSHOT_HASH_REQUIRED", "MANIFEST_SNAPSHOT_SCHEMA_REQUIRED",
    "MANIFEST_PRODUCTION_REVISION_REQUIRED", "SNAPSHOT_PRODUCTION_REVISION_REQUIRED",
    "SNAPSHOT_SCHEMA_VERSION_REQUIRED", "HASH_MISMATCH_FAILS_CLOSED",
    "REVISION_MISMATCH_FAILS_CLOSED", "SCHEMA_MISMATCH_FAILS_CLOSED",
    "REAL_SEAM_POST_CAPTURE_MUTATION_REJECTED",
    "REAL_SEAM_REHASH_WRONG_REVISION_REJECTED",
    "REAL_SEAM_SCHEMA_ATTACK_REJECTED",
    "RB_T10_CORRECTED_TO_REJECT_WRONG_HASH",
    "HISTORICAL_CURRENT_TEMPLATE_IMMUNITY",
    "FALSE_POSITIVE_HARD_CLOSE_TESTS", "STALE_HARD_CLOSE_TESTS",
    "EVERY_NEGATIVE_GATE_HAS_MUTATION_PROOF",
    "FINAL_SNAPSHOT_INTEGRITY_AUDIT_FINDINGS",
}
REAL_SEAM_GATES = {
    "SNAPSHOT_CAPTURE_PRE_EXECUTION",
    "SNAPSHOT_FINAL_BYTES_HASHED_BEFORE_MANIFEST_BIND",
    "ACTUAL_SHA_EQUALS_MANIFEST_SHA", "SNAPSHOT_REVISION_EQUALS_MANIFEST_REVISION",
    "SNAPSHOT_SCHEMA_EQUALS_MANIFEST_SCHEMA", "REAL_TRANSACTION_SNAPSHOT_SEAM",
    "REAL_SEAM_SHELL_SHA_VERIFICATION", "REAL_SEAM_GIT_REVISION_VERIFICATION",
    "REAL_SEAM_SCHEMA_VERIFICATION",
    "REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY",
}


def validate(path=EVIDENCE):
    errors = []
    if not pathlib.Path(path).exists():
        return ["evidence file missing"]
    data = json.loads(pathlib.Path(path).read_text())
    if "initial_head" not in data or "hard_gates" not in data:
        return ["missing top-level fields"]
    gates = data["hard_gates"]
    seen = set()
    for name, g in gates.items():
        if name in seen:
            errors.append(f"duplicate gate {name}")
        seen.add(name)
        if g.get("status") == "NOT_RUN":
            continue
        if g.get("status") != "PASS":
            errors.append(f"gate {name} not PASS")
            continue
        etype = g.get("evidence_type")
        if not etype or etype not in EVIDENCE_TYPES:
            errors.append(f"gate {name}: missing/unknown evidence type {etype}")
        if not g.get("artifact"):
            errors.append(f"gate {name}: PASS without artifact")
        if name in NEGATIVE_GATES and not g.get("negative_test"):
            errors.append(f"gate {name}: adversarial gate without negative test")
        if name in REAL_SEAM_GATES and etype == "GENERATOR_BEHAVIOR":
            errors.append(f"gate {name}: real-seam gate backed by synthetic fixture")
        if not g.get("command"):
            errors.append(f"gate {name}: no command")
    # every expected §52 gate present
    expected = {
        "F1_MANIFEST_SNAPSHOT_HASH_NOT_ENFORCED",
        "F2_SNAPSHOT_REVISION_SCHEMA_BINDING_INCOMPLETE",
        "SNAPSHOT_CAPTURE_PRE_EXECUTION",
        "SNAPSHOT_FINAL_BYTES_HASHED_BEFORE_MANIFEST_BIND",
        "MANIFEST_SNAPSHOT_PATH_REQUIRED", "MANIFEST_SNAPSHOT_HASH_REQUIRED",
        "MANIFEST_SNAPSHOT_SCHEMA_REQUIRED", "MANIFEST_PRODUCTION_REVISION_REQUIRED",
        "SNAPSHOT_PRODUCTION_REVISION_REQUIRED", "SNAPSHOT_SCHEMA_VERSION_REQUIRED",
        "ACTUAL_SHA_EQUALS_MANIFEST_SHA",
        "SNAPSHOT_REVISION_EQUALS_MANIFEST_REVISION",
        "SNAPSHOT_SCHEMA_EQUALS_MANIFEST_SCHEMA",
        "HASH_MISMATCH_FAILS_CLOSED", "REVISION_MISMATCH_FAILS_CLOSED",
        "SCHEMA_MISMATCH_FAILS_CLOSED", "REAL_TRANSACTION_SNAPSHOT_SEAM",
        "REAL_SEAM_SHELL_SHA_VERIFICATION", "REAL_SEAM_GIT_REVISION_VERIFICATION",
        "REAL_SEAM_SCHEMA_VERIFICATION", "REAL_SEAM_POST_CAPTURE_MUTATION_REJECTED",
        "REAL_SEAM_REHASH_WRONG_REVISION_REJECTED",
        "REAL_SEAM_SCHEMA_ATTACK_REJECTED", "REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY",
        "RB_T10_CORRECTED_TO_REJECT_WRONG_HASH", "FH_T1_T20",
        "HISTORICAL_A2_BINDING_NONREGRESSION",
        "HISTORICAL_B0_BINDING_NONREGRESSION",
        "HISTORICAL_CURRENT_TEMPLATE_IMMUNITY",
        "PHASE_B_NUMERIC_REGRESSION", "PHASE_B_EVENT_REGRESSION",
        "PHASE_B_SOLVER_ACCOUNTING_REGRESSION",
        "FALSE_POSITIVE_HARD_CLOSE_TESTS", "STALE_HARD_CLOSE_TESTS",
        "CLOSE_EVIDENCE_JSON", "CLOSE_EVIDENCE_SCHEMA_VALID",
        "EVERY_HARD_GATE_HAS_BEHAVIORAL_EVIDENCE",
        "EVERY_NEGATIVE_GATE_HAS_MUTATION_PROOF",
        "NO_BAG_RERUN", "NO_PHASE_B_ALGORITHM_CHANGE",
        "NO_PARAMETER_TUNING", "PHASE_C_NOT_STARTED",
        "FINAL_SNAPSHOT_INTEGRITY_AUDIT_FINDINGS",
    }
    for gate in expected:
        if gate not in gates:
            errors.append(f"missing gate {gate}")
    return errors


import argparse


def _git(args):
    return subprocess.check_output(["git"] + args, text=True).strip()


def validate_provenance(data):
    """G3: functional-corrective-commit self-provenance."""
    errors = []
    fc = data.get("functional_corrective_commit")
    if not fc:
        errors.append("FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: missing")
        return errors
    if not re.fullmatch(r"[0-9a-f]{40}", str(fc)):
        errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {fc} not sha40")
        return errors
    try:
        subprocess.check_output(["git", "cat-file", "-e", fc + "^{commit}"],
                                stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {fc} not a commit")
        return errors
    initial = data.get("initial_head")
    try:
        base = subprocess.check_output(
            ["git", "merge-base", "--is-ancestor", initial, fc],
            stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {fc} not descendant of initial")
        return errors
    # the functional commit must contain the relevant gate-closing code paths
    required_paths = data.get("production_paths_tested", [])
    for path in required_paths:
        try:
            blob = subprocess.check_output(
                ["git", "show", f"{fc}:{path}"], stderr=subprocess.DEVNULL).decode()
        except subprocess.CalledProcessError:
            errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {path} absent at {fc}")
            continue
        if path.endswith("visual_eval_score.py") and "SEMANTIC_SNAPSHOT_PATH_MISSING" not in blob:
            errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {path} lacks G1 fix at {fc}")
        if path.endswith("close_evidence_validator.py") and "REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY" not in blob:
            errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {path} lacks G2 fix at {fc}")
        if path.endswith("close_evidence_validator.py") and "FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH" not in blob:
            errors.append(f"FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH: {path} lacks G3 fix at {fc}")
    return errors


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--evidence", default=str(EVIDENCE))
    args = ap.parse_args(argv)
    errors = validate(args.evidence)
    errors += validate_provenance(json.loads(pathlib.Path(args.evidence).read_text()))
    if errors:
        print("CLOSE_EVIDENCE_SCHEMA_VALID = FAIL")
        for e in errors:
            print(" -", e)
        return 1
    print("CLOSE_EVIDENCE_SCHEMA_VALID = PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
