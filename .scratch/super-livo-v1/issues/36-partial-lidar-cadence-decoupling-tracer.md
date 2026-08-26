# [Super-LIVO v1][R11X] Partial-LiDAR cadence decoupling tracer

**Status:** ACTIVE — Round 11X Owner contract

Input HEAD: `ab33fa2`

Purpose:

- independently re-audit production parallel safety and scheduler semantics;
- add state-off `partial`, `shadow_fullscan`, and `imu_fullscan` cadence policies;
- prove full-scan point ownership and shadow zero influence;
- run only canonical MCD Day10 B0/C0 cadence attribution;
- answer whether camera-driven partial-LiDAR geometry cadence is the primary C0 accuracy loss.

Pre-existing unknown WIP disposition:

- file: `scripts/super_livo/experiments/run_offline_variant.sh`;
- content: `CAM_OFFSET` CLI plus `/camera/time_offset` set/readback;
- Owner decision: not accepted into current frontier;
- preserved patch: `/tmp/round11x_unknown_cam_offset_wip.patch`;
- SHA-256: `aee116954970a9536421d1c69f65db1aebd59fff1b985f7665b3d111996b671e`;
- restored exactly to `ab33fa2`; no other WIP touched.

Frozen non-actions:

- no offset/timestamp representation change;
- no visual state feedback, A0/A1, FEJ, exposure, or sigma/threshold tuning;
- no Night08, Oxford, M3, SFS, Corridor, eee, or nya runs;
- refs remain read-only.

Architecture deviations: `NONE`
