# Prob-LIO Prompt 11 — Generalization Ablation Matrix + P4/P5 Primary-Metric Pipeline

## 0. Mission

Prompt 11 starts the **generalization / multi-dataset ablation phase**.

This is no longer a P5 debugging round.

The goals are:

1. apply a small, exact Prompt-10 documentation corrective before experiments;
2. create one authoritative cross-dataset ablation table under `spec/prob_lio/`;
3. audit and reuse the old `super-livo` branch's dataset configs, config lineage, and evaluator semantics without switching the active working branch;
4. build a reusable, clean-source P4/P5 evaluation pipeline;
5. separate the known **P4 map/world covariance semantic issue** from the separate **P5 association sensor-covariance semantic issue**;
6. run the P4/P5 ablation variants across the currently auditable datasets in the frozen generalization order;
7. record the dataset-authoritative **primary metric**, not blindly call every metric ATE.

No parameter tuning is authorized.

---

# 1. State consensus

Expected repository:

```text
~/super_livo/src/Super-LIO
```

Expected branch:

```text
prob-lio
```

Expected starting frontier is Prompt-10 docs consolidation, approximately:

```text
f5dfef6
```

Verify:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -20
```

Requirements:

```text
branch = prob-lio
worktree = clean
HEAD = origin/prob-lio
```

Do not reset/rebase/rewrite history.
No force push.

Register this exact prompt as:

```text
prompts/prob_lio/prompt11_generalization_ablation_pipeline.md
```

Update the prompt index.

---

# PART A — EXACT PROMPT-10 DOCUMENTATION CORRECTIVE

## 2. Re-audit and fix these known Prompt-10 documentation defects first

These are documentation corrections only. Do not change production behavior in Part A.

### DOC-1 — `SPEC.md §2` still uses stale future-tense P3/P4 language

Current stale meaning is equivalent to:

```text
QR uncertainty must later propagate...
fixed 1000 / compute_error remain frozen until a later probabilistic replacement...
```

But P3/P4 are already complete.

Rewrite as current frozen semantics:

```text
P3 propagates plane uncertainty through Super-LIO's actual QR estimator;
no PCA/eigen substitute is used.

P4 probabilistic mode replaces the fixed-1000 measurement information with
the audited probabilistic soft weight.

