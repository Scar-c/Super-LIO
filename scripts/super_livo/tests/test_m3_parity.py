#!/usr/bin/env python3
"""M3-S1..S8 snapshot/parity TDD (Round12 final resume)."""
import json
import pathlib
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "super_livo/evaluation"))

from m3_official_runtime_parity import check

OFFICIAL = """\
common:
  img_topic: /camera/color/image_raw
  lid_topic: /livox/avia/lidar
  imu_topic: /livox/avia/imu
preprocess:
  point_filter_num: 2
  blind: 1
  filter_size_surf: 0.1
time_offset:
  lidar_time_offset: 0.0
  img_time_offset: 0.0
max_iteration: 5
extrin_calib:
  extrinsic_T: [0.0, 0.0, 0.0]
  extrinsic_R: [1, 0, 0, 0, 1, 0, 0, 0, 1]
cam_width: 640
cam_height: 480
fx: 200.0
"""

SNAPSHOT_GOOD = """\
common:
  img_topic: /camera/color/image_raw
  lid_topic: /livox/avia/lidar
  imu_topic: /livox/avia/imu
preprocess:
  point_filter_num: 2
  blind: 1
  filter_size_surf: 0.1
time_offset:
  lidar_time_offset: 0.0
  img_time_offset: 0.0
max_iteration: 5
extrin_calib:
  extrinsic_T: [0.0, 0.0, 0.0]
  extrinsic_R: [1, 0, 0, 0, 1, 0, 0, 0, 1]
laserMapping:
  cam_width: 640
  cam_height: 480
  fx: 200.0
"""

SNAPSHOT_BAD = SNAPSHOT_GOOD.replace("blind: 1", "blind: 5").replace(
    "max_iteration: 5", "max_iteration: 3")


def main():
    ok = True

    def expect(name, cond):
        nonlocal ok
        print("%s: %s" % (name, "PASS" if cond else "FAIL"))
        ok &= cond

    with tempfile.TemporaryDirectory(prefix="m3tdd-") as tmp:
        tmp = pathlib.Path(tmp)
        off = tmp / "official.yaml"
        off.write_text(OFFICIAL)
        snap = tmp / "snapshot.yaml"
        out = tmp / "parity.json"

        # M3-S1..S6: good snapshot -> all MATCH -> PASS
        snap.write_text(SNAPSHOT_GOOD)
        rc = check([off], snap, out)
        p = json.loads(out.read_text())
        expect("M3-S1 blind match", p["parameters"]["preprocess.blind"]["classification"] == "MATCH")
        expect("M3-S2 max_iteration match", p["parameters"]["max_iteration"]["classification"] == "MATCH")
        expect("M3-S3 img_time_offset match", p["parameters"]["time_offset.img_time_offset"]["classification"] == "MATCH")
        expect("M3-S4 topics match", p["parameters"]["common.lid_topic"]["classification"] == "MATCH"
               and p["parameters"]["common.imu_topic"]["classification"] == "MATCH"
               and p["parameters"]["common.img_topic"]["classification"] == "MATCH")
        expect("M3-S5 extrinsics match", p["parameters"]["extrin_calib.extrinsic_R"]["classification"] == "MATCH")
        expect("M3-S6 camera config loaded (namespace translation)",
               p["parameters"]["laserMapping.cam_width"]["classification"] == "MATCH")
        expect("M3-S6b parity PASS", rc == 0 and p["parity"] == "PASS")

        # M3-S7: tampered copied config -> MISMATCH -> FAIL (blocks playback)
        snap.write_text(SNAPSHOT_BAD)
        rc = check([off], snap, out)
        p = json.loads(out.read_text())
        expect("M3-S7 tamper -> blind MISMATCH",
               p["parameters"]["preprocess.blind"]["classification"] == "MISMATCH")
        expect("M3-S7 tamper -> max_iteration MISMATCH",
               p["parameters"]["max_iteration"]["classification"] == "MISMATCH")
        expect("M3-S7 parity FAIL (playback blocked)", rc != 0 and p["parity"] == "FAIL")

        # M3-S8: missing snapshot -> parity cannot run -> blocked
        missing = tmp / "missing.yaml"
        try:
            check([off], missing, out)
            expect("M3-S8 missing snapshot blocks", False)
        except (FileNotFoundError, OSError):
            expect("M3-S8 missing snapshot blocks", True)

    print("M3-S1..S8: %s" % ("ALL PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
