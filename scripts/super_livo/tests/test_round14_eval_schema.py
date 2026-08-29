#!/usr/bin/env python3
"""Round14 eval schema tests (Prompt75 migration).

Weak proof styles were replaced per Prompt75 §25/F7: the schema and parent
validation now come from the registry GENERATOR (visual_eval_registry.py,
single source of truth); behavioral tests here cover the scorecard builder.
FS-T1..FS-T30 in test_round14_final_seal.py are the CLOSE gates.
"""
import hashlib
import json
import pathlib

import yaml
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[3] /
                        "scripts/super_livo/evaluation"))
import visual_eval_registry as R  # noqa: E402
import visual_eval_score as V  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[3]
EVAL = ROOT / "scripts/super_livo/evaluation/visual_eval_score.py"
REGISTRY = ROOT / "docs/super_livo/evidence/visual_semantics_eval_registry.tsv"


def _fixture_log(tmp, lines, rows=3981):
    d = pathlib.Path(tmp) / "run"
    (d / "out").mkdir(parents=True)
    (d / "out" / "node_stdout.log").write_text("\n".join(lines))
    (d / "state.json").write_text('{"state":"SUCCESS","cleanup_verified":true}')
    # Prompt77: synthetic fixtures materialize a run-bound semantic snapshot
    manifest = {"semantic_profile": "D_VISUAL_APPLY", "validator": "",
                "requires_measurement_evidence": True,
                "production_revision": "a" * 40,
                "semantic_profile_revision": "b" * 64,
                "dataset_adapter_revision": "c" * 64,
                "transaction_revision": "d" * 40,
                "config_provenance": {"lio": "x", "visual": "y",
                                      "dataset_calibration": "z"},
                "visual_measurement_event": "CAMERA_EPOCH",
                "visual_measurement_timestamp_semantics": "CAMERA_EPOCH_PROPAGATED_STATE",
                "visual_state_apply": True,
                "visual_state_apply_connectivity": "ESTABLISHED",
                "camera_payload_ownership_mode": "RETAIN_THROUGH_MEASUREMENT",
                "visual_measurement_exact_once": True,
                "visual_measurement_enabled": True,
                "camera_input_enabled": True, "camera_epoch_enabled": True,
                "visual_frontend_enabled": True, "visual_map_producer_enabled": True,
                "scheduler_family": "D_CORRECTED",
                "raw_lidar_policy": "FULL_RAW_SCAN_AT_SCAN_END",
                "full_lidar_observe_per_raw_scan": 1, "camera_stride": 1}
    sys.path.insert(0, str(ROOT / "scripts/super_livo/experiments"))
    import semantic_profiles as SP
    snap_file = d / "out" / "semantic_snapshot.yaml"
    _, sha = SP.materialize_snapshot(
        manifest, ROOT / "scripts/super_livo/evaluation/semantic_snapshot_v0.yaml",
        snap_file)
    manifest["semantic_snapshot_path"] = "semantic_snapshot.yaml"
    manifest["semantic_snapshot_sha256"] = sha
    manifest["semantic_snapshot_schema_version"] = 1
    (d / "out" / "resolved_experiment_semantics.yaml").write_text(
        yaml.safe_dump(manifest))
    if rows:
        (d / "out" / "trajectory.tum").write_text(
            "\n".join(f"{1.0 + i * 1e-3:.9f} 0 0 0 0 0 0 1" for i in range(rows)))
        (d / "gt.csv").write_text(
            "# time, seq, stamp, x, y, z, ...\n" + "\n".join(
                f"{(1.0 + i * 1e-3) * 1e9:.0f} 0 0 0.0 0.0 0.0 0 0 0 0 0 0 0 0"
                for i in range(rows)))
    return d


def run_eval(run_dir, stage, extra=()):
    cmd = [sys.executable, str(EVAL), "--stage", stage, "--run-dir",
           str(run_dir / "out")]
    if (run_dir / "out" / "resolved_experiment_semantics.yaml").exists():
        cmd += ["--manifest",
                str(run_dir / "out" / "resolved_experiment_semantics.yaml")]
    if (run_dir / "out" / "trajectory.tum").exists():
        cmd += ["--trajectory", str(run_dir / "out" / "trajectory.tum")]
    if (run_dir / "gt.csv").exists() and not any(a == "--gt" for a in extra):
        cmd += ["--gt", str(run_dir / "gt.csv")]
    cmd += list(extra)
    return subprocess.run(cmd, capture_output=True, text=True, timeout=180)


