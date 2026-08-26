# [Super-LIVO v1][R11T] Canonical benchmark B0/C0 architecture gate

**Status:** BLOCKED — D10-C0-B mandatory Owner stop

Round 11S:

- EXECUTED
- Initial HEAD 85ef93f
- Final HEAD 4d35d4a
- MCD Day10 canonical preprocessing PASS
- B0 raw/canonical bitwise parity PASS
- canonical LZ4 speedup 2.6x
- Day10 B0/C0/A0/A1 completed
- Day10 B0->C0 severe regression discovered

Round 11T:

- ACTIVE
- Day10 targeted C0 architecture/timing audit
- Night08 staged B0/C0 -> conditional A0/A1
- Oxford staged provenance+B0/C0 -> conditional A0/A1

Stop evidence:

- Day10 canonical record-order adjacent header inversions: 12,651
- inversion P50/P90/P99/max: 33.854/99.626/101.614/103.450 ms
- IMU/LiDAR/Camera record-header P50: 0.113/101.848/32.148 ms
- decision: MULTIBAG TEMPORAL ORDERING SUSPECTED
- input was not reordered; estimator semantics were not changed
- Night08 and Oxford were not started