Canonical association remains Super legacy compute_error().
P5 probabilistic association is experimental only.
```

Do not alter the historical baseline-invariant section that documents what the original code used.

### DOC-2 — `SPEC.md §3 Stage roadmap` P1/P2 labels conflict with current authority

Normalize the current stage roadmap to:

```text
P0–P4 = CLOSED / OWNER VERIFIED
P5 = EXPERIMENTAL / NON-CANONICAL
Generalization = ACTIVE / IN PROGRESS   # only after Prompt11 starts real runs
```

Historical corrective labels may be preserved in a `History/Notes` column, not in the current `Status` column.

### DOC-3 — stale Prompt-8/P9 lifecycle claims still appear unqualified inside SPEC history

The detailed history still contains old claims such as:

```text
3980/3981 one iteration
need_converge rarely reached
no mask-persistence skip
Super performs no probability re-evaluation in convergence phase
single-shot lifecycle proven
LIFECYCLE_MISMATCH_* as proven root cause
```

Do not leave these as unqualified prose in the authoritative SPEC.

Either:
- move the invalid details to `HISTORY.md`; or
- prefix the affected whole subsection with a highly visible
  `SUPERSEDED / INVALIDATED — historical record only`.

The current truth remains:

```text
P5 clean regression is reproducible.
Exact root cause is unresolved.
P5 is non-canonical because of empirical performance, not a proven sole root cause.
```

### DOC-4 — S4 seam location is stale

Current SPEC says conceptually:

```text
map_init(g_map_cov_init_enable)
```

but production `map_init()` uses the master:

```text
g_prob_lio_cov_enable
```

Correct S4 to the actual production source/ownership.

### DOC-5 — stale `3586 reach need_converge` wording

Where Prompt-9 history uses the corrected histogram:

```text
final obs_iter2 = 395
final obs_iter3 = 195
final obs_iter4 = 3391
```

remember logging is 1-based:

```text
obs_iter=1 ↔ ESKF iter=0
obs_iter=4 ↔ ESKF iter=3
need_converge=true at ESKF iter>2
```

Therefore the convergence callback count is **3391**, not 3586.

Do not promote old lifecycle causal claims; this is only a factual bookkeeping correction.

---

# 3. HARD GATE G-P11.0 — Documentation corrective

Before starting production changes or dataset runs:

- DOC-1..DOC-5 fixed;
- no production code/config-value changes;
- `SPEC.md` current state internally consistent;
- old invalid lifecycle text either moved or explicitly superseded.

Commit the docs corrective separately if convenient, e.g.:

```text
docs(prob-lio): correct pre-generalization spec details
```

A docs-only commit is acceptable before later production work.

---

# PART B — CREATE THE CROSS-DATASET ABLATION AUTHORITY

## 4. Create `spec/prob_lio/ABLATION_MATRIX.md`

This is the authoritative generalization result ledger.

Do **not** call the entire file an "ATE table", because some datasets do not have full-trajectory GT.

Use the concept:

```text
PRIMARY METRIC
```

ATE is used only where GT semantics permit trajectory ATE.

### Required compact matrix

At minimum:

| Dataset | Sequence | GT type | Primary metric | Evaluator | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status/Notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|

Cell rules:
- numerical value + unit when valid;
- `BLOCKED(<reason>)` when evaluator/GT/config semantics are unresolved;
- `DIVERGED`;
- `INVALID`;
- `NOT_RUN`;
- never invent a number.

Also maintain a detailed per-run ledger below the compact matrix:

```text
dataset
sequence
variant_id
GT_type
primary_metric_name
primary_metric_value
unit
evaluator/provenance
bag path/hash
GT path/hash
config path/hash
config provenance
algorithm_commit
run_git_head
production_code_tree_oid
git_dirty
run_id
trajectory hash
rows/completion
runtime
classification
notes
```

Do not manually edit run values before evidence exists.

---

# PART C — OLD `super-livo` BRANCH CONFIG / EVALUATOR AUDIT

## 5. Audit old branch without switching away from `prob-lio`

Use read-only Git inspection such as:

```bash
git fetch origin super-livo:refs/remotes/origin/super-livo   # if needed
git ls-tree -r origin/super-livo -- configs/super_livo/reference_base
git show origin/super-livo:<path>
```

Do **not** checkout `super-livo` into the active worktree.

Audit at minimum:

```text
configs/super_livo/reference_base/
scripts/super_livo/evaluation/
docs/super_livo/datasets/
```

Expected reference files include:

```text
mcd_night08.yaml
mcd_night08.lineage.yaml
ntu_eee_01.yaml
ntu_eee_01.lineage.yaml
ntu_nya_01.yaml
ntu_nya_01.lineage.yaml
oxford_quarter01.yaml / lineage
m3dgr_corridor01.yaml / lineage
```

Expected evaluator/reference tools include:

```text
eval_ntu_viral_official.py
ntu_viral_official_ate.py
eval_tum_translation.py
prepare_mcd_gt.py
eval_m3dgr_aruco.py
validate_canonical_benchmark_matrix.py
```

Do not trust names alone. Read the source and record semantics/provenance.

---

## 6. Known audit anchors — verify, do not blindly copy

### MCD `ntu_night_08`

Expected old reference-base LIO anchors:

```text
authority        SUPER_LIO_DATASET_CONFIG
blind            2.0
maxrange         1000.0
filter_rate      3
scan_voxel       0.5
map_voxel        0.5
max_iterations   4
quit_eps         0.001
IMU noise        Super-LIO dataset config lineage
extrinsic        dataset-author calibration / reconciled MCD config
time offsets     existing canonical zero-offset Owner decision
```

The old `prepare_mcd_gt.py` documents:

```text
pose_inW.csv
Body = VN100 IMU
W_T_B
no prism/lever arm
no scale
```

However its docstring says Day10.

For `ntu_night_08`, independently verify:
- local `pose_inW.csv` schema;
- body/frame semantics;
- timestamp units/range;
- whether the same conversion semantics apply.

Do not reuse the Day10 assumption merely because the CSV columns look similar.

### NTU `eee_01`, `nya_01`

Expected LIO anchors:

```text
blind 2.0
maxrange 150.0
filter_rate 3
scan/map voxel 0.5
max_iterations 4
quit_eps 0.001
```

Use the accepted dataset-author-compatible evaluator already ported for NTU when applicable.

### Oxford `Quarter_01`

Old lineage explicitly says several LIO parameters are an existing Oxford Owner/config decision, not a published Super-LIO dataset config.

Do not relabel them as official.

Audit:
- local Oxford files/topics;
- current/old Oxford config;
- dataset-author calibration;
- GT type;
- evaluator semantics.

If full-trajectory GT/evaluator semantics cannot be proven, mark quantitative row BLOCKED rather than inventing ATE.

### M3DGR

Old lineage uses existing M3 config + dataset-author calibration.

For `Corridor01`, GT is `FINAL_RELATIVE_POSE`, not full trajectory.

The primary metric is:

```text
M3DGR ArUco first-to-last relative translation error (m)
```

Do **not** report evo ATE for Corridor01.

If other local M3DGR sequences have audited full-trajectory `.tum` GT, they may use a trajectory APE evaluator after explicit frame/time audit.

---

# 7. HARD GATE G-P11.1 — Dataset config/evaluator provenance

Before any sequence runs, generate a preflight record proving:

```text
bag exists + hash
required topics exist
calibration source
GT path/hash
GT type
primary metric
evaluator path/hash/provenance
Super-LIO config source
effective config snapshot
```

No sequence is allowed to enter numeric evaluation without this preflight.

If GT type is:
- `FULL_TRAJECTORY`: trajectory ATE/APE permitted after time/frame audit;
- `FINAL_RELATIVE_POSE`: terminal relative metric only;
- `NONE/UNKNOWN`: qualitative/sanity only; numerical metric forbidden.

---

# PART D — DEFINE THE P4/P5 ABLATION AXES

## 8. Required variant IDs

These variants deliberately isolate different semantics.

### B0 — original Super-LIO weighting/association baseline

```text
covariance probability pipeline = OFF for estimator semantics
p2p_weight_mode = fixed_1000
association_mode = super_legacy
```

Preserve all dataset-specific geometric/config parameters.

### P4-LC — canonical Prob-LIO

```text
cov_enable = ON
cov_validation_mode = light
map_pose_cov_model = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable = ON
p2p_weight_mode = prob_livo2
association_mode = super_legacy
prob_assoc_shadow_enable = OFF
```

This is the canonical generalization candidate.

### P4-RC — corrected P4/P2 map-pose covariance

Same as P4-LC, only:

```text
map_pose_cov_model = super_right_consistent
```

Purpose:
- isolate the previously audited FAST-LIVO2 world/map pose-covariance semantic issue (#89 lineage);
- association remains Super legacy, so no P5 behavior is involved.

### P5-ACTIVE — FAST-LIVO2-active-style association semantics (experimental)

Hold the **map/P4 side fixed**:

```text
map_pose_cov_model = livo2_compat
p2p_weight_mode = prob_livo2
```

Association:

```text
association_mode = prob_livo2
association_pose_cov_model = livo2_compat
association_sensor_cov_model = livo2_active_compat
```

This is explicit bug-compatible/active-reference P5 association.

### P5-SENSOR-CORR — isolate the separate P5 sensor/extrinsic semantic correction

Hold everything from P5-ACTIVE fixed except:

```text
association_sensor_cov_model = extrinsic_consistent
```

Purpose:
- isolate the P5 FAST-LIVO2 active association sensor-covariance semantic issue;
- **do not** change the P4 map model at the same time.

### P5-BOTH-CORR — corrected P5 sensor + corrected association pose covariance

Hold map side fixed:

```text
map_pose_cov_model = livo2_compat
```

Use:

```text
association_mode = prob_livo2
association_sensor_cov_model = extrinsic_consistent
association_pose_cov_model = super_right_consistent
```

This is a sequential P5 correction after P5-SENSOR-CORR.

Do not call it a P4 correction.

### Optional P5-POSE-CORR

A full 2×2 P5 factorial would additionally use:

```text
association_sensor_cov_model = livo2_active_compat
association_pose_cov_model = super_right_consistent
```

This is **optional**, not required in Prompt11.

Only add it if a specific attribution question requires the full factorial. Do not multiply experiments automatically.

---

# PART E — THE SEPARATE P5 FAST-LIVO2 SENSOR-COVARIANCE SEMANTIC

## 9. Audit the reference source before coding

Re-audit local FAST-LIVO2 `StateEstimation()`.

Expected active source behavior to verify:

1. `calcBodyCov()` is called on the downsampled **LiDAR-frame** point.
2. The point is then transformed with `extR_/extT_`.
3. In the current-query association covariance, active code uses conceptually:

\[
\Sigma^{active}_{sensor,W}
=
R_{WI}\Sigma_L R_{WI}^{T}
\]

without the LiDAR→IMU extrinsic rotation around \(\Sigma_L\).

4. In contrast, the active final P4-like weight uses:

\[
(R_{WI}R_{LI})\Sigma_L(R_{WI}R_{LI})^T
\]

which includes `extR_`.

This is a distinct P5 association semantic issue, related to the public FAST-LIVO2 code discussion around issue #174.

Do not rely on issue prose alone; source code is authority.

---

# 10. Add a P5-only association sensor covariance policy if source audit confirms

Current Prob-LIO P1 stores:

\[
\Sigma_I = R_{LI}\Sigma_LR_{LI}^{T}.
\]

Add conceptually:

```text
association_sensor_cov_model:
  extrinsic_consistent
  livo2_active_compat
