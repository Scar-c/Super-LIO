# Prob-LIO Prompt 12 — Generalization Continuation + Repository/Evidence Hygiene

## 0. Mission

Continue Prompt11 generalization, but first clean the repository/workspace organization and freeze a lighter evidence policy.

This round has four goals:

1. clean Prompt11 documentation placement and repository organization;
2. stop committing heavy runtime artifacts and make future production runs light by default;
3. complete the missing NTU `sbs_01` ablation row;
4. continue Oxford / M3DGR generalization under exact config/evaluator provenance, with **M3DGR Corridor01 and Corridor02 explicitly excluded**.

Do **not** modify P0–P5 algorithm math in this round.

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

Expected starting frontier is the Prompt11 published state, approximately:

```text
833ef4d
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

Register this exact prompt as:

```text
prompts/prob_lio/prompt12_generalization_hygiene_sbs_oxford_m3.md
```

Update prompt index.

No history rewrite.
No force push.

---

# PART A — CLEAN REPOSITORY / DOCUMENT PLACEMENT

## 2. Move Prompt11 audit out of `spec/`

Current problem:

```text
spec/prob_lio/PROMPT11_AUDIT.md
```

does not belong in the authoritative spec directory.

`spec/prob_lio/` should contain current architecture / evidence authority / ablation state, not per-prompt execution audit prose.

Move it with history preserved, preferably:

```bash
mkdir -p prompts/prob_lio/audits
git mv spec/prob_lio/PROMPT11_AUDIT.md \
       prompts/prob_lio/audits/PROMPT11_AUDIT.md
```

Update:
- `prompts/prob_lio/README.md`;
- any links from SPEC/EVIDENCE_INDEX/ABLATION_MATRIX;
- Prompt11 references.

Do not duplicate the file.

### Intended hierarchy

```text
spec/prob_lio/
  SPEC.md
  EVIDENCE_INDEX.md
  HISTORY.md
  ABLATION_MATRIX.md
  <future current-authority spec files only>

prompts/prob_lio/
  prompt*.md
  README.md
  audits/
    PROMPT11_AUDIT.md
    <future prompt execution audits>
