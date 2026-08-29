# Round 15 — Prompt84 — Pre-Governance Recovery + Camera-Epoch Sequential Reconstruction

```text
Round:
Prompt84 — Pre-Governance Recovery + Camera-Epoch Sequential Reconstruction

=== Historical Branch ===
branch:
super-livo (historical evidence; preserved verbatim)
historical HEAD:
e86ac63d85503945e64d83ea0444a007bb50df68
remote HEAD:
e86ac63d85503945e64d83ea0444a007bb50df68
history modified:
NO

=== Recovery Frontier Audit ===
candidate:
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
8321586 classification:
PRODUCTION_CLEAN_BUT_RUNNER_GOVERNANCE_PRESENT
  (estimator independent; older Round12 transaction runners present but
  never referenced by src/super_lio)
governance artifacts present:
older run_m3_transaction.sh / run_ntu_transaction.sh + round12 transaction
docs; NO semantic profiles / NO supervisor / NO D_VISUAL profiles
estimator governed by them:
NO
selected base:
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
reason:
cleanest pre-governance production frontier per the mechanical audit

=== Rebuild Branch ===
branch:
super-livo-rebuild
base:
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
initial HEAD:
d97ca4f4523aab1a486ce108307dced2bd3ecacd
final HEAD:
40c3d7f4d9f06c0d79e94d8411f437033e9ef42e
origin:
<pending>
ahead/behind:
<pending>
worktree:
<pending>

=== Historical Commit Audit ===
33c1b3d:
  subject: restore Visual sequential update on D-family (DV0)
  production files: super_lio.cpp (4 lines — V-4A/C gate)
  placement: stateProcess after LiDAR update (PARTIAL-era path)
  payload: n/a
  apply gate: g_lio_v4_apply
  reuse: REUSE_SEMANTICALLY (D-family reachability concept; rewritten)
7d9be50:
  subject: camera-epoch Visual placement + exact-once payload ownership
  production files: ROSWrapper.h/.cpp, offline node
  placement: camera-event bookkeeping
  payload: retain-through + exact-once release
  apply gate: —
  reuse: REUSE_SEMANTICALLY (payload lifetime concept; rewritten)
ce3d1a9:
  subject: camera-epoch Visual lifecycle in D-family IMU_ONLY path
  production files: super_lio.cpp (+53)
  placement: camera event (IMU_ONLY)
  payload: retained
  apply gate: g_lio_v4_apply
  reuse: REUSE_SEMANTICALLY (sequential Visual transaction pattern;
           rewritten cleanly on the 8321586 base)
None cherry-picked wholesale; none carried test/governance coupling.

=== Before Recovery Event Semantics (8321586 base) ===
camera arrival: accepted into camera_buffer_ (IMU_ONLY cadence)
IMU propagation: YES (statePropagateOnly -> PropagateTo(t_c) ->
                 CommitPropagationOnlyEpoch)
LiDAR update event: full raw scan at scan end (FULL_SCAN epoch)
Visual event: NONE at the camera event — the Visual residual runs in a
              later full-LiDAR Observe callback (legacy placement)
payload release: at the IMU_ONLY epoch account (before any Visual use)
later full-LiDAR Observe: runs the legacy Visual
Visual H/b location: full-LiDAR Observe callback (legacy)

=== After Recovery Event Semantics (84-3) ===
camera event: t_c (D-family IMU_FULLSCAN, g_lio_v4_apply)
IMU interval: consumed by statePropagateOnly (existing)
LiDAR measurement group: the event's LiDAR posterior = latest committed
  LiDAR posterior transported to t_c by the event's own IMU interval
x_L/P_L producer: ESKF state after statePropagateOnly (the converged LiDAR
  posterior at t_c)
frozen-prior producer: runCameraEventVisual freezes prior.x/prior.P
Visual consumer: UpdateObserveFromPrior(frozen prior) at the SAME event
post-Visual state: x_LC/P_LC (ESKF state = the sequential posterior; later
  propagation continues from it)
payload release: releaseCameraPayload() exactly once after the post-solve
  lifecycle
legacy full-LiDAR-callback Visual: gated OFF under the D camera-event mode

=== Sequential Invariants ===
LIO before VIO: YES (frozen LiDAR posterior precedes the Visual transaction)
same intended event: YES (camera event t_c; prior transported by the event's
  own IMU interval)
Visual prior identity: x_L/P_L frozen for the transaction (T3 seam PASS)
prior recaptured during iteration: NO (fixed prior; solver iterates the
  work state only)
exact-once Visual transaction: YES (T5 seam PASS; legacy callback gated)
later propagation from Visual posterior: YES (ESKF state is the posterior)

=== FAST-LIVO2 Issue #263 ===
retrieve reference role: bootstrap/safeguard usable reference for young
  points (<=5 obs or no mature reference)
updateReferencePatch role: mature reference maintenance after sufficient
  observations (>5 upstream condition)
>5 observation meaning: eligibility for mature reference refresh
bootstrap safeguard: separate site from mature refresh
mature refresh: separate site; not run twice to emulate bootstrap
double visual map build: NO
double presolve: NO
classification: TWO_LIFECYCLE_SITES

=== Visual Map Scope ===
legacy residual: LEGACY_VISUAL_MEASUREMENT / NOT_FAST_LIVO2_PARITY /
  TO_BE_REPLACED_IN_D2 (unchanged; not tuned)
map generation: unchanged
observation update: unchanged
reference bootstrap: structural — observations[] slots + valid flag can
  express "no mature reference yet"; no construction-time reference
  invariant (84-4 = N/A)
mature reference update: unchanged
full E lifecycle migrated: NO

=== Clean Architecture Audit ===
transaction supervisor: NOT MIGRATED (not present on the branch)
semantic profiles: NOT MIGRATED
generic validator: NOT MIGRATED
round-specific estimator modes: NOT MIGRATED
test-only production APIs: NOT MIGRATED
heavy instrumentation: NOT MIGRATED
duplicate scheduler ownership: NONE (one camera-event path + the legacy
  full-LiDAR path gated under it)

=== Tests ===
count: 3 seam tests (T2/T3/T5/T6 combined in one small binary)
build: PASS
LIO smoke: FAILED — 8321586 base memory defect (see diagnosis)
event-order seam: PASS
frozen-prior seam: PASS
payload lifetime: structural (retain/release in production; runtime
  counter camera_payload_release_count_)
exact-once: PASS (seam + legacy gate)
reference lifecycle contract: PASS (structural)
bag run: bounded attempts executed; camera-enabled and camera-disabled both
  hit the base memory expansion (see diagnosis)

=== Key Commits ===
Prompt/recovery audit: d97ca4f4523aab1a486ce108307dced2bd3ecacd
minimal test commit: 399bc36
KEY CAMERA-EPOCH PRODUCTION COMMIT: 40c3d7f4d9f06c0d79e94d8411f437033e9ef42e
reference lifecycle contract commit: N/A (contract documented; no
  production change required — slots+valid already express it)
evidence commit: <pending>
final report commit: <pending>
delivery metadata commit: <pending>

=== Changed Files ===
M	.scratch/super-livo-v1/issues/39-d-family-and-bag-pipeline.md
A	docs/super_livo/rebuild/prompt84_recovery_architecture.md
A	prompts/84_round15_pre_governance_recovery.md
M	prompts/README.md
M	src/super_lio/CMakeLists.txt
M	src/super_lio/include/lio/super_lio.h
M	src/super_lio/include/ros/ROSWrapper.h
A	src/super_lio/src/common/tests/test_prompt84_rebuild_seam.cpp
M	src/super_lio/src/lio/super_lio.cpp
M	src/super_lio/src/ros/ROSWrapper.cpp

=== Commits ===
40c3d7f fix(rebuild): restore camera-epoch sequential LiDAR-to-Visual update (84-3 KEY)
399bc36 test(rebuild): add minimal camera-event sequential seam checks (84-2)
d97ca4f docs(rebuild): register Prompt84 pre-governance recovery + frontier audit (84-1)

=== Architecture Deviations ===
NONE beyond the authorized reconstruction semantics

=== Final Classification ===
STOP_FOR_OWNER_RECOVERY_BASE_MEMORY_DEFECT

(reason: camera-disabled clean LIO smoke on the accepted recovery base
8321586 expands resident memory to ~20.8 GB (OOM at 24.7 GB in the
camera-enabled attempt) during a bounded 60 s window. The expansion is
independent of the Prompt84 production changes — with the camera disabled
the new camera-event code is entirely inert. 8321586 is a Round12-era
frontier that lacks the Round13 memory repairs. §34 STOP condition hit;
§4 forbids choosing another base automatically.)

=== Next Stage ===
D1 clean migration authorized:
NO
D2 authorized:
NO
LIVO2_COMPAT_BASELINE:
NO

Await Origin decision on the recovery base (later base candidate with
memory fixes, or authorized bounded memory repair on the rebuild branch).
```
