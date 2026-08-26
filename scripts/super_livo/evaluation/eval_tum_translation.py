#!/usr/bin/env python3
"""Translation-APE evaluator: TUM trajectories, rigid SE(3) Umeyama, no scale.
Physical frame being compared is caller-supplied via --prism (body-frame
offset of the GT tracked point), default identity."""
import argparse

import sys, math
import numpy as np

def quat_to_rot(q):
    x, y, z, w = q
    return np.array([
        [1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
        [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
        [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]])

def load_tum(p):
    ts, pos, quats = [], [], []
    for line in open(p):
        v = list(map(float, line.split()))
        if len(v) < 8: continue
        ts.append(v[0]); pos.append(v[1:4]); quats.append(v[4:8])
    return np.array(ts), np.array(pos), np.array(quats)

def umeyama(src, dst):
    n = src.shape[0]
    mu_s = src.mean(0); mu_d = dst.mean(0)
    S = (src - mu_s).T @ (dst - mu_d) / n
    U, D, Vt = np.linalg.svd(S)
    d = np.sign(np.linalg.det(Vt.T @ U.T))
    D = np.diag([1,1,d])
    R = Vt.T @ D @ U.T
    t = mu_d - R @ mu_s
    return R, t

def parse_args():
    ap = argparse.ArgumentParser(description='TUM translation APE (SE3, no scale)')
    ap.add_argument('est'); ap.add_argument('gt')
    ap.add_argument('--prism', default='0,0,0', help='body-frame offset of GT target (comma)')
    ap.add_argument('--frame', default='body', help='physical frame being compared')
    ap.add_argument('--max_diff', type=float, default=0.05, help='association max |dt| s')
    return ap.parse_args()

def main():
    est_tum, gt_tum, prism, out = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
    t_prism = np.array(list(map(float, prism.split(','))))
    et, ep, eq = load_tum(est_tum)
    gt, gp, gq = load_tum(gt_tum)
    ep2 = np.zeros_like(ep)
    for i in range(len(ep)):
        R = quat_to_rot(eq[i])
        ep2[i] = ep[i] + R @ t_prism
    lo = max(et[0], gt[0]); hi = min(et[-1], gt[-1])
    m_est, m_gt = [], []
    for i in range(len(et)):
        if lo <= et[i] <= hi:
            j = np.argmin(np.abs(gt - et[i]))
            m_est.append(ep2[i]); m_gt.append(gp[j])
    A = np.array(m_est); B = np.array(m_gt)
    R, t = umeyama(A, B)
    aligned = (R @ A.T).T + t
    errs = np.linalg.norm(aligned - B, axis=1)
    print('matched samples: %d  duration: %.1f s' % (len(errs), hi - lo))
    print('ATE translation APE (m): RMSE=%.4f mean=%.4f median=%.4f max=%.4f P90=%.4f P95=%.4f' %
          (np.sqrt((errs**2).mean()), errs.mean(), np.median(errs), errs.max(),
           np.percentile(errs, 90), np.percentile(errs, 95)))
    with open(out, 'w') as f:
        f.write('RMSE %.6f\nmean %.6f\nmedian %.6f\nmax %.6f\nP90 %.6f\nP95 %.6f\nmatched %d\n' %
                (np.sqrt((errs**2).mean()), errs.mean(), np.median(errs), errs.max(),
                 np.percentile(errs, 90), np.percentile(errs, 95), len(errs)))

if __name__ == '__main__':
    main()