# [Super-LIVO v1][R11U] Day10 causal scheduling + partial-LIO cadence audit

**Status:** ACTIVE（Round 11U R1 owner contract）

Input HEAD: `70a62a6`

Purpose:

- prove clock-domain and Mid70 scan-end provenance;
- audit causal camera-epoch availability and camera accounting;
- prove stable-identity LiDAR slice conservation and frozen cut rule;
- instrument clean Day10 B0/C0 geometry support without changing semantics;
- classify the Day10 B0→C0 regression and stop for Owner.

Frozen non-actions:

- no header-time reorder or timestamp/offset change;
- no scheduler, slicing, camera-epoch, or update-cadence change;
- no A0/A1, Night08, Oxford, M2/M3, SFS, or Corridor runs.

Architecture deviations: `NONE`
