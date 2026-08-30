# Prob-LIO Prompt 2 — P0 Eval/Parity Closure → P1 Current Point Probability

## 0. Mission

Work in the Prob-LIO project and complete **two strictly ordered scopes**:

1. **P0 corrective closure**: finish the evaluator/parity/SPEC bookkeeping that Owner requested.
2. **P1 Current Point Probability**: only after all P0 corrective gates pass, implement FAST-LIVO2-parity LiDAR point covariance for the current downsampled scan, without allowing it to affect the estimator yet.

Do **not** start P2 map covariance, P3 QR uncertainty, P4 dynamic P2P weighting, or P5 probabilistic association.

Expected workspace:

```text
~/super_livo/
├── bag/NTU/eee_01
├── ref/FAST-LIVO2
└── src/Super-LIO
```

Expected branch: `prob-lio`.

Expected starting frontier is the P0 documented HEAD after the prior follow-up, expected:

```text
bb8596f
```

Verify the exact full SHA yourself before doing anything. If the branch/HEAD/worktree materially differs, STOP FOR OWNER. Do not reset/rebase/merge/discard work.

Normal path/config/filename discovery is not a failure: inspect the actual workspace and adapt. Do not turn harmless path fixes into false non-closure.

---

# 1. State consensus

Before edits, record:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git log --oneline -5
git rev-parse origin/prob-lio
```

Requirements:

- branch = `prob-lio`
- worktree clean
- local HEAD consistent with `origin/prob-lio`
- preserve all existing P0 commits
- no merge/rebase/reset/force-push

Register this exact prompt under:

```text
prompts/prob_lio/prompt2_P0_eval_closure_P1_point_probability.md
```

and update the prompt index.

---

# 2. Project architecture — authoritative stage model

The authoritative Prob-LIO roadmap is:

```text
P0  Baseline Freeze
 ↓
P1  Current Point Probability
 ↓
P2  Probabilistic Map Plumbing
 ↓
P3  Super-native QR Plane Uncertainty
 ↓
P4  Probabilistic P2P Weighting
 ↓
