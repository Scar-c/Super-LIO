# Solver iteration comparison

Strict PCG config: max_iterations=36, relative_tolerance=1e-12, absolute_tolerance=1e-14.

all n=33632 | CG n=33632 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=33632 median=22 P95=26 P99=27 max=30 | DCReg-PCG n=33632 median=25 P95=31 P99=34 max=36 | failures CG/J/D=32624/0/21
low_kappa n=15574 | CG n=15574 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=15574 median=21 P95=26 P99=27 max=29 | DCReg-PCG n=15574 median=24 P95=32 P99=34 max=36 | failures CG/J/D=15359/0/6
middle_kappa n=16376 | CG n=16376 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=16376 median=22 P95=25 P99=27 max=30 | DCReg-PCG n=16376 median=26 P95=30 P99=33 max=36 | failures CG/J/D=15618/0/15
high_condition_P95 n=1682 | CG n=1682 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=1682 median=20 P95=22 P99=25 max=26 | DCReg-PCG n=1682 median=22 P95=25 P99=28 max=31 | failures CG/J/D=1678/0/0
high_kappa_R_P95 n=1682 | CG n=1682 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=1682 median=22 P95=27 P99=29 max=30 | DCReg-PCG n=1682 median=25 P95=28 P99=30 max=36 | failures CG/J/D=1647/0/0
high_kappa_t_P95 n=1682 | CG n=1682 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=1682 median=24 P95=27 P99=28 max=29 | DCReg-PCG n=1682 median=29 P95=34 P99=36 max=36 | failures CG/J/D=1601/0/9
weak_rank_0 n=15574 | CG n=15574 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=15574 median=21 P95=26 P99=27 max=29 | DCReg-PCG n=15574 median=24 P95=32 P99=34 max=36 | failures CG/J/D=15359/0/6
weak_rank_1 n=18044 | CG n=18044 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=18044 median=22 P95=26 P99=27 max=30 | DCReg-PCG n=18044 median=26 P95=30 P99=32 max=36 | failures CG/J/D=17253/0/15
weak_rank_2plus n=27 | CG n=27 median=36 P95=36 P99=36 max=36 | Jacobi-PCG n=27 median=22 P95=24 P99=24 max=24 | DCReg-PCG n=27 median=28 P95=33 P99=35 max=35 | failures CG/J/D=19/0/0

DCReg-specific value is separated from generic Jacobi preconditioning; iteration reduction alone is not treated as production authorization.
