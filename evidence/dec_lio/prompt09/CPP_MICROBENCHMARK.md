# C++ microbenchmark

Executable: `/home/lc/dec_lio/devel/lib/super_lio/dec_lio_d3_solver_benchmark`
Snapshots: 192
Warmup/repetitions: 500/500 per method and system.

native_inverse_us: n=192 median=6.7176 P95=7.97611 P99=11.1129 max=11.8324
ldlt_factor_solve_us: n=192 median=2.0896 P95=2.42204 P99=3.62356 max=3.65681
cg_us: n=192 median=7.21388 P95=8.49139 P99=12.2194 max=12.5115
jacobi_setup_us: n=192 median=0.246952 P95=0.282572 P99=0.401298 max=0.415546
jacobi_solve_us: n=192 median=4.45703 P95=5.85564 P99=7.49408 max=7.73628
jacobi_total_us: n=192 median=4.69211 P95=6.13346 P99=7.76478 max=8.15183
dcreg_setup_us: n=192 median=11.1651 P95=13.288 P99=18.3339 max=18.5262
dcreg_solve_us: n=192 median=5.04354 P95=6.96693 P99=9.0874 max=9.16814
dcreg_total_us: n=192 median=16.1469 P95=20.1291 P99=26.7422 max=27.502

Convergence: CG 7/192, Jacobi 192/192, DCReg 192/192; DCReg preconditioner valid 192/192.

By scene:
- bridge01 n=64 LDLT=n=64 median=2.2297 P95=2.83284 P99=3.65681 max=3.65681 CG=n=64 median=7.73153 P95=10.0895 P99=12.5115 max=12.5115 Jacobi-total=n=64 median=5.43772 P95=7.1379 P99=8.15183 max=8.15183 DCReg-total=n=64 median=18.0347 P95=22.1094 P99=27.502 max=27.502 iterations CG/J/D=n=64 median=36 P95=36 P99=36 max=36/n=64 median=24 P95=27 P99=27 max=27/n=64 median=28 P95=32 P99=34 max=34
- stairs_alpha n=64 LDLT=n=64 median=2.02549 P95=2.33181 P99=3.62356 max=3.62356 CG=n=64 median=7.1094 P95=8.04972 P99=8.47239 max=8.47239 Jacobi-total=n=64 median=4.53064 P95=5.41397 P99=5.6633 max=5.6633 DCReg-total=n=64 median=15.767 P95=18.9774 P99=20.1291 max=20.1291 iterations CG/J/D=n=64 median=36 P95=36 P99=36 max=36/n=64 median=22 P95=24 P99=25 max=25/n=64 median=25 P95=28 P99=31 max=31
- tunnel2 n=64 LDLT=n=64 median=2.03974 P95=2.23445 P99=2.30569 max=2.30569 CG=n=64 median=6.96456 P95=7.53207 P99=7.81227 max=7.81227 Jacobi-total=n=64 median=4.43566 P95=5.35936 P99=5.45672 max=5.45672 DCReg-total=n=64 median=15.8644 P95=16.8569 P99=17.0659 max=17.0659 iterations CG/J/D=n=64 median=36 P95=36 P99=36 max=36/n=64 median=22 P95=25 P99=27 max=27/n=64 median=25 P95=29 P99=31 max=31