P5  Probabilistic Association (optional / second stage)
```

The authoritative Prob-LIO seam IDs must remain:

| Seam | Meaning |
|---|---|
| S0 | Downsample |
| S1 | LiDAR point covariance |
| S2 | current-point association covariance |
| S3 | map point covariance |
| S4 | initial map covariance |
| S5 | map covariance storage |
| S6 | compact-map aggregation |
| S7 | HKNN |
| S8 | Super QR plane estimator |
| S9 | QR plane covariance |
| S10 | correspondence gate |
| S11 | P2P measurement covariance / weighting |
| S12 | current pose covariance in final measurement `R_i` |
| S13 | IESKF information update |

If the current `SPEC.md` reused S0–S13 for general baseline production stages, **repair that vocabulary first**:

- restore S0–S13 to the table above;
- rename any generic baseline pipeline table to another namespace such as `B0...Bn` or “Baseline Production Invariants”;
- do not lose the useful P0 source-audit information.

This is a SPEC correction only; do not modify estimator semantics.

---

# 3. Explicit Owner policy: aggressive cleanup is intentional

The existing baseline runner intentionally cleans stale same-name ROS/Super-LIO/rosbag processes before an experiment to prevent contamination.

**Do not remove or weaken this behavior.**

This project intentionally accepts an aggressive cleanup policy for this dedicated experimental machine. Record that policy in `spec/prob_lio/SPEC.md` so future agents do not repeatedly classify it as a defect.

You may improve diagnostics or make patterns/options configurable, but the requested semantic is:

> before a canonical experiment, stale conflicting estimator / rosbag / rosmaster processes may be proactively terminated to guarantee a clean experiment.

This is Owner-authorized and is **not** a P0 blocker.

---

# PART A — P0 CORRECTIVE CLOSURE

# 4. Recover and audit the old canonical NTU evaluator

The older `super-livo` branch contains the established NTU evaluation contract. Inspect it directly from git; do not blindly trust paths in this prompt.

Expected useful artifacts include:

```text
scripts/super_livo/evaluation/eval_ntu_viral_official.py
scripts/super_livo/evaluation/pose_bag_to_tum.py
docs/super_livo/evidence/round12_pristine_super_lio_ntu_reproduction.md
docs/super_livo/offline/offline_runner_design.md
docs/super_livo/recovery/round4_tb0_recovery_status.md
scripts/super_livo/experiments/run_ntu_transaction.sh
```

Use commands such as:

```bash
git show super-livo:<path>
git ls-tree -r --name-only super-livo | grep ...
```

If filenames moved, locate the equivalent authoritative artifacts.

## 4.1 What must be independently confirmed

Record in SPEC, with branch/path/function/line evidence:

1. exact NTU VIRAL official-compatible ATE semantics;
2. Leica GT topic used for `eee_01`;
3. prism/body lever-arm handling;
4. timestamp matching/interpolation contract;
5. alignment contract (SE(3), no scale if that is what the old canonical evaluator does);
6. old pristine Super-LIO `60b57aa` reference:
   - expected estimate rows;
   - expected matched rows;
   - expected ATE;
7. old online/offline parity contract:
   - byte/hash equality used for strict trajectory parity;
   - quaternion-angle diagnostics can show ~1e-8 rad even when raw trajectory files are byte-identical due to parse/normalize/dot/acos floating-point effects.

Expected historical reference from the old project is approximately:

```text
estimate rows = 3981
matched rows  = 3329
ATE           = 0.118875639 m
```

Do not simply copy these numbers: confirm them from the old evidence.

---

# 5. Port/reuse the evaluator as a Prob-LIO project asset

Do not depend forever on switching to another branch.

Create reusable current-branch evaluation assets under:

```text
eval/prob_lio/
```

Prefer to reuse the old canonical scripts with provenance preserved, e.g.:

```text
eval/prob_lio/eval_ntu_viral_official.py
eval/prob_lio/pose_bag_to_tum.py
eval/prob_lio/README.md
```

Requirements:

- preserve or document upstream/evaluator provenance;
- inputs must be CLI-configurable;
- no hard-coded absolute workspace path;
- bag path, topic, trajectory path, GT path, and output path must be configurable;
- do not replace the canonical NTU metric with generic `evo_ape`;
- do not silently tune evaluator settings;
- do not create a second incompatible evaluator.

If the old evaluator already records hashes/provenance into YAML, preserve that behavior.

---

# 6. Freeze the current Prob-LIO P0 accuracy baseline before P1 edits

Use the **current P0 code before P1 production edits**.

Dataset:

```text
~/super_livo/bag/NTU/eee_01
```

Resolve the actual bag file yourself.

Use the existing P0 offline runner to produce or reuse a canonical full `eee_01` trajectory at the current pre-P1 HEAD.

Extract Leica GT using the recovered canonical tool/contract, expected topic:

```text
/leica/pose/relative
```

Then run the canonical NTU official-compatible evaluator.

Persist a small baseline record under `results/prob_lio/`, containing at least:

- git HEAD;
- bag path + bag hash if practical;
- config/launch/effective params;
- trajectory path + SHA256 + MD5;
- estimate row count;
- matched row count;
- official-compatible ATE;
- evaluator script SHA256;
- evaluator provenance;
- GT path/hash;
- command lines and real return codes.

Do not commit the bag.

## Gate P0-EVAL

The new current baseline must be consistent with the historical pristine `60b57aa` reference.

If the same estimator/config/evaluation contract produces a material discrepancy, **STOP FOR OWNER before P1** and classify the discrepancy.

For an exact same baseline path, use this default acceptance unless source evidence justifies a different tolerance:

```text
rows      = historical reference exactly
matched   = historical reference exactly
|ATE_new - ATE_reference| <= 1e-6 m
```

Do not tune parameters to force a pass.

---

# 7. Establish the two-layer trajectory parity contract

The old project already dealt with the “rotation diagnostic is ~1e-8 rad while trajectory is still bitwise identical” issue.

Implement/reuse a clear two-layer parity tool or extend the existing current comparator so reports distinguish:

## PARITY-BYTE

Check the actual trajectory serialization:

```bash
cmp -s A.tum B.tum
md5sum A.tum B.tum
sha256sum A.tum B.tum
```

If hashes are identical, classify:

```text
BYTE_PARITY = PASS
```

This is the strict bitwise trajectory test.

## PARITY-NUMERIC

Independently compare:

- row count;
- timestamp sequence;
- max/RMS translation difference;
- quaternion orientation difference after the established numerical comparison procedure.

A nonzero orientation diagnostic on the order of the old repeatability envelope (e.g. ~1e-8 rad) does **not** negate byte parity if file hashes are equal.

Persist both results.

If prior online/offline trajectory artifacts are available, evaluate them. If the exact online file is not preserved, perform one bounded same-window online/offline comparison (30 s is sufficient for this parity seam) using the same serialization contract.

The final report must never use the ambiguous single word `IDENTICAL` without saying whether it means byte parity or numerical parity.

---

# 8. P0 bookkeeping fixes

Before P1 begins:

1. restore authoritative Prob-LIO seam IDs S0–S13;
2. record the aggressive cleanup policy as Owner-authorized;
3. repair P0 history so it distinguishes:
   - starting HEAD `60b57aa...`;
   - P0 main implementation/closure commit;
   - P0 documentation follow-up HEAD;
4. remove any inaccurate “single commit” claim if P0 actually used two commits;
5. use “byte-identical” only when hashes prove it;
6. add the frozen official `eee_01` baseline to SPEC;
7. if the new offline executable added a `rosbag` build/runtime dependency but `package.xml` does not declare it, add the correct manifest dependency now.

Do not modify estimator/map/association production semantics in this closure.

Commit the P0 corrective closure separately, e.g.:

```text
chore(prob-lio): close P0 evaluator and parity contracts
```

Update SPEC with the actual commit SHA after commit; a small documentation follow-up commit is acceptable if needed. Do not rewrite history.

---

# 9. HARD GATE A — authorization boundary before P1

P1 may start in this same agent session **only if all of these are true**:

- P0 evaluator provenance confirmed from old branch;
- current-branch canonical evaluator is reusable;
- pre-P1 `eee_01` baseline ATE is frozen and consistent;
- byte/numeric parity semantics are explicitly separated;
- authoritative S0–S13 seam vocabulary is repaired;
- P0 production estimator semantics remain unchanged;
- P0 closure changes are committed;
- worktree clean before P1 begins.

If any of these fail, STOP FOR OWNER and do not implement P1.

---

# PART B — P1 CURRENT POINT PROBABILITY

# 10. P1 exact scope

P1 closes **S1 only**:

> Add FAST-LIVO2-parity LiDAR measurement covariance for each current downsampled Super-LIO scan point and preserve one-to-one identity with the production point array.

P1 must **not**:

- add map point covariance (S3/P2);
- modify `OctVox`, `AddPoint`, `insert`, or `getTopK` covariance storage;
- modify QR plane solve (S8);
- implement QR plane covariance (S9);
- modify `compute_error()` (S10);
- replace fixed `1000` (S11/P4);
- add current pose covariance to final measurement `R_i` (S12);
- modify ESKF math (S13);
- implement probabilistic association (S2/S10).

Trajectory/output must remain baseline-equivalent because P1 covariance is computed/plumbed but not consumed by the estimator update.

---

# 11. Re-audit the actual point/frame path before implementation

Do not assume frame names from the prompt.

Trace both repositories.

## FAST-LIVO2 reference

Find the active production path that:

1. downsamples/selects the point used for covariance;
2. calls `calcBodyCov()`;
3. identifies the coordinate frame of the point passed into `calcBodyCov()`;
4. defines `dept_err` / `beam_err` units and parameter loading;
5. transforms that covariance into later frames.

## Super-LIO production

Trace:

```text
raw LiDAR
→ undistortion
→ current custom VoxelGridClosest downsample
→ points_body_v3_
→ Observe()
```

Confirm:

- `DownSample()` uses the in-repo `VoxelGridClosest`, not PCL centroid averaging;
- the selected output is an actual selected input point rather than a centroid;
- the exact frame of the selected point after undistortion/downsample;
- the exact LiDAR↔IMU/extrinsic and deskew rotation semantics relevant to covariance orientation.

### Critical frame invariant

Do **not** feed a point expressed in the wrong frame directly into FAST-LIVO2's spherical/range-beam covariance formula merely because the vector has xyz coordinates.

The sensor noise model must be evaluated in the frame/parameterization that matches FAST-LIVO2's active `calcBodyCov()` semantics, then rotated through the actual Super-LIO production transform as necessary.

If Super-LIO no longer retains enough information to reproduce the required sensor-frame direction after downsample, solve this with the smallest semantics-preserving seam (e.g. retain the selected point's required frame identity/transform metadata), not by inventing a different noise model.

If a material architectural choice is unavoidable, STOP FOR OWNER with evidence before coding it.

---

# 12. P1 implementation contract

Implement a reusable covariance helper with FAST-LIVO2 active semantics.

Suggested logical components (adapt naming to repository style):

```text
calcBodyCov / CalcLidarPointCov
body_cov_list_
```

Requirements:

1. covariance is `3x3`;
2. use the same range/depth and beam-angle model as the local FAST-LIVO2 reference;
3. parameter values are config-driven, not buried as magic numbers in production code;
4. defaults must match the verified FAST-LIVO2 active/default semantics unless Super-LIO config explicitly overrides them;
5. document units clearly;
6. no pose covariance in P1 current-point sensor covariance;
7. covariance list must be cleared/resized each scan correctly;
8. index `i` must refer to the exact same physical downsampled point as `points_body_v3_[i]`;
9. no stale covariance from prior scans;
10. no per-point production logging by default.

Prefer a project-level switch such as a Prob-LIO enable/plumbing flag if it fits the existing configuration architecture. If added:

- default behavior must preserve baseline;
- P1-ON may compute covariance;
- P1-ON still must not alter estimator output;
- avoid proliferating redundant toggles.

Do not hard-code dataset-specific paths or NTU-specific algorithm parameters into production code.

---

# 13. P1 tests — TDD and semantic gates

Use real tests, not grep-only proxies.

Place reusable tests under:

```text
tests/prob_lio/
```

Integrate them with the project's build/test workflow when reasonable.

## G-P1.1 — FAST-LIVO2 formula parity

**Invariant:** for representative nondegenerate LiDAR points and the same `dept_err`/`beam_err`, the Super-LIO P1 covariance helper numerically matches local FAST-LIVO2 `calcBodyCov()` semantics.

Test representative points across:

- near / medium / far range;
- different beam directions;
- axis-aligned and oblique points.

Acceptance:
- tight double-precision numerical tolerance justified by implementation.

Negative mutation:
- deliberately alter `dept_err`, `beam_err`, or one covariance term in the test fixture/reference path and demonstrate the parity test fails.

Forbidden substitute:
- “matrix looks reasonable.”

## G-P1.2 — covariance validity

**Invariant:** valid production points produce finite, symmetric PSD covariance.

Evidence:
- symmetry norm;
- finite entries;
- eigenvalues within a small numerical tolerance of nonnegative.

Negative mutation:
- an intentionally non-symmetric or indefinite fixture must be rejected by the validation test/helper.

## G-P1.3 — frame/rotation consistency

**Invariant:** when the sensor covariance is rotated by the same rigid rotation used by the production point transform,

\[
\Sigma' = R\Sigma R^\top
\]

and the result matches an independently transformed test case.

This gate must exercise the actual helper/seam used by production, not an unrelated toy implementation.

Negative mutation:
- use the wrong rotation/inverse rotation and ensure the test fails.

## G-P1.4 — point/covariance identity

**Invariant:** after each production downsample:

```text
body_cov_list_.size() == points_body_v3_.size()
```

and covariance entry `i` belongs to point `i`.

Evidence must include at least:
- empty/small scan behavior;
- multiple successive scans with different point counts;
- no stale tail entries.

Negative mutation:
- off-by-one/reordered covariance fixture must fail.

## G-P1.5 — no estimator influence

**Invariant:** P1 covariance is not consumed by QR, correspondence gate, fixed weighting, map storage, or ESKF.

Evidence:
- production diff inspection;
- source-level ownership trace;
- runtime baseline parity below.

Forbidden substitute:
- Agent assertion without diff/evidence.

---

# 14. P1 runtime validation on full `eee_01`

After unit/seam tests pass, run one bounded full `eee_01` experiment with P1 covariance computation active.

Use the same:

- bag;
- config;
- offline production path;
- effective params;
- trajectory serialization;
- official NTU evaluator

as the frozen pre-P1 baseline.

Persist:

- HEAD;
- feature/config state;
- trajectory SHA256/MD5;
- row count;
- official matched count;
- official ATE;
- byte parity versus pre-P1 baseline;
- numerical trajectory parity versus pre-P1 baseline;
- build/test logs;
- real RC and completion sentinel.

## G-P1.6 — trajectory parity

Because P1 covariance is not yet consumed by the estimator:

Preferred strict gate:

```text
BYTE_PARITY(pre-P1, P1) = PASS
```

If byte parity fails, investigate before accepting P1. Do not immediately weaken the gate.

Use the old project's two-layer parity analysis to distinguish:
- actual estimator drift;
- serialization difference;
- quaternion comparison artifact.

If true byte difference remains, classify it and STOP FOR OWNER unless you can prove it is a non-estimator serialization-only change.

## G-P1.7 — accuracy parity

The canonical NTU evaluator must preserve the frozen baseline:

```text
rows    : exact match
matched : exact match
ATE     : |delta| <= 1e-6 m
```

Given P1 should not influence the estimator, any material ATE change is a defect until proven otherwise.

---

# 15. Performance / instrumentation policy

P1 is plumbing, not a profiling round.

- keep heavy diagnostics OFF by default;
- do not add per-point dumps to normal runs;
- if a covariance summary is useful, make it bounded/optional;
- first use tests and trajectory parity;
- only enable deeper instrumentation for a specific anomaly.

Record offline wall time so we notice accidental catastrophic overhead, but do not tune performance in P1 unless there is an obvious regression.

---

# 16. Directory / artifact hygiene

Maintain and reuse:

```text
spec/prob_lio/
prompts/prob_lio/
tools/prob_lio/
eval/prob_lio/
tests/prob_lio/
results/prob_lio/
```

Rules:

- important reusable scripts/tests/evaluators must be committed;
- one-off scratch experiments may use `/tmp`;
- no important production/eval tool may exist only in `/tmp`;
- large trajectories/bags/log floods need not be committed if hashes + small evidence summaries suffice;
- configs and paths must remain parameterizable;
- reuse existing helpers instead of duplicating functionality.

Update `spec/prob_lio/SPEC.md` after each closure:

- P0 → `CLOSED / OWNER-CORRECTIVE-CLOSED`
- P1 → only mark `CLOSED/PASS` after all P1 gates pass
- P2 remains `NOT STARTED`

Record each round/commit/evidence path.

---

# 17. Build / run hygiene

For every build/test/experiment invocation:

- one bounded operation per shell invocation;
- `set -o pipefail` when piping;
- preserve real return code;
- emit completion sentinel;
- if UI spinner continues, first inspect actual process state;
- do not rerun merely because UI still spins;
- prevent concurrent duplicate bag/estimator contamination;
- keep the Owner-authorized aggressive cleanup policy before canonical experiments.

Examples of completion sentinels:

```text
__P0_EVAL_DONE_RC=<rc>
__P1_TEST_DONE_RC=<rc>
__P1_RUN_DONE_RC=<rc>
```

---

# 18. Commit policy

Use at least two logical commits:

### Commit A — P0 corrective closure

Example:

```text
chore(prob-lio): close P0 eval and parity contracts
```

Contains:
- evaluator assets;
- parity contract/tool updates;
- SPEC seam repair/bookkeeping;
- manifest dependency correction if needed;
- P0 baseline eval evidence.

No P1 production covariance code.

### Commit B — P1 implementation

Example:

```text
feat(prob-lio): add current lidar point covariance
```

Contains:
- P1 production implementation;
- config/plumbing;
- P1 tests;
- P1 evidence/SPEC update.

A small final docs-only commit to record actual commit SHA is acceptable. Do not rewrite history.

Push normally to `origin/prob-lio`. No force push.

---

# 19. Final report — required format

## Agent State Consensus
- starting HEAD
- branch/upstream
- worktree
- discovered relevant paths

## P0 Corrective Closure
- old evaluator paths found
- canonical NTU semantics
- historical `60b57aa` rows/matched/ATE
- current pre-P1 rows/matched/ATE
- evaluator/trajectory hashes
- byte parity result
- numeric parity result
- explanation of any ~1e-8 rad diagnostic
- SPEC seam repair
- aggressive cleanup policy recorded
- rosbag manifest status
- P0 corrective commit SHA

## P1 Source/Frame Audit
- FAST-LIVO2 `calcBodyCov()` active path and point frame
- Super-LIO raw→undistort→VoxelGridClosest→`points_body_v3_` path
- exact frame contract chosen
- config parameter source/units

## P1 Implementation
- production files changed
- exact ownership of `body_cov_list_`
- what was deliberately NOT changed (P2–P5)

## P1 Tests
For G-P1.1 ... G-P1.5:
- test name/path
- semantic invariant
- negative mutation used
- PASS evidence

## P1 eee_01 Validation
- run command
- RC + sentinel
- trajectory hash
- byte parity vs pre-P1
- numeric parity
- rows / matched / official ATE
- runtime

## Diff Audit
- diff from pre-P1 P0 closure commit
- confirm no map covariance / QR covariance / weight / gate / ESKF changes

## SPEC
- P0 status
- P1 status
- P2 status
- evidence/result paths

## Commits
- Commit A SHA
- Commit B SHA
- final HEAD
- clean worktree
- push status

## Gate Summary
- P0-EVAL
- HARD GATE A
- G-P1.1 ... G-P1.7
- PASS/FAIL with evidence

If any hard gate fails, do not call the stage closed.

---

# 20. Review contract

Your report is not the acceptance authority.

The Owner/reviewer will independently inspect:

- actual commits and diff;
- estimator production code;
- evaluator implementation/provenance;
- test code and negative mutations;
- trajectory/hash/eval evidence;
- SPEC state;
- reusable tools;
- actual return codes.

Make the repository self-explanatory enough that every PASS can be verified without trusting your prose.
