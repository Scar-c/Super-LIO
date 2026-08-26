# Super-LIVO Round 11Z — FAST-LIVO2 Mixed-Rate Camera Temporal Sampling Corrective

Executor: DS/OpenCode
ROS: ROS1 Noetic only
Repository: https://github.com/Scar-c/Super-LIO
Branch: super-livo

This round SUPERSEDES the current Round11Y Stage-B IMU-segmentation deep dive.

Do NOT continue the current Day10 IMU prior/deskew attribution before completing this mixed-rate camera gate.

Scientific priority:

FAST-LIVO2 reference mixed-rate adaptation
→ camera temporal sampling BEFORE S0 queue/sync
→ MCD stride=3
→ B0 / C0-stride1 / C0-stride3 architecture gate
→ STOP FOR OWNER

## 0. Why Round11Y Stage B is superseded

Round11X already established:

- Day10 B0 RMSE ≈ 1.2181 m
- Day10 C0 partial RMSE ≈ 3.1507 m
- C0/B0 ≈ 2.5866
- shadow_fullscan is bitwise equal to B0
- imu_fullscan/B0 RMSE ≈ 1.2378

Therefore camera plumbing itself can be zero influence; partial-LiDAR cadence is a major cause; camera-time IMU segmentation is secondary.

Pinned FAST-LIVO2 contains an early image-rate gate:

```cpp
// Hiliti2022 40Hz
if (hilti_en)
{
  static int frame_counter = 0;
  if (++frame_counter % 4 != 0) return;
}
```

The return occurs before downstream image buffering/synchronization.

MCD Day10 is approximately Camera 30 Hz / LiDAR 10 Hz, so the first reference-grounded test is camera temporal_stride=3, not deeper redesign of the unadapted 30 Hz scheduler.

Round11Y Stage-B attribution is DEFERRED, not discarded. If stride=3 remains AMBER/RED, Owner may reopen it later.

## 1. Project-State Consensus — hard gate

Before any new functional edit:

```bash
git status --short
git branch --show-current
git rev-parse HEAD
git fetch origin super-livo
git rev-parse origin/super-livo
git log --graph --decorate --oneline -25
git log --stat --oneline 4b8b9e1..HEAD
git merge-base --is-ancestor 4b8b9e1 HEAD
echo "ANCESTOR_4B8_RC=$?"
```

Required: `ANCESTOR_4B8_RC=0`.

Reconstruct all progress since 4b8b9e1 commit-by-commit.

Report:

```text
=== Project-State Consensus ===
Owner known frontier: 4b8b9e1
Local HEAD: ...
origin/super-livo: ...
Commits since 4b8b9e1: ...
Round11Y Stage-A artifact/provenance closure committed: YES/NO
Round11Y Stage-B instrumentation commits: ...
Current functional WIP: ...
Unknown/unattributed WIP: NONE / exact list
Consensus: ESTABLISHED / MISMATCH
```

Unknown/unattributed WIP, ancestry failure, or unexplained semantic divergence => STOP FOR OWNER.

Git commits are the canonical handoff transport.

## 2. Safe termination of current Round11Y Stage B

Do not launch another B0/imu_fullscan attribution run.

Preserve already completed Stage-A commits.

Inspect:

```bash
git status --short
git diff --stat
git diff
```

A known current WIP may exist in:

`src/super_lio/src/lio/super_lio.cpp`

where Layer-II debug prior capture was moved from stateProcess entry to after `Propagation_Undistort()` and before LiDAR geometry Observe.

If and only if the complete functional WIP is exactly:
- debug/audit-gated;
- captures the pre-LiDAR propagated prior at the correct location;
- default OFF;
- no estimator behavior change when audit is OFF;

then DS MAY test/build and commit it separately:

`fix(debug): capture pre-lidar prior after propagation`

Report:
- production estimator semantics: UNCHANGED when audit OFF
- Round11Y Stage-B: SUPERSEDED / DEFERRED

Any additional functional WIP => STOP FOR OWNER.

Update Round11Y tracker/evidence:

```text
TECHNICAL STAGE B:
SUPERSEDED BY FAST-LIVO2 MIXED-RATE CAMERA PROVENANCE

Existing Stage-B evidence:
PRESERVED / DEFERRED
```

