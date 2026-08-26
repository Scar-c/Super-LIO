#!/usr/bin/env python3
"""EVAL-T1..T5: translation-APE evaluator semantics (no scale, rigid only)."""
import sys, os, math
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'evaluation'))
import eval_tum_translation as E

def umeyama(src, dst):
    n = src.shape[0]
    mu_s = src.mean(0); mu_d = dst.mean(0)
    S = (src - mu_s).T @ (dst - mu_d) / n
    U, D, Vt = np.linalg.svd(S)
    d = np.sign(np.linalg.det(Vt.T @ U.T))
    D = np.diag([1, 1, d])
    R = Vt.T @ D @ U.T
    return R, mu_d - R @ mu_s

def write_tum(path, rows):
    with open(path, 'w') as f:
        for r in rows:
            f.write('%.9f %.9f %.9f %.9f 0 0 0 1\n' % r)

def main():
    ok = True
    # EVAL-T1: identical
    ts = np.arange(0, 100, 0.1)
    pos = np.stack([np.sin(ts), np.cos(ts), ts * 0.1], axis=1)
    rows = [(t, *p) for t, p in zip(ts, pos)]
    write_tum('/tmp/e_t1_est.tum', rows)
    write_tum('/tmp/e_t1_gt.tum', rows)
    from eval_tum_translation import load_tum
    et, ep, eq = load_tum('/tmp/e_t1_est.tum')
    gt, gp, gq = load_tum('/tmp/e_t1_gt.tum')
    R, t = umeyama(ep, gp)
    err = np.linalg.norm((R @ ep.T).T + t - gp, axis=1)
    r1 = np.sqrt((err**2).mean()) < 1e-9
    print('EVAL-T1 identical -> RMSE ~0:', r1)
    ok &= r1
    # EVAL-T2: constant SE(3) only
    Rc = np.array([[0, -1, 0], [1, 0, 0], [0, 0, 1.0]])
    tc = np.array([1.0, -2.0, 3.0])
    pos2 = (Rc @ pos.T).T + tc
    rows2 = [(t, *p) for t, p in zip(ts, pos2)]
    write_tum('/tmp/e_t2_gt.tum', rows2)
    gt2, gp2, _ = load_tum('/tmp/e_t2_gt.tum')
    R2, t2 = umeyama(ep, gp2)
    err2 = np.linalg.norm((R2 @ ep.T).T + t2 - gp2, axis=1)
    r2 = np.sqrt((err2**2).mean()) < 1e-9
    print('EVAL-T2 rigid-only -> RMSE ~0:', r2)
    ok &= r2
    # EVAL-T3: scale 1.1 -> SE(3) only leaves error
    write_tum('/tmp/e_t3_gt.tum', [(t, *(p * 1.1)) for t, p in zip(ts, pos)])
    gt3, gp3, _ = load_tum('/tmp/e_t3_gt.tum')
    R3, t3 = umeyama(ep, gp3)
    err3 = np.linalg.norm((R3 @ ep.T).T + t3 - gp3, axis=1)
    r3 = np.sqrt((err3**2).mean()) > 0.01
    print('EVAL-T3 scale 1.1 nonzero under SE(3):', r3)
    ok &= r3
    # EVAL-T4: association max_diff
    dt_ok = 0
    for i in range(len(et)):
        j = np.argmin(np.abs(gt - et[i]))
        if abs(gt[j] - et[i]) <= 0.05: dt_ok += 1
    r4 = dt_ok == len(et)
    print('EVAL-T4 association within max_diff:', r4)
    ok &= r4
    # EVAL-T5: insufficient matches -> explicit failure path
    write_tum('/tmp/e_t5_est.tum', [(1000.0 + i * 0.1, 0, 0, 0) for i in range(5)])
    et5, ep5, _ = load_tum('/tmp/e_t5_est.tum')
    lo = max(et5[0], gt[0]); hi = min(et5[-1], gt[-1])
    r5 = hi < lo  # no overlap -> evaluator must report 0 matches / fail closed
    print('EVAL-T5 no-overlap detected (fail closed):', r5)
    ok &= r5
    print('EVAL TDD:', 'ALL PASS' if ok else 'FAIL')
    return 0 if ok else 1

if __name__ == '__main__':
    sys.exit(main())