# Dec-LIO history

## 2026-09-13 — Prompt00R recovery

- Re-established the independent checkout at `/home/lc/dec_lio/src/Super-LIO`.
- Created `Dec-LIO` from `origin/ros1` and verified
  `HEAD == merge-base == origin/ros1 == 60b57aa` before scaffold work.
- Preserved `/home/lc/prob_lio` as the historical Prob-LIO workspace. No
  artifacts there were deleted, moved, or stashed.
- Added the native-only GEODE Bridge01 Alpha baseline boundary and the
  algorithm-neutral offline transport/evaluation infrastructure.

## 2026-09-13 — Prompt01 closure

- Added the Stairs Alpha native offline D0 baseline and exact repeatability
  evidence. The local text reference is the official Alpha trajectory GT
  produced by GEODE's PALoc-to-GT-map procedure.
- Added the runtime-off D1 DCReg LiDAR-only 6DoF shadow characterization. It
  reads only post-reduction `H_L,b_L`, records per-iteration and per-frame
  CSVs, and fails open on invalid factorization/eigensolver results.
- Verified D1 OFF/ON trajectory identity for Bridge01 and Stairs Alpha, plus
  synthetic tests A–G and an independent Schur/EVD reference parity oracle.
- Stairs Alpha official-GT SE(3)-aligned translation ATE: RMSE 0.197750264 m.
