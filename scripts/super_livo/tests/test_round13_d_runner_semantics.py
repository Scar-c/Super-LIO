#!/usr/bin/env python3
"""Round13 Prompt59 normalized semantic-profile contract (N-T1..N-T8)."""
import importlib.util
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
MODULE = ROOT / "scripts/super_livo/experiments/semantic_profiles.py"

def load_module():
    spec = importlib.util.spec_from_file_location("semantic_profiles", MODULE)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

class TestNormalizedSemanticProfiles(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sp = load_module()

    def resolve(self, profile="D_VISUAL_SHADOW", dataset="NTU", alias="DV0"):
        return self.sp.resolve_profile(
            profile, legacy_alias=alias, dataset=dataset, sequence="eee_01",
            camera_stride=1,
            revisions={"production_revision": "prod", "semantic_profile_revision": "profile",
                       "dataset_adapter_revision": "adapter", "transaction_revision": "tx"},
            provenance={"lio": "Super-LIO", "visual": "FAST-LIVO2",
                        "dataset_calibration": "NTU VIRAL"})

    def test_n_t1_alias_does_not_determine_protected_semantics(self):
        self.assertEqual(self.sp.protected_projection(self.resolve(alias="DV0")),
                         self.sp.protected_projection(self.resolve(alias="D0")))

    def test_n_t2_shadow_resolves_all_protected_fields(self):
        m = self.resolve(); self.sp.validate_manifest(m)
        self.assertEqual(m["scheduler_family"], "D_CORRECTED")
        for key in ("camera_input_enabled", "camera_epoch_enabled", "visual_frontend_enabled",
                    "visual_map_producer_enabled", "visual_measurement_enabled"):
            self.assertIs(m[key], True)
        self.assertIs(m["visual_state_apply"], False)
        self.assertEqual(m["raw_lidar_policy"], "FULL_RAW_SCAN_AT_SCAN_END")
        self.assertEqual(m["full_lidar_observe_per_raw_scan"], 1)

    def test_n_t3_apply_inherits_shadow_and_changes_only_apply(self):
        shadow, apply = self.resolve(), self.resolve("D_VISUAL_APPLY")
        # Prompt64: the APPLY profile additionally declares its REQUESTED
        # camera-epoch event placement (differs from the legacy Shadow
        # placement); state apply is the only enabled/flag change.
        self.assertEqual(self.sp.semantic_diff(shadow, apply),
                         {"semantic_profile", "visual_state_apply",
                          "visual_measurement_event",
                          "visual_measurement_timestamp_semantics",
                          "camera_payload_ownership_mode", "validator"})
        self.assertIs(apply["visual_state_apply"], True)
        self.assertEqual(apply["visual_measurement_event"], "CAMERA_EPOCH")

    def test_n_t4_dataset_adapter_cannot_override_protected_fields(self):
        with self.assertRaises(self.sp.SemanticProfileError):
            self.sp.resolve_profile("D_VISUAL_SHADOW", legacy_alias="DV0", dataset="NTU",
                                    sequence="eee_01", camera_stride=1,
                                    adapter_overrides={"visual_state_apply": True})

    def test_n_t5_missing_protected_field_fails_closed(self):
        m = self.resolve(); del m["visual_map_producer_enabled"]
        with self.assertRaises(self.sp.SemanticProfileError): self.sp.validate_manifest(m)

    def test_n_t6_dv0_is_metadata_only(self):
        m = self.resolve(alias="DV0")
        self.assertEqual(m["legacy_alias"], "DV0")
        self.assertNotIn("DV0", str(self.sp.protected_projection(m)))

    def test_n_t7_profile_identity_across_datasets(self):
        identities = {self.resolve(dataset=d)["semantic_profile"] for d in ("NTU", "Oxford", "MCD")}
        self.assertEqual(identities, {"D_VISUAL_SHADOW"})

    def test_n_t8_manifest_written_and_validated_before_playback(self):
        with tempfile.TemporaryDirectory() as td:
            path = pathlib.Path(td) / "resolved_experiment_semantics.yaml"
            self.sp.write_manifest(self.resolve(), path)
            self.assertTrue(path.is_file())
            loaded = self.sp.load_manifest(path); self.sp.validate_manifest(loaded)
            self.assertEqual(loaded["semantic_profile"], "D_VISUAL_SHADOW")

if __name__ == "__main__": unittest.main(verbosity=2)
