# Super-LIVO Round 11AA — D-Family Corrected Camera-Time Propagation + Conditional Bag Pipeline

**Executor:** DS/OpenCode  
**ROS:** ROS1 Noetic only  
**Branch:** `super-livo`

## Owner correction before execution

Do **not** describe current C0-S1 / C0-S3 as “buggy C”.

The Round11Y `PropagateTo(tc)` gap was demonstrated in the experimental
`imu_fullscan / IMU_ONLY` path. It was **not** demonstrated in the normal
`partial` C0 path.

Therefore this round defines a separate **D-family**:

```text
D = corrected camera-time IMU propagation
    + full raw LiDAR retained to scan end
    + one full LiDAR geometry Observe per raw scan
    + visual state OFF
```

Two Day10 groups:

```text
D-S1:
camera temporal_stride = 1
corrected imu_fullscan semantics

D-S3:
camera temporal_stride = 3
corrected imu_fullscan semantics
```

C remains the FAST-LIVO2-style partial-LiDAR family:
`C-S1 = partial + stride1`, `C-S3 = partial + stride3`.

Do not rename historical C evidence.

---

## 0. Project-State Consensus

Before functional work:

```bash
git status --short
git branch --show-current
git fetch origin super-livo
git rev-parse HEAD
git rev-parse origin/super-livo
git log --graph --decorate --oneline -25
```

Expected known frontier from Round11Z: `7fbb4d1`.

If remote has advanced beyond it, reconstruct every commit since `7fbb4d1`
before work.

Unknown/unattributed WIP or unexplained semantic divergence => STOP FOR OWNER.

Git commits are the canonical agent handoff transport.

---

## 1. Audit D semantics before running

Confirm from current code and committed Round11Y evidence:

1. `PropagateTo(tc)` fix exists in the IMU_ONLY/imu_fullscan path.
2. D camera epochs propagate IMU to accepted camera `tc`.
3. D camera epochs do **not** execute partial LiDAR Observe.
4. D camera epochs do **not** perform camera-driven map update.
5. Visual state apply is OFF.
6. Raw LiDAR scan stays intact to scan end.
7. Every eligible raw LiDAR point is used in exactly one geometry update.
8. Full-scan deskew uses the corrected camera-time propagation history.
9. C-S1/C-S3 partial path does not execute the Round11Y IMU_ONLY gap path.

If item 9 is false, STOP FOR OWNER because that would change the interpretation
of Round11Z.

---

## 2. Parallel safety

Mandatory audit:

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

---

## 3. No new D algorithm unless necessary

Prefer the already-committed corrected `imu_fullscan` policy.

Only add wiring needed to combine:

```text
imu_fullscan + camera temporal_stride
```

Do not redesign propagation, deskew, scheduler, estimator, or map lifecycle.

Any broader architecture change => STOP FOR OWNER.

---

## 4. Day10 matrix

Run exactly:

```text
D10-B0
D10-C-S1
D10-C-S3
D10-D-S1
D10-D-S3
```

Visual state OFF for all C/D variants. No A0/A1.

### Existing anchors

B0:

```text
MD5 = 9931f96e2a2fe2f524982edc5fe19372
RMSE ≈ 1.2181 m
```

C-S1:
retrieve the complete committed Round11X/11Z hash; expected RMSE ≈ 3.1507 m.

C-S3:
retrieve the complete committed Round11Z hash; expected RMSE ≈ 1.2862 m.

If dependencies are unchanged, historical hashes must reproduce exactly.
Do not silently rebaseline.

### D-S1

Must reproduce the **post-PropagateTo-fix** corrected imu_fullscan evidence from
Round11Y, not the earlier pre-fix run.

Retrieve its exact post-fix trajectory MD5 from committed evidence.

Expected RMSE approximately 1.0882 m, but do not force it.

### D-S3

New combination:

```text
temporal_stride=3 + corrected imu_fullscan
```

This is the primary new Day10 result.

---

## 5. Required D accounting

For D-S1/D-S3 report:

- raw camera input
- temporal decimated
- accepted_to_s0
- camera terminal accounting
- raw LiDAR scans
- full geometry updates
- geometry updates/raw scan
- raw LiDAR input points
- pre-Observe excluded
- eligible points
- used exactly once
- duplicate use
- never used
- IMU-only camera propagation segments
- total Predict/PropagateTo boundaries
- points/update P10/P50/P90/P99
- downsampled points/update
- effective correspondences/update
- map updates
- trajectory rows/MD5
- RMSE/mean/median/max

Hard:

```text
duplicate use = 0
never used = 0
```

under the existing accepted pre-Observe exclusion convention.

For Day10 D-S3 with raw camera count 9736 and frozen sampler phase, expected
accepted camera count is 3245 if all raw frames reach the shared ingress seam.

---

## 6. Day10 D gate

Compute:

```text
R_D1 = RMSE(D-S1)/RMSE(B0)
R_D3 = RMSE(D-S3)/RMSE(B0)
```

### GREEN

Require:

```text
R_D1 <= 1.10
AND
R_D3 <= 1.10
```

plus no NaN/covariance/accounting/ownership failure and unchanged B0/C anchors.

Classification:

```text
D_FAMILY_DAY10_GREEN
```

If GREEN, automatically continue to the bag pipeline below. This continuation
is explicitly Owner-authorized.

### AMBER

Either ratio in `(1.10, 1.50]`:

```text
D_FAMILY_DAY10_AMBER
```

STOP FOR OWNER.

### RED

Either ratio `>1.50`:

```text
D_FAMILY_DAY10_RED
```

STOP FOR OWNER.

---

