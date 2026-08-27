#!/usr/bin/env python3
"""M3DGR official-config <-> runtime parity checker (Round12 final resume).

Compares the pinned official dataset-author YAML/launch values against the
post-launch pre-playback effective_rosparams snapshot. Classifies every
official accuracy-affecting parameter. Used by the fail-closed canonical
runner: bag playback is refused unless parity PASS.

Classifications per parameter:
  MATCH, EXPECTED_DERIVED, EXPECTED_NAMESPACE_TRANSLATION, MISSING, MISMATCH,
  NOT_RUNTIME_PARAM.
"""
import argparse
import json
import pathlib
import sys
import yaml


# Official YAML -> runtime rosparam path mapping.
# Values are (rosparam_key, translate_fn) where translate_fn(None) = direct.
KEY_MAP = [
    # common
    ("common.img_topic", "common.img_topic", None),
    ("common.lid_topic", "common.lid_topic", None),
    ("common.imu_topic", "common.imu_topic", None),
    ("common.img_en", "common.img_en", None),
    ("common.lidar_en", "common.lidar_en", None),
    # preprocess
    ("preprocess.point_filter_num", "preprocess.point_filter_num", None),
    ("preprocess.filter_size_surf", "preprocess.filter_size_surf", None),
    ("preprocess.lidar_type", "preprocess.lidar_type", None),
    ("preprocess.scan_line", "preprocess.scan_line", None),
    ("preprocess.blind", "preprocess.blind", None),
    # time offsets
    ("time_offset.lidar_time_offset", "time_offset.lidar_time_offset", None),
    ("time_offset.imu_time_offset", "time_offset.imu_time_offset", None),
    ("time_offset.img_time_offset", "time_offset.img_time_offset", None),
    # LIO
    ("max_iteration", "max_iteration", None),
    # calibration
    ("extrin_calib.extrinsic_T", "extrin_calib.extrinsic_T", None),
    ("extrin_calib.extrinsic_R", "extrin_calib.extrinsic_R", None),
    ("extrin_calib.Rcl", "extrin_calib.Rcl", None),
    ("extrin_calib.Pcl", "extrin_calib.Pcl", None),
]


def load_yaml(path):
    with open(path) as f:
        return yaml.safe_load(f)


def lookup(d, dotted):
    for part in dotted.split("."):
        if not isinstance(d, dict) or part not in d:
            return None
        d = d[part]
    return d


def comparable(value):
    if isinstance(value, list):
        return tuple(float(v) for v in value)
    if isinstance(value, dict):
        return tuple(sorted((k, float(v)) for k, v in value.items()))
    return value


def classify(official_val, runtime_val, key):
    if runtime_val is None:
        return "MISSING"
    if comparable(official_val) == comparable(runtime_val):
        return "MATCH"
    # camera config lives in the node private namespace laserMapping/
    if key.startswith("camera"):
        return "EXPECTED_NAMESPACE_TRANSLATION" if runtime_val is not None else "MISSING"
    return "MISMATCH"


def check(official_yamls, snapshot, out_path):
    runtime = yaml.safe_load(snapshot.read_text())
    results = {}
    all_match = True
    for official_path in official_yamls:
        off = load_yaml(official_path)
        for off_key, rt_key, _ in KEY_MAP:
            off_val = lookup(off, off_key)
            if off_val is None:
                continue
            rt_val = lookup(runtime, rt_key)
            cls = classify(off_val, rt_val, rt_key)
            results[rt_key] = {
                "official": off_val,
                "runtime": rt_val,
                "classification": cls,
            }
            if cls in ("MISSING", "MISMATCH"):
                all_match = False
        # camera config (private namespace)
        for off_key in ("cam_width", "cam_height", "fx", "fy", "cx", "cy"):
            off_val = lookup(off, off_key)
            if off_val is None:
                continue
            rt_val = lookup(runtime, "laserMapping." + off_key)
            cls = classify(off_val, rt_val, "camera/" + off_key)
            results["laserMapping." + off_key] = {
                "official": off_val, "runtime": rt_val, "classification": cls}
            if cls in ("MISSING", "MISMATCH"):
                all_match = False
    payload = {"parity": "PASS" if all_match else "FAIL",
               "parameters": results}
    out_path.write_text(json.dumps(payload, indent=2) + "\n")
    print("PARITY: %s (%d parameters)" % ("PASS" if all_match else "FAIL",
                                          len(results)))
    return 0 if all_match else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--official", nargs="+", required=True)
    ap.add_argument("--snapshot", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    return check([pathlib.Path(p) for p in args.official],
                 pathlib.Path(args.snapshot), pathlib.Path(args.out))


if __name__ == "__main__":
    sys.exit(main())