```

Recommended default:

```text
extrinsic_consistent
```

to preserve current Prob-LIO behavior.

The setting is **P5 experimental only**.

It must not affect:
- `association_mode=super_legacy`;
- map covariance;
- P3;
- P4 final weight.

### `extrinsic_consistent`

\[
\Sigma_{sensor,W}
=
R_{WI}\Sigma_I R_{WI}^{T}.
\]

### `livo2_active_compat`

Recover/use LiDAR-frame sensor covariance:

\[
\Sigma_L = R_{LI}^{T}\Sigma_I R_{LI}
\]

then emulate active reference association:

\[
\Sigma_{sensor,W}^{compat}
=
R_{WI}\Sigma_LR_{WI}^{T}.
\]

Equivalent direct construction from the same P1 LiDAR-frame source is acceptable if it avoids numerical/drift duplication.

Do not alter P1 canonical storage ownership.

---

# 11. HARD GATE G-P11.2 — P5 association-sensor semantic dual mode

Use a synthetic nonidentity `R_LI` and anisotropic \(\Sigma_L\).

Require:

### active-compatible mode

\[
\Sigma_W
=
R_{WI}\Sigma_LR_{WI}^{T}
\]

### corrected mode

\[
\Sigma_W
=
R_{WI}R_{LI}\Sigma_LR_{LI}^{T}R_{WI}^{T}.
\]

Required tests:

1. nonidentity `R_LI`: modes differ exactly as independent reference predicts;
2. `R_LI=I`: modes are equivalent to tight/machine tolerance;
3. multiple normals: scalar `n^T Σ n` parity;
4. changing `association_sensor_cov_model` cannot change P4 final weight;
5. `association_mode=super_legacy` cannot change trajectory/measurement behavior because of this P5-only setting.

Negative mutations:
- accidentally include `R_LI` in `livo2_active_compat`;
- accidentally omit `R_LI` in corrected mode;
- use `R_LI^T` on wrong side;
- route the policy into map insertion;
- route the policy into P4 final weight.

---

# 12. HARD GATE G-P11.3 — Variant isolation

For each sequence, persist the resolved effective-config diff.

Required exact isolation:

```text
P4-LC → P4-RC:
  ONLY map_pose_cov_model changes.

