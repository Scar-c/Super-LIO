#!/usr/bin/env python3
"""M-T9/M-T10: Shadow evidence is observational and validator is fail-closed."""
import pathlib
import subprocess
import tempfile
import unittest
import yaml

ROOT = pathlib.Path(__file__).resolve().parents[3]
VALIDATOR = ROOT / "scripts/super_livo/experiments/validate_d_visual_shadow_result.py"


class MeasurementEvidenceIntegrationTest(unittest.TestCase):
    def test_m_t9_shadow_path_cannot_apply_state(self):
        source = (ROOT / "src/super_lio/src/lio/super_lio.cpp").read_text()
        # Phase-B corrective (b0af1c1): the legacy LiDAR-callback Visual
        # measurement path is additionally gated off under camera-epoch D
        # (d_camera_epoch_visual); the shadow/no-apply semantics are unchanged.
        start = source.index("if(g_lio_v2_enabled && !g_lio_v4_apply")
        call = source[start:]
        call = call[:call.index("return;")]
        residual = source[source.index("int SuperLIO::runVisualResidual"):]
        residual = residual[:residual.index("void SuperLIO::runVisualLifecycle")]
        self.assertIn("runVisualResidual", call)
        self.assertIn("recordNormalEquations", residual)
        self.assertNotIn("UpdateObserve", call + residual)

    def test_m_t10_instrumentation_has_no_ros_or_filter_dependency(self):
        header = (ROOT / "src/super_lio/include/instrumentation/VisualMeasurementEvidence.h").read_text()
        self.assertNotIn("ROSWrapper", header)
        self.assertNotIn("ESKF", header)
        self.assertNotIn("ros/", header)

    def test_validator_accepts_complete_finite_shadow_evidence(self):
        log = """V-0 VisualMap: parents=2 landmarks=2 slots_used=2 created=2 frames=1 attempts=2
VISUAL_MEASUREMENT query: attempts=4 hits=3 misses=1 rejected_explicit=0 conservation=OK
VISUAL_MEASUREMENT observation: frames=1 candidates=3 valid=2 rejected=1 residual_samples=32 conservation=OK
VISUAL_MEASUREMENT H: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1 P95=1 P99=1 max=1
VISUAL_MEASUREMENT b: accumulations=1 nonzero=1 zero=0 nonfinite=0 norm_count=1 P50=1 P95=1 P99=1 max=1
VISUAL_MEASUREMENT proposed_correction=NOT_COMPUTED_BY_SHADOW_PROFILE state_apply_count=0
fullscan ownership: raw_scans=1 geometry_used=1 excluded=0 never_used=0 duplicate_use=0
"""
        self.assertEqual(self.run_validator(log)[0], 0)

    def test_validator_rejects_nonfinite_or_missing_evidence(self):
        base = """V-0 VisualMap: parents=2 landmarks=2 slots_used=2 created=2 frames=1 attempts=2
VISUAL_MEASUREMENT query: attempts=1 hits=1 misses=0 rejected_explicit=0 conservation=OK
VISUAL_MEASUREMENT observation: frames=1 candidates=1 valid=1 rejected=0 residual_samples=16 conservation=OK
VISUAL_MEASUREMENT H: accumulations=1 nonzero=1 zero=0 nonfinite=1 norm_count=0 P50=0 P95=0 P99=0 max=0
VISUAL_MEASUREMENT proposed_correction=NOT_COMPUTED_BY_SHADOW_PROFILE state_apply_count=0
fullscan ownership: raw_scans=1 geometry_used=1 excluded=0 never_used=0 duplicate_use=0
"""
        self.assertNotEqual(self.run_validator(base)[0], 0)

    def run_validator(self, log):
        with tempfile.TemporaryDirectory() as td:
            directory = pathlib.Path(td)
            (directory / "node.log").write_text(log)
            manifest = {"semantic_profile": "D_VISUAL_SHADOW", "visual_state_apply": False}
            (directory / "manifest.yaml").write_text(yaml.safe_dump(manifest))
            result = subprocess.run(["python3", str(VALIDATOR), "--log", str(directory / "node.log"),
                                     "--manifest", str(directory / "manifest.yaml"),
                                     "--out", str(directory / "gate.yaml")], capture_output=True, text=True)
            return result.returncode, result.stdout


if __name__ == "__main__":
    unittest.main()