Push valid Round11Y commits before Round11Z. Require tracked worktree clean and local/remote divergence 0/0.

## 3. Reference-Architecture Consensus — hard gate

Pinned FAST-LIVO2 reference:

`0d2c0346107b75b59934975adec9a6eeeb913c64`

Read-only.

Inspect actual pinned source and verify:
- `preprocess/hilti_en`
- `img_cbk`
- `if (++frame_counter % 4 != 0) return;`
- return occurs before image buffer insertion and sync measurement grouping.

Review FAST-LIVO2 GitHub Issue #283, scenario 30 Hz camera / 10 Hz LiDAR.

Source-review hierarchy:

issue body
→ maintainer/member replies
→ linked source
→ linked commit/PR
→ pinned implementation
→ Owner conclusion

If maintainer/member reply cannot be independently retrieved, DO NOT quote it as verified. Record `VERIFIED / NOT INDEPENDENTLY RETRIEVABLE`.

Pinned source itself is sufficient to authorize this tracer.

Report:

```text
=== Reference-Architecture Consensus ===
Subsystem: mixed-rate camera ingress
Architecture source: FAST-LIVO2
Pinned commit: 0d2c034...
Reference parameter: preprocess/hilti_en
Reference gate location: img_cbk before downstream buffer insertion
Reference production behavior: 40Hz HILTI -> accept every 4th image
Issue #283 30Hz/10Hz scenario: VERIFIED
Maintainer/member reply: VERIFIED / NOT INDEPENDENTLY RETRIEVABLE
Super-LIVO intended deviation: configurable integer stride, default=1
User-authorized innovation: NONE
Consensus: ESTABLISHED / MISMATCH
```

Mismatch => STOP.

## 4. Core architecture provenance doctrine

Persist Owner rule:

- FAST-LIVO2: LIVO synchronization / measurement grouping / LiDAR→visual sequential semantics / core visual lifecycle reference
- Super-LIO: LiDAR geometry/map computational substrate
- OpenVINS: FEJ / consistency semantic reference
- User-authorized deviations: explicit ADR-backed innovations only

For every FAST-LIVO2-derived core subsystem record BASELINE, REFERENCE, PRACTICAL GUIDANCE, and DEVIATION.

No agent may silently redesign a FAST-LIVO2 core semantic to solve a local engineering problem.

## 5. Owner-frozen camera temporal sampler

Production parameter:

`/camera/temporal_stride`

Type integer, default 1, valid >=1.

For Round11Z test only values 1 and 3. No sweep.

Sampling phase must mirror FAST-LIVO2 increment-before-modulo semantics:

```text
raw_camera_counter starts at 0
for every raw image at common ingress:
    ++raw_camera_counter
    accept iff raw_camera_counter % temporal_stride == 0
```

Thus stride=3 accepts frames 3,6,9,...

Do NOT choose "accept first then every Nth" in this round.

### Placement

Must be in common ROS1 online/offline camera ingress seam, preferably `ROSWrapper::HandleImage` or a helper called exactly once by both online and offline paths.

Do NOT implement only in OfflineReader, only in runner, or by preprocessing bag.

Sampler executes before:
- camera queue insertion
- S0 camera identity/terminal accounting
- camera epoch creation
- LiDAR slicing triggered by camera
- visual processing

If raw/compressed paths differ, factor one shared `shouldAcceptCameraFrame()`.

No camera timestamp or offset change.

## 6. Camera accounting

Add cheap counters:

```text
raw_camera_input
temporal_decimated
accepted_to_s0
```

Hard equation:

`raw_camera_input = temporal_decimated + accepted_to_s0`

Only accepted_to_s0 enters existing S0 terminal accounting.

Temporal-decimated frames are a distinct pre-S0 category `TEMPORAL_DECIMATED`; never count them as stale/capacity/EOF/camera epoch.

For canonical Day10 raw count 9736, stride=3 with frozen phase should accept `floor(9736/3)=3245` if all raw messages reach common ingress. If not, trace exactly why.

## 7. Runner contract

Canonical single-variant runner may expose an Owner-authorized `CAMERA_TEMPORAL_STRIDE` argument.

Required print/readback:

