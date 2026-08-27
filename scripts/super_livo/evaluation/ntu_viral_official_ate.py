#!/usr/bin/env python3
"""NTU VIRAL official ATE evaluation (per the dataset evaluation tutorial /
ntuviral_evaluate.ipynb).

Official semantics:
  1. prism compensation: P_est += quatconv(Q_est, T_B_prism), T_B_prism =
     (-0.293656, -0.012288, -0.273095)  [body -> prism]
  2. resample: for each estimate sample, interpolate the ground-truth at the
     estimate time using the temporally preceding/succeeding GT samples; drop
     the estimate sample if either neighbour is too far away (0.1 s window).
  3. align: Umeyama (closed-form SE(3)) between estimate and (interpolated)
     ground truth.
  4. ATE = norm( per-axis RMS( position error ) ).

Inputs:
  --est trajectory in TUM (t tx ty tz qx qy qz qw; body frame, quat x y z w)
  --gt leica_pose.csv (timestamp_ns, ... x y z ...) per the tutorial (position
     read from columns 4..6 1-based; we accept the raw ns + xyz columns here)
  --window association window (default 0.1 s)
"""
import argparse
import math
import numpy as np
import sys


def quat_rotate(q, v):
    # q = (x, y, z, w) -> rotate v
    q = q / np.linalg.norm(q)
    x, y, z, w = q
    R = np.array([
        [1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)],
        [2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
        [2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)],
    ])
    return R @ v


def umeyama(S, D):
    """Closed-form SE(3) alignment: R,t minimizing |R S + t - D|."""
    mu_s = S.mean(0)
    mu_d = D.mean(0)
    Sc = S - mu_s
    Dc = D - mu_d
    H = Sc.T @ Dc
    U, _, Vt = np.linalg.svd(H)
    R = Vt.T @ U.T
    if np.linalg.det(R) < 0:
        Vt[-1, :] *= -1
        R = Vt.T @ U.T
    t = mu_d - R @ mu_s
    return R, t


def load_est_tum(path):
    d = np.loadtxt(path)
    return d[:, 0], d[:, 1:4], d[:, 4:8]


def load_gt_csv(path):
    rows = np.loadtxt(path, skiprows=1)
    # col0 = timestamp(ns), cols 3,4,5 = x,y,z (per tutorial 4..6 1-based)
    t = rows[:, 0] / 1e9
    P = rows[:, 3:6]
    return t, P


def interpolate_gt(t_gt, P_gt, t_est, window):
    """For each est time, interpolate GT; drop if neighbours > window away."""
    idx = np.searchsorted(t_gt, t_est)
    P_itp = np.full((len(t_est), 3), np.nan)
    for i, ti in enumerate(t_est):
        j = idx[i]
        if j == 0 or j >= len(t_gt):
            continue
        t0, t1 = t_gt[j - 1], t_gt[j]
        if ti - t0 > window or t1 - ti > window:
            continue
        a = (ti - t0) / (t1 - t0)
        P_itp[i] = (1 - a) * P_gt[j - 1] + a * P_gt[j]
    return P_itp


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--est", required=True, help="estimate TUM (body frame)")
    ap.add_argument("--gt", required=True, help="leica_pose.csv")
    ap.add_argument("--window", type=float, default=0.1)
    args = ap.parse_args()

    t_est, P_est, Q_est = load_est_tum(args.est)
    t_gt, P_gt = load_gt_csv(args.gt)

    # prism compensation
    T_B2prism = np.array([-0.293656, -0.012288, -0.273095])
    P_prism = P_est + np.array([quat_rotate(q, T_B2prism) for q in Q_est])

    # resample GT at estimate times
    P_gt_itp = interpolate_gt(t_gt, P_gt, t_est, args.window)
    keep = ~np.isnan(P_gt_itp[:, 0])
    P_est_use = P_prism[keep]
    P_gt_use = P_gt_itp[keep]

    if len(P_est_use) < 10:
        print("ERROR: too few associated samples:", len(P_est_use))
        return 1

    # Umeyama alignment
    R, tt = umeyama(P_est_use, P_gt_use)
    P_est_aligned = (R @ P_est_use.T).T + tt

    err = P_gt_use - P_est_aligned
    rms_axis = np.sqrt(np.mean(err ** 2, axis=0))
    ate = np.linalg.norm(rms_axis)

    print("NTU VIRAL official ATE (prism-compensated, %.2f s window)" % args.window)
    print("  associated samples: %d / %d" % (len(P_est_use), len(P_est)))
    print("  per-axis RMS (m): %.6f %.6f %.6f" % tuple(rms_axis))
    print("  ATE (m): %.6f" % ate)
    return 0


if __name__ == "__main__":
    sys.exit(main())
