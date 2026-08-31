#!/usr/bin/env python3
"""Tests for the source/config identity seam used by run manifests."""

import importlib.util
import pathlib


MODULE_PATH = pathlib.Path(__file__).resolve().parents[2] / "tools/prob_lio/production_identity.py"
SPEC = importlib.util.spec_from_file_location("production_identity", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def make_checkout(root, config_text):
    (root / "src/super_lio/include/lio").mkdir(parents=True)
    (root / "src/super_lio/src/lio").mkdir(parents=True)
    (root / "src/super_lio/include/lio/estimator.h").write_text(
        "#pragma once\n", encoding="utf-8"
    )
    (root / "src/super_lio/src/lio/estimator.cpp").write_text(
        "int estimate() { return 0; }\n", encoding="utf-8"
    )
    (root / "src/super_lio/CMakeLists.txt").write_text(
        "project(super_lio)\n", encoding="utf-8"
    )
    config = root / "src/super_lio/config/NTU.yaml"
    config.parent.mkdir(parents=True)
    config.write_text(config_text, encoding="utf-8")
    return config


def test_code_identity_excludes_dataset_config_and_changes_for_source(tmp_path):
    first = tmp_path / "first"
    second = tmp_path / "second"
    config_a = make_checkout(first, "sequence: eee_01\n")
    config_b = make_checkout(second, "sequence: nya_01\n")

    assert MODULE.production_code_oid(first) == MODULE.production_code_oid(second)
    assert MODULE.dataset_config_sha256(config_a) != MODULE.dataset_config_sha256(config_b)

    (second / "src/super_lio/src/lio/estimator.cpp").write_text(
        "int estimate() { return 1; }\n", encoding="utf-8"
    )
    assert MODULE.production_code_oid(first) != MODULE.production_code_oid(second)