```text
/camera/enabled
/camera/temporal_stride
/lio/camera_epoch/enabled
/lio/camera_epoch/lidar_update_policy
/lio/v4/apply
/lio/v4/outlier_gate
```

Round11Z:

```text
B0:
camera=false
temporal_stride=1
visual apply=false

C0 stride1:
camera=true
temporal_stride=1
policy=partial
visual apply=false

C0 stride3:
camera=true
temporal_stride=3
policy=partial
visual apply=false
```

Missing/invalid runner value => fail before node start.

Do not expose CAM_OFFSET.

## 8. TDD required

Use `/tdd`.

- Z-T1 default stride=1 accepts all
- Z-T2 stride=3 accepts 3,6,9...
- Z-T3 zero/negative stride fails closed
- Z-T4 raw = decimated + accepted
- Z-T5 decimated image never enters S0 queue or triggers LiDAR slice
- Z-T6 accepted image preserves existing timestamp/offset semantics
- Z-T7 online/offline paths share the same sampler helper
- Z-T8 raw/compressed consistency: exactly one sampling decision per source frame
- Z-T9 stride1 event sequence regression equals accepted current C0 semantics
- Z-T10 B0 camera-disabled zero influence
- Z-T11 no vector<bool>/bit-packed parallel writes
- Z-T12 new ROSWrapper instance resets counter to zero; no function-static shared sampler state

## 9. Parallel-safety hard gate

Run:

```bash
git grep -n -E 'std::vector[[:space:]]*<[[:space:]]*bool|vector[[:space:]]*<[[:space:]]*bool' -- src
git grep -n -E 'parallel_for|parallel_reduce|tbb::|task_group' -- src
```

Require:
- concurrent vector<bool> = 0
- bit-packed proxy parallel writes = 0
- shared push_back = 0
- shared unordered mutation = 0
- parallel H/b reduction = 0

Do not reintroduce BIEVR-COIN/Super-LIVO race patterns.

## 10. Preserve corrected S0

Do NOT undo:
- pending-tail repeated re-slicing
- true LiDAR coverage-through-tc
- all-arrival ready-camera drain
- exact point/camera accounting
- raw scan lineage
- binary64 representation-collision classifier

Temporal sampling happens before S0.

## 11. Day10 experiment order

Canonical cached MCD Day10 only.

One bounded run per shell invocation.

Run:
- Z-B0
- Z-C0-S1
- Z-C0-S3

No A0/A1, shadow, imu_fullscan, Night08, Oxford, M3, eee/nya visual reruns.

Heavy Gate-M/HB/sanitizer/profiler OFF.

### B0

Hard canonical MD5:

`9931f96e2a2fe2f524982edc5fe19372`

If changed, HARD FAIL; do not create a new B0 baseline.

### C0 stride=1

Read the full accepted C0 stride1 trajectory MD5 and evaluator values from committed Round11X evidence.

Expected approximately:
- RMSE 3.1507 m
- C0/B0 2.5866
- geometry updates ~9701

If relevant dependencies unchanged, require exact trajectory MD5 parity with Round11X. Different hash => diagnose, do not silently rebaseline.

### C0 stride=3

Report:
- raw camera input
- temporal decimated
- accepted_to_s0
- accepted camera Hz
- raw LiDAR scans
- geometry updates
- updates/raw scan
- points/update P10/P50/P90/P99
- downsampled points
- effective correspondences/update
- map updates
- S0 lost/dup/wrong-side/overlap
- camera S0 terminal accounting
- trajectory MD5/rows/RMSE/mean/median/max/matched duration

## 12. Architecture gate

`R_s3 = RMSE(C0_stride3) / RMSE(B0)`

GREEN:
`R_s3 <= 1.10`
=> `FAST_LIVO2_MIXED_RATE_ADAPTATION_SUPPORTED`
STOP FOR OWNER. No automatic A0/A1.

AMBER:
`1.10 < R_s3 <= 1.50`
=> `MIXED_RATE_ADAPTATION_PARTIAL_CLOSURE`
STOP FOR OWNER. Do not automatically resume Round11Y Stage-B.

RED:
`R_s3 > 1.50`
=> `MIXED_RATE_ADAPTATION_INSUFFICIENT`
STOP FOR OWNER.

