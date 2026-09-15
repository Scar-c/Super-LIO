# Corrected directional diagnostic examples

These are GT-blind strongest rotational-degeneracy examples selected by the
corrected DCReg eigenmode multiplier from the canonical L1 diagnostics. The
projection basis is the final ESKF innovation tangent.

| sequence | frame | i_weak | m_i | angle(u_transport,u_cov_weak) rad | q_weak | weak innovation/correction | strong innovation/correction |
|---|---:|---:|---:|---:|---:|---|---|
| Stairs | 1159 | 0 | 16.1699 | 0.00161229 | 0.978409 | 0.0155763 / 0.00769011 | 0.000328677 / 0.000308260 |
| Tunnel2 Alpha | 1725 | 0 | 13.3523 | 0.00273498 | 0.867154 | -0.0113920 / -0.00614024 | -0.000924479 / -0.000834483 |
| Tunnel2 Gamma | 1051 | 0 | 21.2097 | 0.000228387 | 1.51615 | 0.00115200 / 0.000426140 | 0.00106665 / 0.000953081 |

Selected-frame covariance eigenvalues `(weak/other/strong ordering)` were:

- Stairs: `(6.73347e-7, 1.00136e-6, 1.60335e-5)`;
- Alpha: `(9.88681e-7, 1.01536e-6, 1.25304e-5)`;
- Gamma: `(6.71654e-7, 1.00438e-6, 2.13165e-5)`.

The small transport-to-covariance angles in these strongest examples confirm
that the corrected transported DCReg weak mode corresponds to the actual
low-confidence direction seen by the ESKF. The occasional large angle tail in
the full sequence is retained as evidence rather than hidden.
