# [Super-LIVO v1][R11U] Day10 causal scheduling + partial-LIO cadence audit

**Status:** BLOCKED — `SCHEDULER_IMPLEMENTATION_BUG`; STOP FOR OWNER

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

Outcome:

- `CLOCK-COMPARABLE`; scan timing `ACQ-CONSISTENT`; all three source headers monotonic;
- causal availability PASS (`emit_without_lidar=0`, `emit_without_imu=0`);
- stable identity conservation PASS (`lost=0`, `duplicate=0`);
- frozen cut hard gate FAIL: `wrong_side_count=4,284,862` because the pending tail is appended wholesale without a new `tc` cut;
- geometry-cadence instrumentation and B0/C0 reruns were not started after the mandatory hard stop;
- evidence: `docs/super_livo/evidence/day10_causal_scheduling_partial_lio_audit_round11u.md`.