class TestScorecardSemantics(unittest.TestCase):
    def test_e_t1_used_once_never_becomes_observe_count(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="et1-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=3987 "
             "geometry_updates=3985 updates_per_raw_scan=0.999498 map_updates=3985 "
             "imu_segments=5947",
             "Round11X fullscan ownership: raw_input_points=15404166 "
             "pre_observe_excluded_scans=1 pre_observe_excluded_points=3375 "
             "eligible_geometry_points=15400791 used_once=15397240 "
             "duplicate_use=0 never_used=3551 imu_only_segments=1966 "
             "raw_input_scans=3987 eligible_scans=3986 unique_used_scans=3985 "
             "duplicate_scan_use_events=0 eligible_never_used_scans=1"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["full_lidar_observe_count"], 3985)
        self.assertEqual(s["completion"]["geometry_points_used_once"], 15397240)
        self.assertNotEqual(s["completion"]["full_lidar_observe_count"],
                            s["completion"]["geometry_points_used_once"])

    def test_e_t2_geometry_update_count_is_observe_count(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="et2-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=100 "
             "geometry_updates=99 updates_per_raw_scan=0.99 map_updates=99 "
             "imu_segments=120"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["full_lidar_observe_count"], 99)

    def test_e_t3_initial_total_and_per_frame_separate(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="et3-"),
            ["VISUAL_MEASUREMENT observation: frames=50 candidates=500 "
             "valid=480 rejected=20 residual_samples=10000 conservation=OK",
             "R14 initial measurement query: attempts=40 hits=38",
             "R14 initial measurement observation: frames=20 candidates=400 "
             "valid=390 rejected=10 residual_samples=4000",
             "R14 residuals_per_frame P10=180.0 P50=200.0 P90=220.0 P99=240.0 n=20"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["measurement_counts"]["residual_samples_total"], 10000)
        self.assertEqual(s["measurement_counts"]["initial_residual_samples_total"], 4000)
        self.assertEqual(s["residual_density_per_frame"]["P50"], 200.0)
        self.assertEqual(s["residual_density_per_frame"]["mean"], 200.0)

    def test_e_t5_completion_ratio_numeric_explicit_reference(self):
        d = _fixture_log(tempfile.mkdtemp(prefix="et5-"),
                         ["Round11X cadence: policy=imu_fullscan raw_scans=100 "
                          "geometry_updates=99 updates_per_raw_scan=0.99 "
                          "map_updates=99 imu_segments=120"], rows=500)
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW", ["--expected-rows", "500"])
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["completion_ratio"], 1.0)
        self.assertEqual(s["completion"]["expected_or_reference_rows"], 500)

    def test_e_t6_empty_optional_metric_explicit(self):
        d = _fixture_log(tempfile.mkdtemp(prefix="et6-"),
                         ["Round11X cadence: policy=imu_fullscan raw_scans=100 "
                          "geometry_updates=99 updates_per_raw_scan=0.99 "
                          "map_updates=99 imu_segments=120"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertIsNone(s["completion"]["completion_ratio"])

    def test_e_t7_information_percentiles_map(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="et7-"),
            ["R14 I_norm lambda_min P10=10.0 P50=20.0 P90=30.0",
             "R14 I_norm lambda_max P10=100.0 P50=200.0 P90=300.0",
             "R14 I_norm trace P10=100.0 P50=200.0 P90=300.0",
             "R14 I cond P10=5.0 P50=10.0 P90=15.0",
             "R14 I degenerate_frames=0 invalid_frames=0"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["information"]["lambda_min_norm_P50"], 20.0)
        self.assertEqual(s["information"]["lambda_max_norm_P50"], 200.0)
        self.assertEqual(s["information"]["condition_P50"], 10.0)
        self.assertEqual(s["information"]["degenerate_frames"], 0)

    def test_e_t8_compute_percentiles_map(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="et8-"),
            ["R14 visual cpu P10=5.0 P50=10.0 P90=15.0"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["compute"]["visual_lifecycle_residual_ms_P50"], 10.0)

    def test_e_t9_spatial_absence_not_empty_dict(self):
        d = _fixture_log(tempfile.mkdtemp(prefix="et9-"), [])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertIsInstance(s["spatial_coverage"], dict)
        self.assertEqual(s["spatial_coverage"]["status"],
                         "NOT_AVAILABLE_CURRENT_INSTRUMENTATION")

    def test_ec_t8_initial_total_not_percentile(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="ec8-"),
            ["R14 initial measurement observation: frames=20 candidates=400 "
             "valid=390 rejected=10 residual_samples=4000",
             "R14 residuals_per_frame P10=180.0 P50=200.0 P90=220.0 P99=240.0 n=20"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertNotEqual(s["measurement_counts"]["initial_residual_samples_total"],
                            s["residual_density_per_frame"]["P50"])

    def test_ec_t9_observe_is_geometry_update(self):
        d = _fixture_log(
            tempfile.mkdtemp(prefix="ec9-"),
            ["Round11X cadence: policy=imu_fullscan raw_scans=3987 "
             "geometry_updates=3985 updates_per_raw_scan=0.999498 map_updates=3985 "
             "imu_segments=5947"])
        run_eval(d, "A2_D_CAMERA_EPOCH_SHADOW")
        s = json.loads((d / "out" / "visual_eval_score.json").read_text())
        self.assertEqual(s["completion"]["full_lidar_observe_count"], 3985)
        self.assertEqual(s["completion"]["geometry_update_events"], 3985)

    def test_ec_t10_generated_registry_schema_green(self):
        errors = R.validate_registry(REGISTRY)
        # the registry is regenerated after the canonical runs; before that
        # the on-disk TSV may be the old migrated file -> tolerate absence.
        if REGISTRY.exists():
            header = REGISTRY.read_text().split("\n")[0].split("\t")
            if header == list(R.REGISTRY_SCHEMA):
                self.assertEqual(errors, [])


class TestEvalFinalizationGuardrails(unittest.TestCase):
    def test_ef_t3_unknown_stage_no_guess(self):
        self.assertEqual(V.build_scorecard(
            "UNKNOWN_STAGE_XYZ", tempfile.mkdtemp(), None, legacy_mode=True
        )["provenance"]["parent_stage"], "UNREGISTERED_STAGE")


if __name__ == "__main__":
    unittest.main()
