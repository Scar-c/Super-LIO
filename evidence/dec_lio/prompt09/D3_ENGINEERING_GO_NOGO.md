# D3 engineering GO/NO-GO

PROMPT09 STATUS:

Git:
- start HEAD: da21a98e1307728ee9b9db5512beb29f6a062125
- final HEAD: c2f3ca387034b0c9b04f1d1ce666aa8574a42842
- origin/Dec-LIO: checked at final push
- merge-base: 60b57aaac8dc397f80c56364e7ccb008c300cc29
- worktree: clean after final evidence commit

Native solver authority:
- source file/function: src/super_lio/src/lio/ESKF.cpp, ESKF::UpdateObserve, lines 251-334
- native equation: dx_native = Q*b + (Q*Hbar-I)*d
- P_pred meaning: pre-LiDAR propagated filter prior
- pose ordering: rotation, position, velocity, gyro bias, accelerometer bias, gravity
- reformulated equation: A=Lambda+Hbar, A*dx=r
- rhs equation: r=b-Lambda*d, Lambda=Pk^-1

Reformulation equivalence:
- systems tested: 33632
- dx relative error median/P99/max: n=33632 median=1.40292e-06 P95=0.000117072 P99=0.000539803 max=0.013334
- original residual median/P99/max: n=33632 median=1.41107e-07 P95=0.000162776 P99=0.0006524 max=0.0196996
- PASS/FAIL: FAIL — required thresholds are not met

Matrix authority:
- A asymmetry median/P95/max: n=33632 median=1.64476e-09 P95=8.84643e-09 P99=2.57155e-08 max=1.43161e-06
- LLT success fraction: 33632/33632 (33632/33632 (100.000%))
- LDLT success fraction: 33632/33632 (33632/33632 (100.000%))
- fused lambda_min: n=33632 median=9483.78 P95=13371.2 P99=13914.6 max=14226.7
- fused condition median/P95/max: n=33632 median=8212.69 P95=73574.5 P99=94708.3 max=1.59533e+06

DCReg source authority:
- pinned commit: 8ce8451b15491a4bbe17cf85ab02a8bed6696861
- exact PCG/preconditioner source: DCReg/include/dcreg.hpp, utils.hpp at pinned checkout
- M or M^-1 semantics: left preconditioner M applied as z=M*r
- eigenvalue conditioning: aligned Schur eigenvalues clamped only inside M
- kappa target: 10 (Prompt09 strict audit; upstream stopping defaults not inherited)
- failure policy: invalid DCReg construction fails open to Jacobi/identity in shadow

Full-state lift:
- pose preconditioner: aligned rotation/translation 3x3 inverse clamped blocks
- nuisance preconditioner: inverse diagonal of fused A for states 6..17
- SPD validation: finite self-adjoint eigenvalue check; invalid -> fail-open
- A modified: MUST BE NO
- rhs modified: MUST BE NO

Synthetic:
- T1-T14: PASS
- objective invariance: PASS (P09-N1)
- wrong-A negative control: PASS
- PASS/FAIL: PASS

Native parity:
- Bridge01 Alpha: expected/actual 6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203/6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203, cmp=PASS
- Stairs_Alpha: expected/actual 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11/26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11, cmp=PASS
- Tunneling_tunnel2_Alpha: expected/actual 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30/3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30, cmp=PASS
Bridge shadow:
scene=Bridge01 Alpha systems=12169 ex=n=12169 median=1.68714e-06 P95=6.78299e-05 P99=0.000301956 max=0.00389333 residual=n=12169 median=1.65265e-07 P95=6.33095e-05 P99=0.00037128 max=0.00573087 asym=n=12169 median=5.61955e-10 P95=3.31204e-09 P99=5.22858e-09 max=1.36627e-08 ldlt_err=n=12169 median=2.02119e-06 P95=8.08456e-05 P99=0.000345969 max=0.00450033 A_condition=n=12169 median=37719.1 P95=91918.1 P99=96316.1 max=109251 kappa_R=n=12169 median=3.64987 P95=8.9549 P99=11.86 max=17.0965 kappa_t=n=12169 median=3.01546 P95=9.22611 P99=13.1962 max=20.252


Stairs shadow:
scene=Stairs_Alpha systems=12322 ex=n=12322 median=1.37328e-06 P95=0.000141131 P99=0.000544951 max=0.00580064 residual=n=12322 median=1.38711e-07 P95=0.000165083 P99=0.00057268 max=0.00793728 asym=n=12322 median=3.54175e-09 P95=1.45693e-08 P99=4.41753e-08 max=1.0265e-07 ldlt_err=n=12322 median=1.55968e-06 P95=0.000152283 P99=0.00055605 max=0.00720066 A_condition=n=12322 median=2122.87 P95=16390.3 P99=63038.8 max=79706.6 kappa_R=n=12322 median=14.2668 P95=60.8901 P99=84.3062 max=183.596 kappa_t=n=12322 median=3.5684 P95=8.74906 P99=14.069 max=21.3636


