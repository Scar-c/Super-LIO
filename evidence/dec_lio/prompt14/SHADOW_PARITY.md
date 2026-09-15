# Prompt14 native/shadow parity

Each sequence was run once with the native path and once with the Prompt14
shadow enabled. The native runs are in the preserved Prompt14 runtime root;
the final schema/basis-corrected shadow runs are in `prompt14_v2`. Runtime bags,
logs, and trajectories remain outside Git.

| sequence | native trajectory SHA256 | shadow trajectory SHA256 | bytes | cmp |
|---|---|---|---:|---:|
| bridge01 | `6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203` | same | 622301 | 0 |
| stairs_alpha | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` | same | 557579 | 0 |
| tunnel1_gamma | `ab93cbaa4a1e823d2ab1c983a5bef478345cf4588eb47a2f4ac632e906378a6c` | same | 341978 | 0 |
| tunnel2_alpha | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | same | 449428 | 0 |
| tunnel2_gamma | `6eb07eed27c4e22b19ec83bbac63e8d797a02032e46b8cff4235051a46a388f6` | same | 426058 | 0 |

All ten processes returned zero. Fatal-marker scans were `NONE`. The v2
shadow CSVs contain 86 fields with schema version 1 and zero row repairs. This
is exact trajectory parity, not merely similar APE/RPE.