P5-ACTIVE → P5-SENSOR-CORR:
  ONLY association_sensor_cov_model changes.

P5-SENSOR-CORR → P5-BOTH-CORR:
  ONLY association_pose_cov_model changes.
```

Dataset-specific algorithm parameters, bag, GT, evaluator, and all other Prob-LIO settings must be identical within the same sequence.

If this fails, the A/B result is INVALID.

---

# PART F — REUSABLE PIPELINE

## 13. Build/reuse a parameterized ablation runner

Prefer reusing the established offline runner rather than creating another estimator implementation.

Create a thin reusable orchestration layer under:

```text
tools/prob_lio/
```

that accepts parameters conceptually like:

```text
--dataset
--sequence
--variant
--bag
--config
--evaluator-profile
--output-root
--canonical
```

Do not hard-code the user's absolute home directory when a repo/workspace-relative or CLI path is available.

The runner must:

1. enforce canonical clean-worktree rule;
2. snapshot effective loaded config;
3. save bag/config/GT/evaluator identities;
4. run one bounded experiment;
5. preserve true return code/completion sentinel;
6. evaluate with the registered evaluator;
7. write a machine-readable result manifest;
8. never automatically edit tracked `ABLATION_MATRIX.md` during a canonical run.

A separate aggregation tool may update/generate matrix-ready output after runs.

---

# 14. Evaluator registry

Create or normalize a reusable evaluator registry, e.g.:

```text
eval/prob_lio/evaluator_registry.yaml
```

or an equivalent documented data structure.

Each profile must state:

```text
dataset family
GT type
primary metric name
unit
evaluator path
upstream provenance
required frame transform
timestamp policy
alignment policy
```

Port old-branch evaluator tools into `eval/prob_lio/` only when missing and only with provenance headers/tests.

Do not fork subtly different duplicate evaluators if current branch already contains an accepted one.

---

# 15. HARD GATE G-P11.4 — evaluator semantic correctness

Synthetic/unit tests must prove:

- NTU profile uses the accepted dataset-author-compatible interpolation/lever-arm/SE3-no-scale semantics;
- M3DGR ArUco `FINAL_RELATIVE_POSE` cannot be routed into trajectory ATE;
- `UNKNOWN/NONE` GT cannot emit a numerical accuracy metric;
- generic TUM translation evaluator does not silently apply scale;
- MCD conversion/evaluator uses only verified night08 frame/time semantics.

A wrong evaluator profile must fail before a bag run is called "canonical".

---

# PART G — CLEAN CODE COMMIT BEFORE GENERALIZATION RUNS

## 16. Commit production/tooling changes first

If Prompt11 adds:
- P5 association sensor semantic switch;
- runner;
- evaluator registry/tools;
- reusable configs;

then:

```text
implement
→ focused tests
→ full build/tests
→ commit
→ verify clean
→ only then canonical runs
```

Suggested logical commits:

```text
docs(prob-lio): correct prompt10 pre-generalization spec
feat(prob-lio): add p5 association sensor semantic ablation
tools(prob-lio): add generalization ablation pipeline
```

They may be combined cleanly if appropriate, but do not run canonical evidence from dirty source.

---

# PART H — `eee_01` REGRESSION / PIPELINE SMOKE

## 17. Use `eee_01` as a smoke gate before wider datasets

Do not blindly rerun every historical variant if clean evidence is reusable.

### Reuse existing clean evidence in ABLATION_MATRIX

Allowed for:
- B0;
- P4-LC;
- P4-RC;
- historical applied P5 corrected-sensor behavior.

Mark:

```text
REUSED_CLEAN_EVIDENCE
```

with exact run path/source.

### Required post-change smoke

Because Prompt11 introduces a P5-only sensor semantic switch, run enough to prove no contamination:

1. **P4-LC** with `association=super_legacy`:
   - trajectory must byte-match canonical P4 (`259d3fbc...`);
   - ATE `0.088831554...`.

2. **P5-ACTIVE vs P5-SENSOR-CORR on NTU**:
   NTU has `R_LI=I`.

   Therefore, with every other setting identical:

```text
trajectory(P5-ACTIVE)
==
trajectory(P5-SENSOR-CORR)
```

must hold byte/numerically because the two sensor covariance semantics coincide at identity extrinsic.

If they differ, the new P5 sensor-mode implementation is wrong or variant isolation failed.

Expected applied P5 behavior remains non-canonical; do not tune it.

3. If P5-BOTH-CORR is run, compare with the prior pure association-pose corrected observation where semantics align. Treat as regression smoke, not a required accuracy target.

---

# 18. HARD GATE G-P11.5 — `eee_01` smoke

Required GREEN before running new sequences:

```text
P4-LC canonical byte parity
P5 active-vs-sensor-corrected identity under R_LI=I
variant isolation exact
all runs clean
no evaluator regression
```

No new dataset begins if this gate is RED.

---

# PART I — GENERALIZATION DATASET ORDER

## 19. Frozen execution order

Use the current Owner order:

```text
1. MCD ntu_night_08
2. NTU VIRAL eee_01 / nya_01
3. Oxford Quarter_01
4. M3DGR
```

`eee_01` smoke occurs before step 1 as a pipeline regression gate; its existing result also fills the NTU matrix row.

Within M3DGR:
- start with `Corridor01`;
- additional local sequences may be added only after GT/config/evaluator provenance is audited.

Do not introduce unrelated datasets in Prompt11.

---

# 20. Per-sequence preflight transaction

Before the first variant for each sequence:

1. locate bag(s);
2. hash bag(s);
3. inspect topics/types/counts;
4. locate calibration;
5. locate GT;
6. classify GT type;
7. audit old `super-livo` config + lineage;
8. map old config values to current Super-LIO config keys;
9. audit evaluator;
10. write a frozen effective-config base for that sequence.

If multiple bags must be synchronized/merged for MCD, reuse audited infrastructure; do not invent ordering semantics.

### Failure classifications

```text
CONFIG_PROVENANCE_BLOCKED
GT_SEMANTICS_BLOCKED
EVALUATOR_BLOCKED
INPUT_ADAPTER_BLOCKED
ALGORITHM_DIVERGED
EXECUTION_FAILED
CANONICAL_VALID
```

A provenance/infra BLOCKED sequence may be recorded and the pipeline may continue to the next sequence after cleanly documenting the block.

An algorithmic RED must not be hidden by skipping to a nicer dataset; record it prominently.

---

# PART J — REQUIRED GENERALIZATION ABLATIONS

## 21. Required variants per quantitatively auditable sequence

Run, in this order:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

Why this order:

```text
B0 → P4-LC
  = effect of canonical probabilistic modeling/soft weighting vs original.

