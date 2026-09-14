# PSD and monotonicity

Synthetic T13/T14 passed. Final shadow CSVs contain no NaN in the audited
mathematical fields; every row has `attenuation_valid=1` and
`fail_open_reason=NONE`.

The minimum recorded eigenvalues of H-tilde were positive. The minimum
recorded eigenvalues of H-H-tilde were only small floating residuals:

| scene | min eig(H-tilde) | min eig(H-H-tilde) |
|---|---:|---:|
| Bridge01 | 7.6235e3 | -3.5067e-7 |
| Stairs | 2.1267e3 | -1.9553e-7 |
| Tunnel2 | 3.5180e4 | -1.0386e-7 |

The negative residuals are far below the implementation's relative PSD
tolerance; no information-increase row was accepted. Strong-mode maximum
reduction stayed below 5.83e-7, so there was no systematic attenuation of
strong complementary directions.
