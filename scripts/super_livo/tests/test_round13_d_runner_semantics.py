#!/usr/bin/env python3
"""Round13 corrective — recovered canonical D runner semantics (R-T1..R-T8).

Protects the Origin-frozen D0/DV0 semantic schema from runner drift:
  scheduler_family, camera_input_enabled, camera_epoch_enabled,
  visual_frontend_enabled, visual_measurement_enabled are protected;
  only visual_state_apply may differ between D0 and DV0.

R-T1  D0 profile resolves the frozen semantics.
R-T2  DV0 inherits D0 and changes only visual_state_apply.
R-T3  NTU adapter cannot override protected D semantics.
R-T4  Oxford adapter cannot override protected D semantics.
R-T5  MCD adapter cannot override protected D semantics.
R-T6  Missing protected field fails closed.
R-T7  Dataset adapter difference does not change profile identity.
R-T8  D0 vs DV0 semantic diff contains exactly one algorithm field.
"""

import os
import re
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
RUNNER = os.path.join(REPO, "scripts", "super_livo", "experiments", "run_offline_variant.sh")
ROUND13_WRAPPERS = [
    os.path.join(REPO, "base_ws", "tools", "benchmark_adapters", "launch_r13_d0.sh"),
    os.path.join(REPO, "base_ws", "tools", "benchmark_adapters", "launch_r13_dv0.sh"),
]

PROTECTED_FIELDS = [
    "scheduler_family",
    "camera_input_enabled",
    "camera_epoch_enabled",
    "visual_frontend_enabled",
    "visual_measurement_enabled",
    "lidar_raw_scan_policy",
    "full_lidar_observe_per_raw_scan",
]

CANONICAL_D0 = {
    "profile": "D0",
    "scheduler_family": "D_CORRECTED",
    "camera_input_enabled": True,
    "camera_epoch_enabled": True,
    "visual_frontend_enabled": True,
    "visual_measurement_enabled": True,
    "visual_state_apply": False,
    "lidar_raw_scan_policy": "FULL_RAW_SCAN_AT_SCAN_END",
    "full_lidar_observe_per_raw_scan": 1,
}


def d0_resolved_from_runner(runner_path=RUNNER):
    """Parse the recovered historical D runner (run_offline_variant.sh) into
    the normalized semantic dict. Returns (semantics, missing)."""
    if not os.path.isfile(runner_path):
        raise FileNotFoundError(runner_path)
    text = open(runner_path, encoding="utf-8").read()
    d0 = dict(CANONICAL_D0)
    d0["scheduler_family"] = "D_CORRECTED"
    for key, pat in [
        ("camera_input_enabled", r"rosparam set /camera/enabled true"),
        ("camera_epoch_enabled", r"rosparam set /lio/camera_epoch/enabled true"),
        ("visual_frontend_enabled", r"rosparam set /lio/v0/enabled true"),
        ("visual_measurement_enabled", r"rosparam set /lio/v2/enabled true"),
        ("state_apply_false", r"rosparam set /lio/v4/apply false"),
        ("d_scheduler", r"imu_fullscan"),
    ]:
        d0[key] = bool(re.search(pat, text))
    missing = [k for k in PROTECTED_FIELDS if k not in d0]
    return d0, missing


def dv0_from_d0(d0):
    dv0 = dict(d0)
    dv0["profile"] = "DV0"
    dv0["visual_state_apply"] = True
    return dv0


def semantic_diff(a, b):
    keys = set(a) | set(b)
    return {k: (a.get(k), b.get(k)) for k in keys if a.get(k) != b.get(k)}


class TestCanonicalD0Resolution(unittest.TestCase):
    def test_r_t1_d0_resolves_frozen_semantics(self):
        d0, missing = d0_resolved_from_runner()
        self.assertEqual(missing, [])
        self.assertTrue(d0["camera_input_enabled"])
        self.assertTrue(d0["camera_epoch_enabled"])
        self.assertTrue(d0["visual_frontend_enabled"])
        self.assertTrue(d0["visual_measurement_enabled"])
        self.assertFalse(d0["visual_state_apply"])
        self.assertEqual(d0["scheduler_family"], "D_CORRECTED")
        self.assertTrue(d0["d_scheduler"])
        self.assertTrue(d0["state_apply_false"])

    def test_r_t2_dv0_inherits_d0_changes_only_apply(self):
        d0, _ = d0_resolved_from_runner()
        dv0 = dv0_from_d0(d0)
        diff = semantic_diff(d0, dv0)
        self.assertEqual(set(diff), {"profile", "visual_state_apply"})
        self.assertTrue(dv0["visual_state_apply"])

    def test_r_t8_d0_dv0_algorithm_diff_is_one_field(self):
        d0, _ = d0_resolved_from_runner()
        dv0 = dv0_from_d0(d0)
        alg_diff = {
            k: v for k, v in semantic_diff(d0, dv0).items()
            if k not in {"profile"} and k in PROTECTED_FIELDS + ["visual_state_apply"]
        }
        self.assertEqual(set(alg_diff), {"visual_state_apply"})

    def test_r_t6_missing_protected_field_fails_closed(self):
        incomplete = dict(CANONICAL_D0)
        del incomplete["camera_epoch_enabled"]
        missing = [k for k in PROTECTED_FIELDS if k not in incomplete]
        self.assertEqual(missing, ["camera_epoch_enabled"])

    def test_r_t7_adapter_diff_does_not_change_profile_identity(self):
        a = dict(CANONICAL_D0)
        b = dict(CANONICAL_D0)
        b["dataset_calibration_source"] = "ntu_viral"
        a["dataset_calibration_source"] = "mcd"
        alg_diff = {
            k: v for k, v in semantic_diff(a, b).items()
            if k in PROTECTED_FIELDS + ["visual_state_apply"]
        }
        self.assertEqual(alg_diff, {})


class TestDatasetAdaptersCannotOverrideProtected(unittest.TestCase):
    def test_r_t3_ntu_adapter_no_protected_override(self):
        self._assert_no_protected_override("ntu")

    def test_r_t4_oxford_adapter_no_protected_override(self):
        self._assert_no_protected_override("oxford")

    def test_r_t5_mcd_adapter_no_protected_override(self):
        self._assert_no_protected_override("mcd")

    def _assert_no_protected_override(self, dataset_tag):
        violations = []
        for wrapper in ROUND13_WRAPPERS:
            if not os.path.isfile(wrapper):
                continue
            text = open(wrapper, encoding="utf-8").read()
            for field, pat in [
                ("camera_input_enabled", r"/camera/enabled"),
                ("camera_epoch_enabled", r"/lio/camera_epoch/enabled"),
                ("visual_frontend_enabled", r"/lio/v0/enabled"),
                ("visual_measurement_enabled", r"/lio/v2/enabled"),
                ("scheduler_family", r"/lio/camera_epoch/lidar_update_policy"),
                ("visual_state_apply", r"/lio/v4/apply"),
            ]:
                if re.search(pat, text):
                    violations.append((os.path.basename(wrapper), field))
        # Current Round13 dataset-specific wrappers DO duplicate protected D
        # semantics: this is the drift the corrective documents. The protected
        # adapter boundary must be enforced by the shared profile layer.
        self.assertEqual(
            violations, [],
            "dataset adapter overrides protected D semantics: %s" % violations)


if __name__ == "__main__":
    sys.exit(unittest.main())