P4-LC → P4-RC
  = P4/P2 world/map pose-covariance semantic correction only.

P5-ACTIVE → P5-SENSOR-CORR
  = P5 association sensor/extrinsic correction only.

P5-SENSOR-CORR → P5-BOTH-CORR
  = P5 association pose-covariance correction only.
```

Do not compare P4-RC directly against P5-BOTH-CORR and claim one isolated cause; multiple axes differ.

---

# 22. Run count / repeat policy for Prompt11

Prompt11 is the **generalization screening ablation**, not the final paper repeatability campaign.

Default:

```text
1 clean deterministic canonical run per sequence × variant
```

because the offline path has established deterministic/parity behavior.

If:
- same clean config produces nondeterministic trajectory hashes; or
- runtime process contamination is suspected;

then stop that sequence and diagnose reproducibility before adding repetitions.

Do not automatically multiply every cell into three repeats in this prompt.

Record `n=1 screening` in the matrix.

Final promoted benchmark repetitions can be a later Owner decision.

---

# 23. Execution hygiene

For every canonical run:

```text
commit → clean → run
```

Require:

```text
git_dirty=false
run_git_head
algorithm_commit
production_code_tree_oid
bag hash
effective config hash
evaluator hash
```

One bounded experiment per shell invocation.

No concurrent duplicate rosbag/estimator runs.

Retain the established aggressive stale-process cleanup policy.

Heavy diagnostics OFF:
- full covariance eigensolver diagnostics;
- P5 shadow;
- per-point dumps;
- FD;
- heavy profiler.

Only bounded production summaries.

---

# 24. P5 failure handling

P5 is experimental/non-canonical.

If a P5 variant:
- diverges;
- crashes;
- produces NaN;
- completes with very poor metric;

record the result exactly.

Do not tune `sigma_num`, noise, floor, association threshold, voxel size, or dataset config to rescue it.

Continue to the next predeclared P5 variant only if the runner is clean and the sequence remains evaluable.

P4 is the canonical generalization decision path.

---

# 25. HARD GATE G-P11.6 — clean evidence transaction

For every populated matrix cell with a numerical metric:

- clean committed source;
- exact variant identity;
- effective config;
- bag/GT/evaluator identities;
- completion;
- trajectory/output;
- evaluator result;
- no cross-run contamination.

If any element is missing, cell status is not `CANONICAL_VALID`.

---

# PART K — MATRIX / TRACEABILITY GATES

## 26. HARD GATE G-P11.7 — ablation matrix completeness

For each attempted sequence:

The matrix must contain:
- GT type;
- primary metric name/unit;
- evaluator provenance;
- config provenance;
- each required variant status;
- metric or reason no metric exists;
- run/evidence reference.

No empty ambiguous cells.

Use `NOT_RUN`, `BLOCKED`, etc.

---

# 27. HARD GATE G-P11.8 — no tuning / provenance lock

Within one sequence:
- all dataset-specific non-ablation parameters are frozen across variants;
- only declared ablation knobs change;
- no parameter chosen after observing an ATE/metric result.

Compare effective config snapshots programmatically.

If unexpected fields differ, mark affected A/B invalid and rerun only after fixing the pipeline.

---

# 28. HARD GATE G-P11.9 — metric naming correctness

The matrix/tooling must not label all values as `ATE`.

Examples:

```text
NTU full trajectory:
  NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M

