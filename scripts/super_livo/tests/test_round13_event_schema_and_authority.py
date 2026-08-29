#!/usr/bin/env python3
"""Prompt64 TDD — runner/profile semantic authority + event schema + supervisor.

RP-T1..RP-T10  runner/profile semantic authority and event-placement schema
TX-T1..TX-T8    generic transaction supervisor boundary
"""

import os
import re
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
EXP = os.path.join(REPO, "scripts", "super_livo", "experiments")
sys.path.insert(0, EXP)

import semantic_profiles as sp

RUNNER = os.path.join(EXP, "run_offline_variant.sh")
SUPERVISOR = os.path.join(EXP, "run_superlivo_transaction.sh")

REVISIONS = {k: "r" for k in sp.REVISION_FIELDS}
PROVENANCE = {"lio": "a", "visual": "b", "dataset_calibration": "c"}


def resolve(profile, alias="d0", stride=1):
    return sp.resolve_profile(profile, legacy_alias=alias, dataset="ntu",
                              sequence="eee_01", camera_stride=stride,
                              revisions=REVISIONS, provenance=PROVENANCE)


class TestEventSchema(unittest.TestCase):
    def test_rp_t6_event_field_mandatory(self):
        with self.assertRaises(sp.SemanticProfileError):
            m = resolve("D_VISUAL_SHADOW")
            del m["visual_measurement_event"]
            sp.validate_manifest(m)

    def test_rp_t9_exact_once_field_required(self):
        with self.assertRaises(sp.SemanticProfileError):
            m = resolve("D_VISUAL_SHADOW")
            del m["visual_measurement_exact_once"]
            sp.validate_manifest(m)

    def test_rp_t10_payload_ownership_required(self):
        with self.assertRaises(sp.SemanticProfileError):
            m = resolve("D_VISUAL_SHADOW")
            del m["camera_payload_ownership_mode"]
            sp.validate_manifest(m)

    def test_rp_t7_apply_executable_after_phase_b(self):
        # Phase B: camera-event Apply connectivity established; the profile
        # resolves and passes executability.
        m = resolve("D_VISUAL_APPLY")
        self.assertEqual(m["visual_measurement_event"], "CAMERA_EPOCH")
        sp.validate_executability(m)

    def test_rp_t8_current_shadow_truth(self):
        m = resolve("D_VISUAL_SHADOW")
        self.assertTrue(m["visual_measurement_enabled"])
        self.assertEqual(m["visual_measurement_event"], "CAMERA_EPOCH")  # Phase A
        self.assertFalse(m["visual_state_apply"])
        sp.validate_executability(m)  # matches effective capability

    def test_rp_t3_alias_invariance(self):
        a = resolve("D_VISUAL_SHADOW", alias="d0")
        b = resolve("D_VISUAL_SHADOW", alias="arbitrary-meta-alias")
        diff = sp.semantic_diff(a, b)
        self.assertNotIn("visual_state_apply", diff)
        self.assertNotIn("visual_measurement_event", diff)
        self.assertEqual(a["camera_input_enabled"], b["camera_input_enabled"])


class TestAuthorityModel(unittest.TestCase):
    def test_rp_t2_profile_sole_authority_in_normalized_mode(self):
        text = open(RUNNER, encoding="utf-8").read()
        self.assertIn("SEMANTIC_AUTHORITY_CONFLICT", text)
        self.assertRegex(text, r"check-executable --manifest")
        # legacy variant expectations must not run in normalized mode
        norm_start = text.index("Prompt64: normalized mode")
        norm_end = text.index("else\n", norm_start)
        normalized_block = text[norm_start:norm_end]
        self.assertNotIn("b0|c0|d0", normalized_block)
        self.assertIn("producer-expected --manifest", normalized_block)

    def test_rp_t4_legacy_only_compatibility(self):
        text = open(RUNNER, encoding="utf-8").read()
        # legacy-only mode keeps the variant matrix + variant readback
        self.assertIn("case \"$VARIANT\" in", text)
        self.assertIn("b0|c0|d0) is_false \"$app\"", text)

    def test_rp_t1_legacy_leak_reproducible(self):
        # The leak the corrective removes: normalized apply=true rejected by
        # legacy d0 readback. Before the corrective the readback checked
        # "b0|c0|d0 -> apply false" unconditionally. After the corrective the
        # normalized-mode readback derives expectations from the manifest only.
        text = open(RUNNER, encoding="utf-8").read()
        normalized = text[text.index("Prompt64: normalized mode"):]
        normalized = normalized[:normalized.index("else\n")]
        self.assertIn("producer-expected --manifest", normalized)
        self.assertNotIn("b0|c0|d0", normalized)

    def test_rp_t5_authority_conflict_fail_closed(self):
        text = open(RUNNER, encoding="utf-8").read()
        self.assertIn("SEMANTIC_AUTHORITY_CONFLICT: legacy CLI lidar_update_policy",
                      text)


class TestGenericSupervisor(unittest.TestCase):
    def test_tx_t7_no_protected_field_hardcoded(self):
        text = open(SUPERVISOR, encoding="utf-8").read()
        for forbidden in ("D_VISUAL_SHADOW ]", "validate_d_visual_shadow_result",
                          "/lio/v4/apply", "imu_fullscan", "camera_stride"):
            self.assertNotIn(forbidden, text)

    def test_tx_t3_validator_selection_outside_process_logic(self):
        text = open(SUPERVISOR, encoding="utf-8").read()
        self.assertIn("validator --manifest \"$MANIFEST\"", text)
        self.assertIn('"$SEMANTIC_TOOL" validator --manifest "$MANIFEST"', text)
        self.assertIn("SLV_TEST_VALIDATOR", text)

    def test_tx_t1_t2_profile_identity_generic(self):
        text = open(SUPERVISOR, encoding="utf-8").read()
        self.assertNotIn("expected D_VISUAL_SHADOW profile", text)
        self.assertIn("post-run validator missing", text)

    def test_tx_t4_unknown_validator_fails_explicitly(self):
        # validator dispatch fails closed on a missing contract
        self.assertIn('raise SemanticProfileError("no validator contract for profile "',
                      open(os.path.join(EXP, "semantic_profiles.py")).read())
        self.assertIn("post-run validator missing", open(SUPERVISOR).read())


class TestDatasetAdapterBoundary(unittest.TestCase):
    def test_ntu_adapter_no_protected_override(self):
        self._adapter_clean("run_ntu_super_lio.sh")

    def test_ntu_shadow_adapter_no_protected_override(self):
        self._adapter_clean("run_ntu_d_visual_shadow.sh")

    def _adapter_clean(self, name):
        path = os.path.join(EXP, "adapters", name)
        if not os.path.isfile(path):
            self.skipTest(name)
        text = open(path, encoding="utf-8").read()
        for leaked in ("/lio/v4/apply", "/camera/enabled",
                       "/lio/camera_epoch/enabled", "/lio/v0/enabled",
                       "/lio/v2/enabled", "lidar_update_policy",
                       "g0/shadow", "g1/enabled"):
            self.assertNotIn(leaked, text)


if __name__ == "__main__":
    sys.exit(unittest.main())