## 7. Architecture limitation

D remains visual-state-OFF.

Do **not** claim D is already the final visual Super-LIVO architecture. Future
visual-on LiDAR→visual sequential semantics still require explicit Owner review.

This round establishes the inertial/full-scan backbone only.

---

## 8. Conditional bag pipeline

Enter only after `D_FAMILY_DAY10_GREEN`.

Use D as the state-off backbone.

For mixed-rate MCD where camera≈30Hz and LiDAR≈10Hz, D-S3 is authorized.

For datasets that do not need decimation, default temporal_stride=1; do not
force stride3 globally and do not tune stride from ATE.

---

## 9. Dataset order — Owner frozen

Run in this exact order:

```text
1. MCD second designated sequence
2. NTU
3. Oxford
4. M3DGR
```

### 9.1 MCD second designated sequence

Resolve from the committed dataset registry.

Expected project mapping if unchanged:

```text
ntu_night_08 / MCD SeqID4
```

Do not trust this prompt alone; verify against the committed dataset/config
registry. If it resolves differently, STOP and report instead of substituting.

Run:

```text
B0
D-state-off
```

If this MCD sequence has the same ~30Hz camera / ~10Hz LiDAR sensor-rate
provenance, stride3 is authorized without sweep.

### 9.2 NTU

Then return to:

```text
eee_01
nya_01
```

Default temporal_stride=1 unless committed sensor-rate provenance says otherwise.

Run state-off:

```text
B0
D0
```

Preserve historical eee B0 anchor:

```text
9af9b9d9b7fdeda4ffcd031b9f0cb544
```

unless a proven production dependency legitimately changes B0 semantics.

If eee B0 differs unexpectedly, STOP; do not rebaseline.

Also report:

```text
online/offline direct trajectory MD5 parity:
VERIFIED / NOT VERIFIED
```

Do not infer it from recollection.

### 9.3 Oxford

Then the existing canonical Oxford target, expected if registry unchanged:

```text
Oxford Quarter01
```

Verify committed calibration, bag, GT, and evaluator before execution.
No invented offsets/calibration.

Run B0 + D0.

Missing prerequisite => STOP at Oxford; do not substitute sequence.

### 9.4 M3DGR

M3 is last.

Read committed registry and resolve exact designated sequence(s). Preserve the
already-defined dataset-specific evaluator class.

If no single canonical M3 target is currently frozen, STOP FOR OWNER before
running M3 rather than choosing one ad hoc.

---

## 10. Per-dataset continuation gate

For every dataset report:

- B0 health
- D-state-off health
- completion/rows
- NaN/cov failures
- camera accounting
- LiDAR ownership
- geometry cadence
- canonical evaluator result
- D/B0 ratio where meaningful

Continue if:
- run completes
- no correctness/accounting/ownership anomaly
- no architecture regression >1.50x B0

Where comparable full-trajectory RMSE exists:

```text
D/B0 <=1.10        GREEN
1.10<D/B0<=1.50   AMBER
D/B0>1.50          STOP FOR OWNER
```

For evaluator classes where this ratio is not meaningful, use the already
committed dataset-specific gate; do not invent a universal ATE gate.

---

## 11. No visual experiments

Do NOT run:
- A0/A1
- FEJ
- exposure
- reference update
- visual weight/threshold sweep

After the state-off pipeline, Owner decides when to restore visual experiments.

---

## 12. Semantic tool provenance

Every semantic runner/audit/evaluator/generator must be tracked.

Per dataset persist:
- runner path + commit
- config path + hash
- bag identity/hash
- GT source/generator/hash
- evaluator path + commit
- audit path + commit
- exact command
- trajectory MD5

No critical semantic dependency may live only in `/tmp`.

---

## 13. COIN-BIEVR-style commit discipline

One logical purpose per commit.

For every commit report:
- hash/message
- purpose
- files
- production semantics
- runner/audit semantics
- tests
- evidence

At each handoff:
- Starting HEAD
- Ending HEAD
- Remote HEAD
- local/remote divergence 0/0
- tracked worktree CLEAN
- exact remaining untracked list

Never `git add .`, `git add -A`, or `git clean -fd`.

---

## 14. Project-state consensus at dataset transitions

Before each next dataset:
- verify current HEAD/remote HEAD
- verify clean worktree
- inspect last completed dataset commit
- verify runner/evaluator commit

If another agent/user advanced the branch, reconstruct progress from commits
before continuing.

---

## 15. Prompt registration

Expected:

```text
#47
prompts/04_v1_implementation/47_round11aa_d_family_and_bag_pipeline.md
```

Tracker:

```text
.scratch/super-livo-v1/issues/39-d-family-and-bag-pipeline.md
```

If occupied, STOP; do not auto-renumber.

---

## 16. Skills

Required:
- `/tdd`
- `/diagnosing-bugs`
- `/grill-with-docs` for unresolved reference/spec ambiguity only, then STOP

---

## 17. Spinner-safe execution

- one bounded build/test/run per shell invocation
- `set -o pipefail`
- preserve real RC/PIPESTATUS
- explicit completion sentinel
- check pgrep/ps before rerun
- no duplicate bag run
- no broad pkill/killall
- preserve first failure logs

---

## 18. Final classification

Exactly one:

```text
D_FAMILY_DAY10_GREEN_PIPELINE_COMPLETE
D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_MCD2
D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_NTU
D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_OXFORD
D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_M3
D_FAMILY_DAY10_AMBER
D_FAMILY_DAY10_RED
IMPLEMENTATION_BLOCKED
PROJECT_STATE_MISMATCH
```

Then STOP FOR OWNER.