```

---

# 3. HARD GATE G-P12.H1 — documentation placement

Require:

```text
spec/prob_lio/PROMPT11_AUDIT.md does not exist
prompts/prob_lio/audits/PROMPT11_AUDIT.md exists
all internal links valid
no duplicate authority copy
```

`spec/prob_lio/` must remain an authoritative current-state directory, not a dump directory.

---

# PART B — STOP COMMITTING HEAVY RUNTIME ARTIFACTS

## 4. New evidence policy

Prompt11 committed too much runtime material.

From Prompt12 onward, tracked Git evidence must be **lightweight**.

### Allowed to track

Per canonical run, track only compact evidence such as:

```text
run_manifest.yaml/json
evaluation_result.yaml/json/txt
effective_config.yaml
variant/config diff
hashes / source identities
small summary CSV/JSON
README/index references
```

### Do NOT track by default

```text
node.log
roscore/master logs
roslaunch logs
raw debug dumps
per-point/per-frame heavy dumps
duplicate ground_truth.tum
duplicate bag-derived GT
full profiler output
build/devel artifacts
rosbag files
large generated trajectories
large raw stdout/stderr captures
```

For trajectory reproducibility, the tracked manifest must at least preserve:

```text
trajectory_sha256
trajectory_rows
completion
metric
```

The full trajectory may stay in local runtime storage and does not need to be committed for every screening run.

If a specific trajectory is later promoted to paper/canonical archival evidence, Owner may explicitly authorize committing it.

---

# 5. Separate local runtime artifacts from tracked evidence

Refactor the generalization runner so normal full run output goes to a dedicated **git-ignored / outside-repo runtime directory**, for example:

```text
~/super_livo/results/prob_lio_runtime/<run_id>/
```

or another clear workspace-level path outside the Git repository.

Tracked lightweight evidence should be exported separately into:

```text
Super-LIO/results/prob_lio/evidence/<run_id>/
```

or the existing compact evidence structure.

Do not use `/tmp` for authoritative runtime evidence.

The final chosen paths must be:
- explicit;
- reusable;
- path-parameterized where practical.

---

# 6. Existing Prompt11 heavy evidence cleanup

Do **not** rewrite history.

For currently tracked Prompt11 runtime-heavy artifacts:

- keep the historical commits untouched;
- remove them from the **current tracked tree** if they are unnecessary under the new light-evidence policy;
- preserve local copies before `git rm --cached`/removal if needed.

Examples to untrack from current HEAD when present:

```text
node.log
roscore logs
duplicate GT exports
large trajectory files
raw stdout/stderr
```

Do not remove:
- manifests;
- evaluator outputs;
- effective configs;
- hashes;
- compact summaries required by `ABLATION_MATRIX.md`.

If an existing heavy file is actually small and essential, document why it remains tracked.

No filter-branch, no rebase, no force push.

---

# 7. Add precise ignore rules

Add precise `.gitignore` rules for runtime artifacts.

Avoid an overly broad rule that hides all of `results/prob_lio/`.

Prefer targeted patterns such as:

```text
**/node.log
**/roscore*.log
**/roslaunch*.log
**/stdout.log
**/stderr.log
**/*.bag
**/raw_dump*
```

For trajectory/GT outputs, ignore only the runtime-storage paths or explicit generated filenames chosen by the new runner.

Do not accidentally ignore tracked manifests/evaluator outputs.

---

# 8. HARD GATE G-P12.H2 — evidence hygiene

Create a focused hygiene test/check proving:

1. a normal canonical run can write full runtime artifacts locally;
2. tracked evidence export contains only the allowed compact files;
3. `git status --short` after a production run does not show heavy runtime artifacts;
4. no `.bag`, build/devel, giant log, or raw debug dump is staged/tracked;
5. manifests still reference/hash the local full artifacts.

Report:

```text
tracked evidence size per run
local runtime size per run
```

for at least one smoke run.

---

# PART C — PRODUCTION RUNS MUST BE LIGHT BY DEFAULT

## 9. Heavy diagnostics OFF

For every normal generalization run, freeze:

```text
cov_validation_mode = light
prob_assoc_shadow_enable = OFF
full eigensolver diagnostics = OFF
per-point covariance dumps = OFF
per-candidate association dumps = OFF
FD diagnostics = OFF
sanitizers = OFF unless diagnosing a failure
heavy profiler = OFF
verbose debug logging = OFF
```

Use only bounded production summaries/counters.

If a run behaves abnormally:
1. record the phenomenon;
2. form a hypothesis;
3. enable only the minimum relevant diagnostic in a separate diagnostic run.

Do not routinely rerun heavy intrinsic instrumentation when trajectory/execution is normal.

---

# 10. HARD GATE G-P12.H3 — production instrumentation freeze

Before each canonical dataset sequence, save an effective-config check proving heavy diagnostics are OFF.

If any heavy diagnostic is ON unintentionally:
- mark the run noncanonical;
- do not use its metric in `ABLATION_MATRIX.md`.

---

# PART D — PRODUCTION SOURCE IDENTITY CLEANUP

## 11. Split code identity from dataset config identity

Current `production_tree_oid` covers too much of `src/super_lio/`, so dataset config changes can alter it even when estimator code is identical.

Add metadata fields:

```text
production_code_oid
dataset_config_sha256
```

### `production_code_oid`

Must identify actual production estimator/tool build semantics, e.g. a deterministic hash/tree identity over:

```text
src/super_lio/include/**/*.h
src/super_lio/include/**/*.hpp
src/super_lio/src/**/*.cpp
relevant CMake/build files
```

Do not include:
- runtime outputs;
- dataset YAML configs;
- generated CSV/logs.

### `dataset_config_sha256`

Hash the exact loaded dataset config separately.

Keep old `production_tree_oid` for backward compatibility if desired, but future cross-dataset comparisons should use:

```text
production_code_oid
dataset_config_sha256
```

---

# 12. HARD GATE G-P12.H4 — code/config identity

For two sequences using the same estimator binary/source but different dataset configs:

```text
production_code_oid = identical
dataset_config_sha256 = different as expected
```

Negative mutation:
- change estimator C++ and prove `production_code_oid` changes.

---

# PART E — COMPLETE NTU `sbs_01`

## 13. Add `sbs_01` to the generalization matrix

The current local NTU inventory includes:

```text
eee_01
nya_01
sbs_01
```

Prompt11 omitted `sbs_01`.

Prompt12 must audit and run `sbs_01`.

Before running, audit old `super-livo` branch for:

```text
sbs_01 config
lineage/provenance
evaluator assumptions
topics/calibration
```

If there is no explicit `sbs_01` reference config, determine whether the NTU VIRAL family config is legitimately shared across sequences by source/provenance.

Do not assume merely because all three are NTU.

Record the exact justification.

---

# 14. `sbs_01` preflight

Require:

```text
bag exists + hash
topics/types/counts
GT topic/path
GT frame semantics
evaluator profile
calibration
config source/provenance
effective config
```

If `sbs_01` uses the same accepted NTU evaluator contract:
- prove it;
- record the same evaluator hash/provenance.

If a sequence-specific difference exists, register it explicitly.

---

# 15. Required `sbs_01` variants

Run the same screening matrix:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

No tuning.

Because NTU has identity `R_LI` in the accepted config, if `sbs_01` has the same identity extrinsic:

```text
P5-ACTIVE
vs
P5-SENSOR-CORR
```

should be identical up to the established deterministic tolerance.

If `sbs_01` has a different extrinsic, do not assume identity; use actual config.

---

# 16. HARD GATE G-P12.NTU — `sbs_01` canonical screening

All 6 cells require:

```text
clean committed source
heavy diagnostics OFF
exact variant isolation
accepted NTU evaluator
effective config snapshot
metric + unit
run manifest
no process contamination
```

Update:

```text
spec/prob_lio/ABLATION_MATRIX.md
```

with all 6 statuses/metrics.

---

# PART F — OXFORD CONFIG PORT + ABLATION

## 17. Port the old exact Oxford Super-LIO config

Prompt11 treated Oxford as blocked because the current branch lacked a config.

But `origin/super-livo` contains an executable Oxford config and lineage.

Audit and port the exact old config rather than inventing parameters.

Expected old path:

```text
origin/super-livo:src/super_lio/config/oxford_quarter01.yaml
```

Expected lineage:

```text
origin/super-livo:configs/super_livo/reference_base/oxford_quarter01.lineage.yaml
```

Use `git show`, not branch checkout.

Record exact source blob/hash.

---

# 18. Oxford semantic checks before run

Audit:

```text
LiDAR topic/type
IMU topic/type
compressed/raw image irrelevant to LIO-only Prob-LIO unless runner depends on it
extrinsics
time semantics
GT type
GT frame
GT timestamp policy
evaluator
```

Do not revive old visual-adapter complexity unless the current Prob-LIO LIO pipeline actually needs camera input.

This phase is LIO P4/P5 ablation.

---

# 19. Oxford evaluator rule

If a verified full-trajectory GT/evaluator exists:

```text
GT_TYPE = FULL_TRAJECTORY
```

and use the audited primary trajectory metric.

If GT semantics are still unresolved:

```text
GT_SEMANTICS_BLOCKED
```

Do not invent ATE.

The config is no longer allowed to remain blocked solely because the current branch originally lacked the YAML after the exact old config has been ported and verified.

---

# 20. Required Oxford variants

If preflight is GREEN:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

No tuning.

Record actual Oxford `R_LI`.

This sequence is especially useful for the P5 sensor/extrinsic semantic if the extrinsic rotation is nonidentity.

---

# PART G — M3DGR: EXCLUDE BOTH CORRIDOR BAGS

## 21. Explicit dataset exclusion

Owner decision:

```text
M3DGR Corridor01 = DO NOT RUN
M3DGR Corridor02 = DO NOT RUN
```

Do not execute either bag in Prompt12.

Do not spend runtime/config-recovery effort specifically to enable Corridor01/02.

In `ABLATION_MATRIX.md`, mark them explicitly:

```text
EXCLUDED_BY_OWNER
```

not:

```text
NOT_RUN
```

and not:

```text
CONFIG_PROVENANCE_BLOCKED
```

This is an intentional experimental-design exclusion.

---

# 22. M3DGR sequences allowed in Prompt12

Use only non-Corridor local sequences, currently expected:

```text
Outdoor01
Outdoor04
```

First verify actual local inventory.

Do not invent `Dynamic01`.

For each allowed M3 sequence:
- locate exact config provenance;
- audit GT type;
- audit evaluator.

If exact M3 config provenance remains unresolved:

```text
CONFIG_PROVENANCE_BLOCKED
```

and do not run.

Do not substitute:
- `M2DGR.yaml`;
- `livox_360.yaml`;
- a guessed config;
- a config tuned from observed results.

---

# 23. Recover M3 config provenance narrowly

Search read-only:

```text
origin/super-livo
git history
old runner scripts
old evidence manifests
lineage files
```

for the exact M3 config referenced historically as:

```text
current_committed_m3dgr_runner_config
```

If found:
- record commit/blob/path;
- port exact semantics;
- verify topics/extrinsic/range/noise/IESKF parameters.

If not found:
- keep Outdoor rows BLOCKED;
- stop M3 execution.

Do not broaden into a new config-design task.

---

# 24. M3 evaluator semantics

For Outdoor sequences with audited `.tum` full trajectory GT:

```text
GT_TYPE = FULL_TRAJECTORY
```

and use the accepted trajectory evaluator after time/frame verification.

Do not use the Corridor ArUco final-relative evaluator for Outdoor sequences unless dataset semantics explicitly require it.

Corridor01/02 remain excluded regardless of evaluator availability.

---

# 25. Required M3 Outdoor variants

Only if exact config + evaluator provenance are GREEN:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

Otherwise mark all corresponding Outdoor cells:

```text
CONFIG_PROVENANCE_BLOCKED
```

or the exact block classification.

---

# PART H — RUN ORDER

## 26. Prompt12 execution order

Use:

```text
0. repository/evidence hygiene
1. NTU sbs_01
2. Oxford Quarter_01
3. M3DGR Outdoor01
4. M3DGR Outdoor04
```

Explicitly skip:

```text
M3DGR Corridor01
M3DGR Corridor02
```

Do not rerun existing valid Prompt11 MCD/eee/nya cells unless a pipeline regression smoke is genuinely required.

---

# PART I — VARIANT ISOLATION / NO TUNING

## 27. Freeze existing Prompt11 variants

Keep exactly:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

Within one sequence, only declared ablation knobs may differ.

No changes after observing metrics to:

```text
sigma_num
dept_err
beam_err
0.001 floor
blind
maxrange
voxel
IESKF iterations
plane threshold
HKNN
IMU noise
```

unless an authoritative old dataset config proves the starting value was wrong before the sequence's first canonical run.

---

# 28. HARD GATE G-P12.AB — exact config isolation

Programmatically diff effective configs.

Require:

```text
B0 → P4-LC:
  only intended Prob-LIO enable/weight semantics

