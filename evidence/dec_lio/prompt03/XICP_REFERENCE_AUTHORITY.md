# X-ICP reference authority

- Repository: https://github.com/kelebujiabing/faster-lio
- Frozen reference commit: `f6daa633b8a4fd12be35399626c3e2d5d6264843`
- Relevant authority: `include/xicp.h`, `src/xicp.cc`, `config/velodyne_alpha.yaml`
- Alpha parameters: `kc=80°`, `ks=60°`, `k1=250`, `k2=180`, `k3=35`
- Classification: FULL if `Lc >= 250 OR Ls >= 180`; otherwise PARTIAL if
  `Lc >= 180 OR Ls >= 35`; otherwise NONE.

Dec-LIO reproduces localizability statistics only. It does not import the
X-ICP solver, matching, constrained optimization, or pose correction. The
statistics are named `XICP_ON_SUPER_RESIDUALS` and use only the exact accepted
native Super-LIO residual Jacobians from iteration 0.
