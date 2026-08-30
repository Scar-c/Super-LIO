#!/usr/bin/env python3
"""G-P1.F5 — real NTU extrinsic seam (config-aware).

Resolves the LiDAR->IMU extrinsic from the production config
(src/super_lio/config/NTU.yaml) exactly as the production loader consumes it
(ROSWrapper.cpp LoadParamFromRos: first 3 values = t_LI, next 9 = R_LI), and
verifies:

  1. R_LI and t_LI are resolved;
  2. t_LI is nonzero for the canonical NTU config;
  3. for a representative IMU-frame point, the corrected lidar-frame path
     Sigma_I = R_LI Calc(R_LI^T (p_I - t_LI)) R_LI^T
     is NOT mathematically identical to the Prompt-2 shortcut Calc(p_I).

Persists a compact evidence artifact (--out).
"""
import argparse
import math
import pathlib
import sys

import numpy as np
import yaml

REPO = pathlib.Path(__file__).resolve().parents[2]


def ref_calc_body_cov(p, dept, beam):
    p = np.asarray(p, dtype=float).copy()
    if p[2] == 0.0:
        p[2] = 0.0001
    rng = float(np.sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]))
    range_var = dept * dept
    deg2rad = 0.017453293  # PCL pcl_macros.h:150 (what the FAST-LIVO2 build uses)
    s2 = math.sin(beam * deg2rad) ** 2
    direction = p / np.linalg.norm(p)
    dhat = np.array([[0, -direction[2], direction[1]],
                     [direction[2], 0, -direction[0]],
                     [-direction[1], direction[0], 0]], dtype=float)
    b1 = np.array([1.0, 1.0, -(direction[0] + direction[1]) / direction[2]])
    b1 /= np.linalg.norm(b1)
    b2 = np.cross(b1, direction)
    b2 /= np.linalg.norm(b2)
    N = np.stack([b1, b2], axis=1)
    A = rng * dhat @ N
    cov = (direction * range_var)[:, None] * direction[None, :]
    cov = cov + A @ np.diag([s2, s2]) @ A.T
    return cov


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=pathlib.Path,
                    default=REPO / "results/prob_lio/gp1f5_ntu_extrinsic.yaml")
    args = ap.parse_args()

    cfg_path = REPO / "src/super_lio/config/NTU.yaml"
    cfg = yaml.safe_load(cfg_path.read_text())
    ext = cfg["lio"]["extrinsic"]["lidar_imu"]
    t = np.asarray(ext[:3], dtype=float)
    r = np.asarray(ext[3:12], dtype=float).reshape(3, 3)
    dept = float(cfg["lio"]["sensor"]["dept_err"])
    beam = float(cfg["lio"]["sensor"]["beam_err"])

    p_I = np.array([5.0, -2.0, 8.0])  # representative IMU-frame point
    p_L = r.T @ (p_I - t)
    sigma_I = r @ ref_calc_body_cov(p_L, dept, beam) @ r.T
    sigma_wrong = ref_calc_body_cov(p_I, dept, beam)
    diff = float(np.abs(sigma_I - sigma_wrong).max())

    checks = {
        "R_LI_identity": bool(np.allclose(r, np.eye(3), atol=1e-12)),
        "t_LI_nonzero": bool(np.linalg.norm(t) > 1e-12),
        "t_LI": t.tolist(),
        "wrong_vs_correct_max_abs_diff": diff,
        "paths_not_identical": diff > 1e-9,
    }
    result = {
        "gate": "G-P1.F5",
        "config": str(cfg_path),
        "extrinsic_lidar_imu": ext,
        "R_LI": r.tolist(),
        "t_LI": t.tolist(),
        "dept_err_m": dept,
        "beam_err_deg": beam,
        "representative_point_IMU": p_I.tolist(),
        "representative_point_LIDAR": p_L.tolist(),
        "checks": checks,
        "pass": bool(checks["t_LI_nonzero"] and checks["paths_not_identical"]),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(yaml.safe_dump(result, sort_keys=False))
    print(f"R_LI =\n{r}")
    print(f"t_LI = {t}")
    print(f"t_LI nonzero: {checks['t_LI_nonzero']}")
    print(f"wrong-vs-correct max abs diff: {diff:.3e}")
    print(f"paths_not_identical: {checks['paths_not_identical']}")
    print(f"RESULT: {'PASS' if result['pass'] else 'FAIL'}")
    print(f"evidence: {args.out}")
    sys.exit(0 if result["pass"] else 1)


if __name__ == "__main__":
    main()