M3DGR Corridor01:
  M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M

Unknown GT:
  no numerical primary metric
```

A final-relative metric in an `ATE` column is a gate failure.

---

# PART L — EXPECTED OLD-BRANCH RESOURCES TO REUSE

## 29. Config provenance

Reference, adapt, and cite old branch material rather than rediscovering from memory.

Expected useful paths:

```text
origin/super-livo:configs/super_livo/reference_base/mcd_night08.yaml
origin/super-livo:configs/super_livo/reference_base/mcd_night08.lineage.yaml
origin/super-livo:configs/super_livo/reference_base/ntu_eee_01.yaml
origin/super-livo:configs/super_livo/reference_base/ntu_nya_01.yaml
origin/super-livo:configs/super_livo/reference_base/oxford_quarter01.yaml
origin/super-livo:configs/super_livo/reference_base/oxford_quarter01.lineage.yaml
origin/super-livo:configs/super_livo/reference_base/m3dgr_corridor01.yaml
origin/super-livo:configs/super_livo/reference_base/m3dgr_corridor01.lineage.yaml
```

If a file is absent, record it; do not fabricate it.

---

## 30. Evaluator provenance

Expected useful paths:

```text
origin/super-livo:scripts/super_livo/evaluation/eval_ntu_viral_official.py
origin/super-livo:scripts/super_livo/evaluation/ntu_viral_official_ate.py
origin/super-livo:scripts/super_livo/evaluation/eval_tum_translation.py
origin/super-livo:scripts/super_livo/evaluation/prepare_mcd_gt.py
origin/super-livo:scripts/super_livo/evaluation/eval_m3dgr_aruco.py
```

Old branch dataset semantics:

```text
origin/super-livo:docs/super_livo/datasets/dataset_registry.md
origin/super-livo:docs/super_livo/datasets/evaluation_protocol.md
origin/super-livo:docs/super_livo/datasets/calibration_time_sync.md
```

Port only what is needed; maintain upstream and old-branch provenance headers.

---

# PART M — FINAL REPORT

## 31. Final report format

### Agent State Consensus
- starting HEAD
- branch/upstream
- clean status
- prompt registration

### Prompt-10 Corrective
For DOC-1..DOC-5:
- exact old statement
- exact replacement
- file/section
- commit

### ABLATION_MATRIX
- path
- schema
- rows/sequences created
- metric-type handling

### Old-Branch Audit
For each target sequence:
- config source/authority
- evaluator source/authority
- GT type
- calibration source
- anything blocked/ambiguous

### FAST-LIVO2 P5 Sensor Semantic Audit
- exact active source trace
- `calcBodyCov` frame
- missing/included `R_LI`
- final P4 contrast
- issue/provenance
- conclusion

### P5 Sensor-Mode Implementation
- production files
- config values/default
- formula for both modes
- no-P4-contamination proof

### Gates
Report:

```text
G-P11.0 docs corrective
G-P11.1 config/evaluator provenance
G-P11.2 P5 sensor dual mode
G-P11.3 variant isolation
G-P11.4 evaluator semantics
G-P11.5 eee smoke
G-P11.6 clean evidence
G-P11.7 matrix completeness
G-P11.8 no tuning/config lock
G-P11.9 metric naming
```

PASS/FAIL with evidence.

### `eee_01` Smoke
- reused historical cells
- new P4-LC byte parity
- P5-ACTIVE vs P5-SENSOR-CORR identity at `R_LI=I`
- clean source IDs

### Generalization Results
For each sequence in execution order:
- preflight
- B0
- P4-LC
- P4-RC
- P5-ACTIVE
- P5-SENSOR-CORR
- P5-BOTH-CORR
- primary metric and unit
- runtime
- classification

### Variant Attribution
Do not merely rank scores. State the pairwise deltas:

```text
B0 → P4-LC
P4-LC → P4-RC
P5-ACTIVE → P5-SENSOR-CORR
P5-SENSOR-CORR → P5-BOTH-CORR
```

per sequence.

### Blocked Rows
- exact block reason
- evidence
- no invented metric

### Diff Audit
Confirm:
- no tuning;
- no S6 change;
- no P4 formula change;
- no QR/HKNN/ESKF change;
- new P5 sensor switch is association-only.

### Commits
- docs corrective SHA
- P5 sensor semantic SHA
- pipeline/tool SHA
- evidence/docs SHA(s)
- final HEAD
- worktree clean
- push status

### Final Matrix State
- link/path to `spec/prob_lio/ABLATION_MATRIX.md`
- completed cells
- blocked cells
- next Owner decision

---

# 32. Stop conditions

Stop for Owner if:
- `eee_01` smoke G-P11.5 fails;
- new P5 sensor mode contaminates P4;
- old-branch config provenance conflicts materially with current config and cannot be reconciled;
- evaluator/GT semantics cannot be safely determined for the next sequence;
- canonical clean-run infrastructure fails.

A provenance-blocked individual sequence may be marked `BLOCKED` and skipped to the next sequence only after the block is fully documented and no algorithmic failure is being hidden.

---

# 33. Review contract

The final report is not acceptance authority.

Owner/reviewer will independently audit:
- Prompt-10 doc corrections;
- `ABLATION_MATRIX.md`;
- old-branch config/evaluator provenance;
- P5 active-vs-extrinsic-consistent association source semantics;
- variant config diffs;
- clean run manifests;
- evaluator selection;
- numerical matrix cells.

No matrix cell is accepted merely because the Agent wrote `PASS`.
