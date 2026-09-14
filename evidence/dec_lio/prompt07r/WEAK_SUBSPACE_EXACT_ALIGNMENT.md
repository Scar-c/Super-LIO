# Weak subspace under exact finite→stride

| scene | arm | lambda_R | kappa_R | weak rank | O_P | Frobenius | rank1 angle | O_course |
|---|---|---|---|---|---|---|---|---|
| stairs | N | 1.1599e+06/3.25995e+07 | 14.5889/61.0856 | {'0': 1152, '1': 2287, '2': 7} | 1/1/1/1 | 0 | 0/1.47878e-06/2.09131e-06 deg | 0.885704/0.999084 |
| stairs | S | 2.62491e-13/1.06177e+07 | 4.42933/375.736 | {'0': 1872, '1': 1211, '2': 363} | 0.994149/1.40703e-06/0.99995/1 | 0.153255 | 3.82255/61.3628/89.932 deg | 0.274646/0.984084 |
| stairs | A* | 1.33381e+06/3.92943e+07 | 13.8825/54.6218 | {'0': 1167, '1': 2273, '2': 6} | 0.998586/0.119931/0.999958/1 | 0.0531978 | 2.15648/7.90179/69.7382 deg | 0.884729/0.999734 |
| tunnel2 | N | 1.09598e+06/2.3965e+06 | 59.4367/89.1614 | {'1': 2738, '2': 1} | 1/1/1/1 | 0 | 0/1.47878e-06/2.09131e-06 deg | 0.992275/0.998709 |
| tunnel2 | S | 1.07534e+06/2.44699e+06 | 61.8175/94.2574 | {'1': 2738, '2': 1} | 0.999932/0.998301/0.999995/1 | 0.0116658 | 0.471468/1.15468/2.36264 deg | 0.987553/0.998663 |
| tunnel2 | A* | 1.75487e+06/4.21086e+06 | 50.9017/75.5884 | {'1': 2739} | 0.999921/0.99658/0.999995/1 | 0.0125541 | 0.508495/1.46599/3.35238 deg | 0.988697/0.998903 |

The persisted DCReg diagnostics contain block eigenvalues and projectors, not
the complete first-iteration 6x6 H_L matrix. Therefore the full
`H_L/trace(H_L)` comparison is classified
`FULL_H_SHAPE_NORMALIZATION_NOT_AVAILABLE`; the stored Schur normalized-spectrum
diagnostics are retained. Projector comparisons use matched frames and the
subspace formula documented below.
