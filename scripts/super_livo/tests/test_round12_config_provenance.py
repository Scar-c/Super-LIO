#!/usr/bin/env python3
"""Round12 durable dual-parent configuration contract."""

from pathlib import Path
import re
import yaml


ROOT = Path(__file__).resolve().parents[3]
REF = ROOT / ".scratch/super-livo-v1/reference"
PARENTS = ("super_lio", "fast_livo2")
DATASETS = ("ntu", "mcd", "oxford", "m3dgr")
CRITICAL = {
    "blind", "max_range", "point_filter", "scan_voxel", "map_voxel",
    "ieskf_iterations", "ieskf_convergence", "search_neighbor_count",
    "search_extent", "max_correspondence_distance", "plane_point_count",
    "plane_fit_threshold", "planarity_threshold", "depth_beam_error",
    "imu_noise", "extrinsic", "time_offsets", "visual",
}
ALLOWED = {
    "EXACT_EQUIVALENT", "EQUIVALENT_AFTER_UNIT_CONVERSION",
    "SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION", "RELATED_NOT_EQUIVALENT",
    "SUPER_LIO_ONLY", "FAST_LIVO2_ONLY", "HARD_CODED_ARCHITECTURE", "UNKNOWN",
}


def load_yaml(path: Path):
    with path.open(encoding="utf-8") as stream:
        return yaml.safe_load(stream)


def test_parent_bundles_have_machine_provenance():
    for parent in PARENTS:
        for dataset in DATASETS:
            bundle = REF / "upstream_configs" / parent / dataset
            assert (bundle / "SOURCE.md").is_file(), bundle
            manifest = load_yaml(bundle / "manifest.yaml")
            assert manifest["repository"]
            assert manifest["revision"]
            assert manifest["classification"] in {
                "COMPLETE_REPRODUCIBLE", "PARTIAL_CONFIG",
                "PARTIAL_METADATA_ONLY", "MISSING", "NOT_PUBLISHED",
            }
            if manifest["classification"] in {"MISSING", "NOT_PUBLISHED"}:
                assert manifest["status"] in {"NOT_FOUND", "NOT_PUBLISHED"}
            for item in manifest.get("files", []):
                assert re.fullmatch(r"[0-9a-f]{64}", item["sha256"])
                assert (bundle / item["path"]).is_file()


def test_hardcoded_inventory_has_sources_and_roles():
    data = load_yaml(REF / "hardcoded_accuracy_parameters.yaml")
    assert {row["algorithm"] for row in data["parameters"]} == {
        "Super-LIO", "FAST-LIVO2"
    }
    for row in data["parameters"]:
        assert row["revision"] and row["source_path"] and row["physical_role"]
        assert row["configurable"] is False
        assert row["value"] not in (None, "")


def test_semantic_mapping_is_total_and_classified():
    text = (ROOT / "docs/super_livo/evidence/round12_super_lio_vs_fast_livo2_lio_semantics.md").read_text()
    for role in (
        "blind", "max range", "point filtering", "scan voxel", "map resolution",
        "IESKF iterations", "convergence epsilon", "neighbor count", "search extent",
        "max correspondence distance", "plane point count", "plane threshold",
        "eigen/planarity test", "IMU acc noise", "IMU gyro noise", "bias RW",
        "LiDAR–IMU extrinsic",
    ):
        assert f"| {role} |" in text
    found = set(re.findall(r"\b[A-Z][A-Z_]+\b", text)) & ALLOWED
    assert "UNKNOWN" in found
    assert found <= ALLOWED


def test_reference_bases_have_complete_lineage():
    base = ROOT / "configs/super_livo/reference_base"
    for name in ("ntu_eee_01", "ntu_nya_01", "mcd_day10", "mcd_night08",
                 "oxford_quarter01", "m3dgr_corridor01"):
        config = load_yaml(base / f"{name}.yaml")
        lineage = load_yaml(base / f"{name}.lineage.yaml")
        assert config["reference_base"]["visual_apply"] is False
        fields = lineage["critical_parameters"]
        assert CRITICAL <= fields.keys()
        assert all(value not in (None, "") for value in fields.values())
        assert lineage["sensor_calibration_authority"] == "DATASET_AUTHOR_SENSOR"


if __name__ == "__main__":
    test_parent_bundles_have_machine_provenance()
    test_hardcoded_inventory_has_sources_and_roles()
    test_semantic_mapping_is_total_and_classified()
    test_reference_bases_have_complete_lineage()
    print("ROUND12 CONFIG PROVENANCE TDD: ALL PASS")
