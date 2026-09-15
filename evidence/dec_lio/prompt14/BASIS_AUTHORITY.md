# Prompt14 basis authority

The audited six-dimensional local basis is fixed as

`[R0, R1, R2, p0, p1, p2]`

where `R0..R2` are local/body/right rotation coordinates and `p0..p2` are
world-frame translation coordinates. The captured native residual Jacobian
uses the native point-to-plane form

`J = [-(R * p_body)^ x, n_world]`

and the shadow accumulates the same weighted point-to-plane residuals. The
first three columns therefore remain rotational and the final three remain
world translation; no left/world rotational or body translation reinterpretation
is introduced.

DCReg is evaluated in this same raw six-dimensional basis. Raw eigenvalues are
classified with `kappa=10` and `rho=lambda_min/lambda_max < 0.1`; weak and
strong projectors are formed from the corresponding raw eigenspaces. The
summary retains the per-frame rank distribution and condition metrics rather
than silently replacing the native basis with a normalized coordinate system.

The v2 closure audit reports `basis_contract_fraction=1.0` and
`matched_valid_frames=frames_processed` for every sequence:

| sequence | frames | basis-contract fraction | matched-valid frames |
|---|---:|---:|---:|
| bridge01 | 3814 | 1.0 | 3814 |
| stairs_alpha | 3446 | 1.0 | 3446 |
| tunnel1_gamma | 2078 | 1.0 | 2078 |
| tunnel2_alpha | 2739 | 1.0 | 2739 |
| tunnel2_gamma | 2605 | 1.0 | 2605 |

There are no NaN/Inf basis-contract failures in the v2 CSVs.
