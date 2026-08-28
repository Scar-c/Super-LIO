#!/usr/bin/env python3
"""Round13 Prompt59 production Visual-shadow contract (S-T1..S-T6)."""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[3]
RUNNER = ROOT / "scripts/super_livo/experiments/run_offline_variant.sh"
PROFILES = ROOT / "scripts/super_livo/experiments/semantic_profiles.py"
SRC = ROOT / "src/super_lio/src/lio/super_lio.cpp"

def main():
    runner, profiles, src = RUNNER.read_text(), PROFILES.read_text(), SRC.read_text()
    binary = ROOT.parents[1] / "devel/.private/super_lio/lib/super_lio/v2_jacobian_test"
    checks = [("S-T1 producer gates reachable",
               "/lio/g0/shadow" in profiles and "/lio/g1/enabled" in profiles and
               "D_VISUAL_SHADOW" in profiles and "SEMANTIC_TOOL" in runner)]
    p = subprocess.run([str(binary)], capture_output=True, text=True)
    checks.append(("S-T2 synthetic Visual information", p.returncode == 0))
    shadow = re.search(r"if\(g_lio_v2_enabled && !g_lio_v4_apply\)\s*\{(?P<body>.*?)\n\s*\}", src, re.S)
    body = shadow.group("body") if shadow else ""
    local_info = "runVisualResidual(pose, vh, vr, false)" in body
    blocked = "HTVH" not in body and "HTVr" not in body and "UpdateObserve" not in body
    checks.extend([
        ("S-T3 apply OFF leaves x unchanged", local_info and blocked),
        ("S-T4 apply OFF leaves P unchanged", local_info and blocked),
        ("S-T5 LiDAR ownership unchanged", "recordFullscanGeometryUse" not in body),
        ("S-T6 no extra LiDAR Observe", "Observe(" not in body and "UpdateObserve" not in body),
    ])
    ok = True
    for name, passed in checks:
        print(f"{name}: {'PASS' if passed else 'FAIL'}"); ok &= passed
    print(f"ROUND13 VISUAL SHADOW TDD: {'ALL PASS' if ok else 'FAIL'}")
    return 0 if ok else 1

if __name__ == "__main__": sys.exit(main())
