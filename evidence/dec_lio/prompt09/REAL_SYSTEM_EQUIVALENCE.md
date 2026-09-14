# Real-system equivalence

systems=33632
reformulation ex: n=33632 median=1.40292e-06 P95=0.000117072 P99=0.000539803 max=0.013334
reformulation original residual: n=33632 median=1.41107e-07 P95=0.000162776 P99=0.0006524 max=0.0196996
LDLT relative error vs native: n=33632 median=1.61976e-06 P95=0.000130769 P99=0.000595886 max=0.0116935

Hard gate checks:
- ex median/P99/max <= 1e-10/1e-8/1e-6: FAIL
- residual median/P99 <= 1e-12/1e-10: FAIL
- asymmetry max <= 1e-10: FAIL
- nonfinite ex/residual/asym: none observed in finite CSV rows

Asymmetry >1e-12: 33632; asymmetry >1e-10: 33510.
all n=33632 ex=n=33632 median=1.40292e-06 P95=0.000117072 P99=0.000539803 max=0.013334 residual=n=33632 median=1.41107e-07 P95=0.000162776 P99=0.0006524 max=0.0196996 ldlt_err=n=33632 median=1.61976e-06 P95=0.000130769 P99=0.000595886 max=0.0116935 cg_fail=32624 jacobi_fail=0 dcreg_fail=21
low_kappa n=15574 ex=n=15574 median=1.49049e-06 P95=8.48569e-05 P99=0.000366796 max=0.00531458 residual=n=15574 median=1.56444e-07 P95=0.000111896 P99=0.00048623 max=0.00793728 ldlt_err=n=15574 median=1.75988e-06 P95=9.65998e-05 P99=0.000426654 max=0.00638844 cg_fail=15359 jacobi_fail=0 dcreg_fail=6
middle_kappa n=16376 ex=n=16376 median=1.31654e-06 P95=0.000151136 P99=0.0006556 max=0.013334 residual=n=16376 median=1.27033e-07 P95=0.000194496 P99=0.000740396 max=0.00534769 ldlt_err=n=16376 median=1.5008e-06 P95=0.000165167 P99=0.000714458 max=0.0116935 cg_fail=15618 jacobi_fail=0 dcreg_fail=15
high_condition_P95 n=1682 ex=n=1682 median=1.11421e-06 P95=1.81959e-05 P99=0.000120123 max=0.00334195 residual=n=1682 median=2.30276e-07 P95=2.17043e-05 P99=0.000469397 max=0.00815688 ldlt_err=n=1682 median=1.32773e-06 P95=2.30217e-05 P99=0.000142686 max=0.00613274 cg_fail=1678 jacobi_fail=0 dcreg_fail=0
high_kappa_R_P95 n=1682 ex=n=1682 median=1.44347e-06 P95=0.000131926 P99=0.000866372 max=0.00288656 residual=n=1682 median=1.48468e-07 P95=0.00032813 P99=0.0010136 max=0.0196996 ldlt_err=n=1682 median=1.64143e-06 P95=0.00015668 P99=0.000915803 max=0.00349082 cg_fail=1647 jacobi_fail=0 dcreg_fail=0
high_kappa_t_P95 n=1682 ex=n=1682 median=2.16473e-06 P95=9.20775e-05 P99=0.000459377 max=0.00426197 residual=n=1682 median=1.72807e-07 P95=8.4917e-05 P99=0.000363534 max=0.00446275 ldlt_err=n=1682 median=2.52184e-06 P95=0.000113687 P99=0.000444054 max=0.00574294 cg_fail=1601 jacobi_fail=0 dcreg_fail=9
weak_rank_0 n=15574 ex=n=15574 median=1.49049e-06 P95=8.48569e-05 P99=0.000366796 max=0.00531458 residual=n=15574 median=1.56444e-07 P95=0.000111896 P99=0.00048623 max=0.00793728 ldlt_err=n=15574 median=1.75988e-06 P95=9.65998e-05 P99=0.000426654 max=0.00638844 cg_fail=15359 jacobi_fail=0 dcreg_fail=6
weak_rank_1 n=18044 ex=n=18044 median=1.33446e-06 P95=0.000148827 P99=0.000668288 max=0.013334 residual=n=18044 median=1.29516e-07 P95=0.000205747 P99=0.000754242 max=0.0196996 ldlt_err=n=18044 median=1.51328e-06 P95=0.000164236 P99=0.000730423 max=0.0116935 cg_fail=17253 jacobi_fail=0 dcreg_fail=15
weak_rank_2plus n=27 ex=n=27 median=6.75325e-07 P95=2.31754e-05 P99=3.2487e-05 max=3.2487e-05 residual=n=27 median=2.23828e-07 P95=1.11882e-05 P99=2.96574e-05 max=2.96574e-05 ldlt_err=n=27 median=9.22103e-07 P95=2.86686e-05 P99=4.28336e-05 max=4.28336e-05 cg_fail=19 jacobi_fail=0 dcreg_fail=0
