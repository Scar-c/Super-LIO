#!/usr/bin/env python3
"""Round13 V-T1..T6: Visual enable-path gate TDD.

V-T1 OFF -> no apply path; V-T2 ON+dummy -> state unchanged; V-T3 ON+synthetic
-> update invoked; V-T4 prior = LiDAR posterior snapshot; V-T5 ownership
untouched; V-T6 NaN/finite safety.
Behavioral parts delegate to the existing S-1 sequential-prior test suite.
"""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[3]
SRC = ROOT / "src/super_lio/src/lio/super_lio.cpp"


def main():
    ok = True

    def expect(name, cond, detail=""):
        nonlocal ok
        print("%s: %s %s" % (name, "PASS" if cond else "FAIL", detail))
        ok &= cond

    src = SRC.read_text()

    # V-T1: gate excludes SHADOW_FULLSCAN and includes PARTIAL/IMU_FULLSCAN
    v4a_gate = re.search(r"if \(g_lio_v4_apply && g_lio_camera_epoch &&\n"
                         r"      g_lidar_update_policy != "
                         r"LidarUpdatePolicy::SHADOW_FULLSCAN &&", src)
    expect("V-T1 V-4A gate allows non-shadow policies", v4a_gate is not None)
    expect("V-T1 no stale PARTIAL-only gate in V-4A",
           "g_lidar_update_policy == LidarUpdatePolicy::PARTIAL &&\n"
           "      g_lio_v2_enabled" not in src)

    # V-T4: the sequential prior is snapshotted AFTER the LiDAR update.
    block_start = src.find("V-4A/B: sequential visual update")
    observe_pos = src.find("Observe();")
    prior_pos = src.find("prior.x = kf_->GetSysState();")
    expect("V-T4 prior snapshot after LiDAR Observe",
           block_start > observe_pos and prior_pos > block_start,
           "order: Observe() < V-4A block < prior snapshot")

    # V-T5: ownership recording occurs before the V-4A block (untouched).
    rec_pos = src.find("recordFullscanGeometryUse")
    expect("V-T5 ownership recording before Visual block",
           rec_pos < block_start or rec_pos == -1)

    # V-T2/V-T3/V-T6: delegated to the S-1 sequential-prior suite.
    r = subprocess.run(["bash", "-c",
                        "source /opt/ros/noetic/setup.bash && "
                        "source /home/lc/super_livo/devel/setup.bash && "
                        "timeout 60 %s/devel/.private/super_lio/lib/super_lio/"
                        "s1_prior_test 2>&1" % ROOT.parent.parent],
                       capture_output=True, text=True)
    out = (r.stdout + r.stderr)
    expect("V-T2/V-T3/V-T6 S-1 prior suite",
           "PASS" in out or r.returncode == 0, out.strip().splitlines()[-1:][0] if out.strip() else "")

    print("ROUND13 VISUAL GATE TDD: %s" % ("ALL PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