Tunnel2 shadow:
scene=Tunneling_tunnel2_Alpha systems=9141 ex=n=9141 median=1.18737e-06 P95=0.000165776 P99=0.000879685 max=0.013334 residual=n=9141 median=1.17021e-07 P95=0.000316392 P99=0.000996453 max=0.0196996 asym=n=9141 median=1.66235e-09 P95=3.65559e-09 P99=7.45863e-09 max=1.43161e-06 ldlt_err=n=9141 median=1.32895e-06 P95=0.000186892 P99=0.000961792 max=0.0116935 A_condition=n=9141 median=8865.85 P95=11938.4 P99=21565.6 max=1.59533e+06 kappa_R=n=9141 median=60.0805 P95=90.0403 P99=99.214 max=119.336 kappa_t=n=9141 median=1.76484 P95=5.38741 P99=5.98932 max=9.41606


High-kappa subset:
- CG: n=1682 median=36 P95=36 P99=36 max=36
- Jacobi: n=1682 median=22 P95=27 P99=29 max=30
- DCReg: n=1682 median=25 P95=28 P99=30 max=36
- interpretation: DCReg is not better than generic Jacobi on the selected high-kappa authority; reformulation gate already fails.

High fused-condition subset:
- CG: n=1682 median=36 P95=36 P99=36 max=36
- Jacobi: n=1682 median=20 P95=22 P99=25 max=26
- DCReg: n=1682 median=22 P95=25 P99=28 max=31
- interpretation: Jacobi remains lower-iteration than DCReg; no production inference is authorized after Gate A failure.

LiDAR vs fused conditioning:
- correlation: log(kappa_max(LiDAR), condition(A)) Pearson=-0.433097
- examples where LiDAR is weak but fused A is well conditioned: 241 systems with LiDAR kappa at global P95 and A condition at/below fused median
- implication: the prior changes fused solver conditioning; it does not erase LiDAR geometric degeneracy.

C++ microbenchmark:
- snapshot count: 192
- repeats: warmup=500, timed=500
- native inverse median/P95 us: n=192 median=6.7176 P95=7.97611 P99=11.1129 max=11.8324 / 7.976112
- LDLT median/P95 us: n=192 median=2.0896 P95=2.42204 P99=3.62356 max=3.65681 / 2.42204
- CG median/P95 us: n=192 median=7.21388 P95=8.49139 P99=12.2194 max=12.5115 / 8.491388
- Jacobi setup/solve/total: n=192 median=0.246952 P95=0.282572 P99=0.401298 max=0.415546 / n=192 median=4.45703 P95=5.85564 P99=7.49408 max=7.73628 / n=192 median=4.69211 P95=6.13346 P99=7.76478 max=8.15183
- DCReg setup/solve/total: n=192 median=11.1651 P95=13.288 P99=18.3339 max=18.5262 / n=192 median=5.04354 P95=6.96693 P99=9.0874 max=9.16814 / n=192 median=16.1469 P95=20.1291 P99=26.7422 max=27.502
- fastest method: ldlt_factor_solve_us
- P95 fastest method: ldlt_factor_solve_us

DCReg-specific gain:
- vs CG iteration improvement: real all-system median 36 -> 25 (30.6%)
- vs Jacobi improvement: real all-system median 22 -> 25 (-13.6%, negative)
- vs LDLT total time: benchmark DCReg total median / LDLT median = 7.73x
- stability advantage: NO; reformulation/asymmetry gates fail and real DCReg non-convergence=21
- demonstrated YES/NO: NO

Primary classification:
- D3-A / D3-B / D3-C / D3-D / D3-E: D3-E — FUSED_SYSTEM_REFORMULATION_AUTHORITY_FAILURE

Engineering decision:
- production DCReg-PCG integration next: NO
- generic LDLT cleanup worth pursuing: NO in this prompt because fused authority is unresolved
- abandon PCG path: YES for production until authority is repaired

Boundary:
- PCG controls estimator dx: MUST BE NO
- H modified: NO
- b modified: NO
- P modified: NO
- map modified: NO
- gamma: NO
- Prob-LIO: NO

STATUS:
PARTIAL / STOP — FUSED_SYSTEM_REFORMULATION_NOT_NUMERICALLY_EQUIVALENT; secondary symmetry STOP
