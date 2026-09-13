# Tunnel2 forcing persistence

The baseline is Stairs full Prompt06 rows.  Percentiles below are computed on
finite values only.  Tunnel2 onset is the fixed interval
`[1706584541.828, 1706584579.030]`.

| metric | Stairs P95 | Tunnel2 onset median | P90 | P95 | P99 | max | above Stairs P95 | longest exact run / duration |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| weak chi_R | 6.90690 | 1.17298 | 4.73804 | 6.58879 | 19.2214 | 66.3550 | 14/299 (4.68%) | 3 frames / 0.199105 s |
| weak psi_R | 10.5304 | 2.97004 | 10.0277 | 12.6174 | 20.8104 | 33.4321 | 27/299 (9.03%) | 3 frames / 0.199132 s |
| Psi_weak_R | 13.4311 | 2.97004 | 10.0277 | 12.6174 | 20.8104 | 33.4321 | 12/299 (4.01%) | 2 frames / 0.0995493 s |
| A_weak_R | 0.00988534 | 0.00265440 | 0.00939354 | 0.0122142 | 0.0228572 | 0.0398339 | 25/299 (8.36%) | 3 frames / 0.199132 s |
| C_yaw_L | 0.478859 | 0.0199870 | 0.210582 | 0.324205 | 2.45208 | 5.72316 | 10/299 (3.34%) | 4 frames / 0.298608 s |
| C_yaw_F | 0.402042 | 0.0174987 | 0.174009 | 0.256951 | 0.864420 | 2.05087 | 9/299 (3.01%) | 4 frames / 0.298608 s |
| C_L | 17.3773 | 10.0120 | 31.0938 | 44.9552 | 86.8491 | 123.140 | 85/299 (28.43%) | 13 frames / 1.20102 s |
| G/N | 0.210469 | 0.0244087 | 0.256519 | 0.558687 | 2.00073 | 3.41025 | 39/299 (13.04%) | 4 frames / 0.298608 s |

`full/pre5/pre3/pre1/onset` distributions are in the machine-readable
analysis JSON.  The exact-run detector uses observed frame step 2 and rejects
timestamp gaps above 0.2 s; it never bridges a missing observation.  Thus the
Prompt05-style peaks are not persistent forcing events.  This supports an E
annotation, but the primary scientific classification remains physical-axis C.