P4-LC → P4-RC:
  only map_pose_cov_model

P5-ACTIVE → P5-SENSOR-CORR:
  only association_sensor_cov_model

P5-SENSOR-CORR → P5-BOTH-CORR:
  only association_pose_cov_model
```

Any unexpected diff invalidates the A/B pair.

---

# PART J — PRODUCTION EXECUTION CONTRACT

## 29. Canonical run transaction

For every new canonical run:

```text
code/tool changes
→ tests
→ commit
→ clean
→ run
→ evaluate
→ export lightweight evidence
```

Metadata:

```text
algorithm_commit
run_git_head
run_git_dirty=false
run_git_status_short=""
production_code_oid
dataset_config_sha256
bag_sha256
GT hash
evaluator hash
effective config hash
variant
completion
trajectory hash
metric
runtime
```

---

# 30. Process hygiene

One bounded experiment per shell.

Before run:
- verify no duplicate estimator/rosbag process;
- use the established cleanup policy;
- do not allow concurrent same-sequence experiments.

After run:
- verify cleanup;
- record completion;
- do not commit process logs.

Any contamination:

```text
INVALID / CONTAMINATED
```

and rerun only after cleanup.

---

# PART K — UPDATE THE MATRIX

## 31. Update `spec/prob_lio/ABLATION_MATRIX.md`

Add/update rows for:

```text
NTU sbs_01
Oxford Quarter_01
M3DGR Outdoor01
M3DGR Outdoor04
M3DGR Corridor01 = EXCLUDED_BY_OWNER
M3DGR Corridor02 = EXCLUDED_BY_OWNER
```

Every cell must be one of:

```text
CANONICAL_VALID(<metric>)
DIVERGED
INVALID
CONFIG_PROVENANCE_BLOCKED
GT_SEMANTICS_BLOCKED
EVALUATOR_BLOCKED
EXCLUDED_BY_OWNER
NOT_RUN
```

No blank ambiguous cell.

---

# 32. Generalization interpretation

Do not tune or over-generalize conclusions in Prompt12.

After new rows, report pairwise deltas:

```text
B0 → P4-LC
P4-LC → P4-RC
P5-ACTIVE → P5-SENSOR-CORR
P5-SENSOR-CORR → P5-BOTH-CORR
```

per sequence.

Especially track whether the existing pattern persists:

```text
eee_01: P4 improves
nya_01: P4 slightly improves
night08: P4 regresses
```

Do not start a noise-calibration attribution round yet unless Owner explicitly authorizes after reviewing the expanded matrix.

---

# PART L — HARD GATES SUMMARY

## 33. Required gates

Report each independently:

```text
G-P12.H1  Prompt11 audit moved out of spec
G-P12.H2  lightweight evidence policy
G-P12.H3  heavy diagnostics OFF
G-P12.H4  production_code_oid/config hash split
G-P12.NTU sbs_01 6-cell screening
G-P12.OX  Oxford config/evaluator provenance
G-P12.M3  Corridor exclusions + Outdoor provenance
G-P12.AB  exact variant isolation
G-P12.RUN clean run transaction
G-P12.MATRIX no ambiguous cells
```

A gate is not PASS because the report says so; include evidence.

---

# 34. Commit strategy

Use focused commits.

Suggested:

```text
chore(prob-lio): clean prompt audit and runtime evidence layout
tools(prob-lio): split runtime artifacts from tracked evidence
docs(prob-lio): add prompt12 generalization rows
```

If Oxford config is ported:

```text
config(prob-lio): port audited Oxford Super-LIO config
```

If M3 exact config is recovered:

```text
config(prob-lio): recover audited M3DGR outdoor config
```

Evidence-only commits should contain lightweight evidence only.

No force push.

---

# 35. Final report format

## Agent State Consensus
- starting HEAD
- branch/upstream
- worktree
- Prompt12 path

## Repository Cleanup
- moved `PROMPT11_AUDIT.md`
- final directory hierarchy
- links updated

## Evidence Hygiene
- old heavy tracked files removed from current tree
- ignore rules
- local runtime root
- tracked evidence root
- tracked/local size comparison

## Production Identity
- `production_code_oid`
- `dataset_config_sha256`
- verification across datasets

## NTU sbs_01
- config provenance
- GT/evaluator provenance
- 6 variant metrics
- P5 active/sensor-corrected identity check if `R_LI=I`

## Oxford
- old config source path/blob
- lineage
- port diff
- GT/evaluator semantics
- 6 variant metrics or exact block reason

## M3DGR
- Corridor01 = EXCLUDED_BY_OWNER
- Corridor02 = EXCLUDED_BY_OWNER
- Outdoor01 provenance/result
- Outdoor04 provenance/result

## Instrumentation Audit
Confirm all production runs had:
```text
light covariance validation
P5 shadow OFF
heavy debug OFF
```

## Matrix
- updated `spec/prob_lio/ABLATION_MATRIX.md`
- completed cells
- blocked cells
- excluded cells

## Pairwise Generalization Deltas
Per sequence:
- B0→P4-LC
- P4-LC→P4-RC
- P5-ACTIVE→P5-SENSOR-CORR
- P5-SENSOR-CORR→P5-BOTH-CORR

## Gates
PASS/FAIL with evidence.

## Commits
- cleanup/tool/config/evidence commits
- final HEAD
- origin/prob-lio
- worktree clean
- no force push

## Stop
Do not start a noise-tuning or P4 attribution round automatically.
Return the expanded matrix to Owner for the next decision.

---

# 36. Review contract

Owner/reviewer will independently inspect:

- repository layout;
- Prompt11 audit relocation;
- `.gitignore` / evidence policy;
- production runner defaults;
- heavy-debug effective config;
- `sbs_01` config/evaluator;
- Oxford old config port;
- M3 Corridor exclusions;
- M3 Outdoor provenance;
- variant config diffs;
- lightweight manifests;
- matrix cells.

No result is accepted solely because the Agent labels it `CANONICAL_VALID`.