No stride 2/4/5 sweep. No adaptive selector. No ATE-based tuning.

## 13. Architecture documentation

Persist mixed-rate provenance in existing architecture/context docs or a dedicated file.

Must distinguish:
- inherited FAST-LIVO2 early camera temporal sampling intent
- Super-LIO geometry substrate
- corrected Super-LIVO S0 implementation correctness
- explicit User-authorized innovations

Record:
`sensor acquisition rate != estimator camera epoch rate`

Do not state camera and LiDAR must be equal frequency.

## 14. Source-review hard rule

Final evidence for reference-derived core changes must state:

```text
paper checked: YES/NO
pinned source checked: YES
relevant GitHub issue body checked: YES
maintainer/member replies checked: YES / NOT RETRIEVABLE
linked commit/PR checked: YES / NONE
official dataset config checked: YES/NO
Owner/User deviation: NONE / exact ADR
```

Never form an Owner conclusion from issue body alone.

## 15. Commit discipline — COIN-BIEVR style

One logical purpose per commit.

Expected when applicable:
- docs(super-livo): supersede Round11Y Stage-B with mixed-rate provenance
- docs(super-livo): register FAST-LIVO2 camera temporal sampling round
- test(super-livo): specify camera temporal sampling semantics
- feat(super-livo): add early camera temporal sampler
- tools(super-livo): expose canonical camera stride runner control
- docs(super-livo): record Day10 stride3 architecture gate

For every commit report:
- hash/message
- purpose
- files
- production semantics touched
- runner/audit semantics touched
- tests
- evidence
- reference provenance

Before commit inspect staged diff. Never `git add .` or `git add -A`.

After push:
- local/remote divergence 0/0
- tracked worktree clean

## 16. Prompt/tracker registration

Expected prompt #46:

`prompts/04_v1_implementation/46_round11z_fast_livo2_mixed_rate_camera_sampling.md`

Expected tracker:

`.scratch/super-livo-v1/issues/38-fast-livo2-mixed-rate-camera-sampling.md`

If occupied, STOP; do not auto-renumber.

Register this exact Owner prompt before Round11Z functional implementation.

## 17. User-delivery prompt cleanup

After canonical prompt copy is verified, exact downloaded/untracked Owner prompt copies may be removed by exact path only.

No git clean or wildcard deletion.

## 18. Skills

Required:
- `/tdd`
- `/diagnosing-bugs`
- `/grill-with-docs` for unresolved architecture/reference ambiguity only, then STOP

## 19. Spinner-safe execution

Mandatory:
- one bounded build/test/run per shell invocation
- `set -o pipefail`
- preserve real RC/PIPESTATUS
- explicit completion sentinel
- check pgrep/ps before rerun
- no duplicate bag process
- no broad pkill/killall
- preserve first failure logs

## 20. Final classifier

Exactly one:

```text
REFERENCE_ARCHITECTURE_MISMATCH
STRIDE1_REGRESSION
FAST_LIVO2_MIXED_RATE_ADAPTATION_SUPPORTED
MIXED_RATE_ADAPTATION_PARTIAL_CLOSURE
MIXED_RATE_ADAPTATION_INSUFFICIENT
IMPLEMENTATION_BLOCKED
```

Then STOP FOR OWNER.

## 21. Final report

Must include:
- Project-State Consensus
- Round11Y supersession commit transport
- Reference-Architecture Consensus
- camera sampler implementation
- Z-T1..Z-T12
- parallel safety audit
- B0
- C0 stride1
- C0 stride3 sampler/accounting/cadence/accuracy
- architecture provenance
- non-actions
- canonical commit transport with every commit hash/purpose/files/tests/evidence
- Ending HEAD / Remote HEAD / divergence 0/0 / clean worktree
- exact final classifier
- STOP FOR OWNER

Owner reminder:

Do not manufacture a research problem by omitting an upstream FAST-LIVO2 core adaptation.

Order:
reference semantic parity
→ Super-LIO backend parity
→ User-authorized innovations/ablations.

For MCD Day10 first test:
30Hz raw camera
→ deterministic stride=3 before S0
→ ~10Hz estimator camera epochs
→ corrected S0
→ C0 architecture gate.
