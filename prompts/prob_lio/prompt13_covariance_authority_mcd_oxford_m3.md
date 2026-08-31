# Prob-LIO Prompt 13 — Covariance-Config Authority Corrective + Oxford Official FAST-LIVO2 Semantics + MCD Rerun + M3 Outdoor Bring-up

## 0. Mission

Prompt 13 corrects dataset-specific LiDAR uncertainty configuration authority and continues generalization.

Goals:
1. correct MCD `ntu_night_08` probability runs because `dept_err/beam_err` previously fell back to generic defaults;
2. normalize Oxford to the **official Oxford Spires FAST-LIVO2 benchmark semantics**, including code defaults for omitted `dept_err/beam_err`;
3. use proper M3DGR/FAST-LIVO2 Avia covariance semantics for M3 Outdoor;
4. preserve Prompt12 lightweight evidence / heavy-debug-off policy.

No P0–P5 math changes.
No tuning.
No P5 lifecycle work.
No VIO/camera work.

Explicit exclusions remain:
```text
M3DGR Corridor01 = EXCLUDED_BY_OWNER
M3DGR Corridor02 = EXCLUDED_BY_OWNER
```

## 1. State consensus

Expected:
```text
repo   ~/super_livo/src/Super-LIO
branch prob-lio
HEAD   approximately f23cede
```

Verify:
```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -20
```

Require clean worktree and `HEAD == origin/prob-lio`.

Register:
```text
prompts/prob_lio/prompt13_covariance_authority_mcd_oxford_m3.md
```

No history rewrite or force push.

---

# PART A — Dataset-specific covariance authority

## 2. General rule

For `dept_err/beam_err`, use the strongest algorithm-comparison authority:

```text
A. If dataset authors publish the exact FAST-LIVO2 benchmark fork/config:
   reproduce that exact benchmark semantics,
   including same-branch code defaults for parameters omitted from YAML.

B. Else if official FAST-LIVO2 provides the applicable sensor/dataset config:
   use its values.

C. Else use audited sensor-spec-derived values only with an explicit
   SENSOR_SPEC_DERIVED / PROXY label.
```

Never silently inherit Prob-LIO generic code defaults.

---

# PART B — Oxford official benchmark semantics

## 3. Oxford authority

Oxford Spires' official dataset repository states that its localisation benchmark uses method forks with branch:

```text
config-used-OSD
```

as the configurations used for evaluation.

For FAST-LIVO2 the authority is:

```text
repo   https://github.com/ori-drs/FAST-LIVO2
branch config-used-OSD
config config/oxford_spires.yaml
```

Audit exact commit/blob identities.

The benchmark config is expected to contain:
```text
lidar topic   /hesai/pandar
imu topic     /alphasense_driver_ros/imu
scan_line     64
blind         1
point_filter  1
Oxford benchmark extrinsics
```

### Critical covariance semantics

`config/oxford_spires.yaml` omits:

```text
lio/dept_err
lio/beam_err
```

The **same official benchmark branch** defines defaults in `loadVoxelConfig()`:

```text
dept_err = 0.05 m
beam_err = 0.02 deg
```

Therefore Prompt13 Oxford comparison authority is:

```text
dept_err = 0.05
beam_err = 0.02
provenance = OXFORD_FASTLIVO2_BENCHMARK_DEFAULT
```

Do NOT replace these with Hesai data-sheet values merely because they look more physical.

Reason: the experiment is comparing against the **official Oxford FAST-LIVO2 benchmark configuration semantics**.

## 4. G-P13.OX1 — Oxford covariance authority

Before any Oxford rerun prove:
1. official Oxford dataset repo points benchmark methods to `config-used-OSD`;
2. exact FAST-LIVO2 fork/branch/commit;
3. `oxford_spires.yaml` omits `dept_err/beam_err`;
4. same branch defaults are exactly `0.05/0.02`;
5. Prob-LIO effective config explicitly resolves those values.

Persist repo/branch/commit/config hash/source hash/values/provenance label.

## 5. Oxford config lineage

Do not wholesale replace Super-LIO with FAST-LIVO2 parameters.

Keep existing audited Super-LIO Oxford settings for:
- Super geometry/HKNN;
- Super IESKF;
- topic transport;
- other frozen Super-specific settings.

Explicitly source Prob-LIO point covariance from the official Oxford FAST-LIVO2 benchmark semantics:

```text
dept_err=0.05
beam_err=0.02
```

Maintain per-field lineage:
```text
Oxford Super-LIO owner config
Oxford dataset calibration
Oxford FAST-LIVO2 benchmark config/default
Prob-LIO fixed ablation switch
```

---

# PART C — Oxford evaluator + offline transport

## 6. Re-audit Oxford evaluator

Prompt12 Oxford numbers cannot remain final if evaluator registry still says both:
```text
ACTIVE
```
and:
```text
not publishable / frame audit pending
```

Resolve:
- GT source/frame;
- estimator output frame;
- required body/extrinsic transform;
- timestamp policy;
- alignment;
- primary metric.

If proven full-trajectory:
```text
GT_TYPE = FULL_TRAJECTORY
```

Else:
```text
GT_SEMANTICS_BLOCKED
```

No invented ATE.

## 7. Oxford LIO-only cache

Original Oxford bag is bz2 and camera-heavy. Topic filtering via `rosbag::View` still incurs chunk decompression.

Create/reuse an LIO-only cache containing only:
```text
/hesai/pandar
/alphasense_driver_ros/imu
```

Requirements:
- exact message contents;
- exact record order;
- exact timestamps;
- no estimator-side preprocessing;
- uncompressed or LZ4;
- cache stored outside tracked Git;
- original/cache bag hashes saved.

## 8. G-P13.OX2 — cache parity

Before all Oxford variants, compare original bz2 vs cache on a bounded B0/P4 smoke.

Require:
- equal rows/timestamps;
- byte-identical trajectory if deterministic, otherwise strict numeric identity;
- same metric;
- LiDAR/IMU message count/order parity.

Report original runtime, cache runtime, speedup.

---

# PART D — MCD night08 corrective

## 9. MCD sensor identity

Audit MCD `ntu_night_08` from dataset-author sources.

Expected LiDAR:
```text
Livox Mid-70
```

Do not use Avia covariance just because FAST-LIVO2 has `avia.yaml`.

Prompt11 MCD probability variants used omitted parameters and fell back to generic:
```text
dept_err=0.05
beam_err=0.02
```

Those P4/P5 probability cells must be superseded.

## 10. MCD covariance authority

Determine Mid-70 `dept_err/beam_err` from strongest authority:

1. dataset-author / exact algorithm config if available;
2. official Livox Mid-70 measurement spec mapped carefully to `CalcLidarPointCov`;
3. published Mid-70 algorithm config if stronger sources absent.

Do not silently map a maximum angular accuracy or beam divergence to a 1σ covariance.

Record whether the bearing value is:
```text
1σ precision
accuracy bound
repeatability
beam divergence
proxy
```

If only a proxy can be justified, label:
```text
MCD_MID70_SENSOR_SPEC_PROXY
```

Do not call it OFFICIAL_FASTLIVO2.

## 11. G-P13.MCD1 — MCD provenance

Before rerun record:
```text
LiDAR model
dept_err
beam_err
units
statistical meaning
source
mapping into CalcLidarPointCov
provenance class
```

Effective config must explicitly contain both values.

## 12. MCD rerun policy

B0 is covariance-independent if probability pipeline is off.

Prove:
```text
old B0 config
vs
same B0 with explicit corrected dept/beam
→ byte-identical trajectory
```

If GREEN, reuse B0.

Mark old current probability cells:
```text
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

as:
```text
SUPERSEDED_INVALID_COVARIANCE_CONFIG
```

and rerun all five with corrected explicit Mid-70 authority.

---

# PART E — M3DGR Outdoor

## 13. Exclusions

Do not run:
```text
Corridor01
Corridor02
```

Both remain:
```text
EXCLUDED_BY_OWNER
```

## 14. Allowed targets

Only:
```text
Outdoor01
Outdoor04
```

Verify local inventory.

Expected LiDAR:
```text
Livox Avia
```

## 15. M3 point covariance authority

For M3 Outdoor use FAST-LIVO2 official Avia covariance semantics:

```text
dept_err = 0.02 m
beam_err = 0.05 deg
```

Persist exact FAST-LIVO2 `config/avia.yaml` commit/blob/hash.

## 16. M3 remaining config provenance

Do not copy all FAST-LIVO2 Avia defaults.

Separately recover/audit:
- topics;
- extrinsics;
- IMU noise;
- blind/max range;
- point filtering;
- Super-LIO geometry;
- IESKF settings.

Sources:
- M3DGR dataset calibration;
- old `super-livo` branch;
- old runner/evidence manifests;
- exact historical M3 config.

If exact Super-side config provenance is unresolved:
```text
CONFIG_PROVENANCE_BLOCKED
```
Do not guess.

## 17. G-P13.M3

Before numeric Outdoor run require:
```text
bag/hash
GT/hash
GT type
evaluator/hash
Avia covariance authority
calibration/extrinsic authority
Super-LIO config authority
effective config
```

---

# PART F — Frozen variants

## 18. Keep semantics unchanged

### B0
```text
fixed_1000
Super legacy association
probability covariance not used by estimator
```

### P4-LC
```text
cov ON
map_pose_cov_model=livo2_compat
QR plane covariance ON
P4 probabilistic soft weight
Super legacy association
```

### P4-RC
Only:
```text
map_pose_cov_model=super_right_consistent
```

### P5-ACTIVE
Map/P4 fixed to P4-LC:
```text
association_mode=prob_livo2
association_pose_cov_model=livo2_compat
association_sensor_cov_model=livo2_active_compat
```

### P5-SENSOR-CORR
Only:
```text
association_sensor_cov_model=extrinsic_consistent
```

### P5-BOTH-CORR
Only from previous:
```text
association_pose_cov_model=super_right_consistent
```

No new variants.

---

# PART G — Execution order

## 19. Order

```text
1. Oxford authority/evaluator/cache audit
2. Oxford corrected 6-cell set if gates GREEN
3. MCD covariance provenance
4. MCD B0 non-interference parity
5. MCD corrected 5 probability cells
6. M3 Outdoor provenance recovery
7. Outdoor01 6-cell if GREEN
8. Outdoor04 6-cell if GREEN
```

Do not rerun valid:
```text
NTU eee_01
NTU nya_01
NTU sbs_01
```

Do not run Corridors.

---

# PART H — Production diagnostics remain light

## 20. Freeze

Every canonical run:
```text
cov_validation_mode=light
prob_assoc_shadow_enable=false
full eigensolver diagnostics=false
per-point dumps=false
per-candidate dumps=false
FD=false
sanitizers=false
heavy profiler=false
verbose debug=false
```

If a run fails, use a separate minimal diagnostic run. Diagnostic evidence is not canonical.

---

# PART I — Lightweight Git evidence only

## 21. Track only

```text
run_manifest
evaluation output
effective config
variant diff
small summary
hashes
```

Keep outside Git:
```text
node.log
roscore/roslaunch logs
cache bags
duplicate GT
large trajectories unless explicitly promoted
raw debug dumps
raw stdout/stderr
```

No history rewrite.

---

# PART J — Source/config identity

## 22. Metadata

Every new run:
```text
algorithm_commit
run_git_head
run_git_dirty=false
production_code_oid
dataset_config_sha256
bag_sha256
GT_sha256
evaluator_sha256
effective_config_sha256
variant
trajectory_sha256
runtime
metric
classification
```

Oxford cache also:
```text
original_bag_sha256
lio_cache_bag_sha256
cache_transport
cache_parity_status
```

---

# PART K — Matrix update

## 23. `spec/prob_lio/ABLATION_MATRIX.md`

### Oxford
Supersede Prompt12 Oxford cells if covariance/evaluator authority was incomplete. New cells must point to:
- explicit Oxford FAST-LIVO2 benchmark `0.05/0.02`;
- verified evaluator;
- cache parity if cache used.

### MCD
Old probability cells:
```text
SUPERSEDED_INVALID_COVARIANCE_CONFIG
```
until replaced.

B0 may be reused after non-interference proof.

### M3
```text
Corridor01 = EXCLUDED_BY_OWNER
Corridor02 = EXCLUDED_BY_OWNER
Outdoor01 = metric/BLOCKED
Outdoor04 = metric/BLOCKED
```

No ambiguous blanks.

---

# PART L — Gates

## 24. Required

```text
G-P13.OX1  Oxford official FAST-LIVO2 covariance authority
G-P13.OX2  Oxford cache transport parity
G-P13.OX3  Oxford evaluator/frame semantics
G-P13.MCD1 MCD Mid-70 covariance provenance
G-P13.MCD2 MCD B0 covariance-parameter non-interference
G-P13.M3   M3 Outdoor provenance + Avia covariance
G-P13.AB   exact variant isolation
G-P13.DBG  heavy diagnostics OFF
G-P13.EVID lightweight tracked evidence only
G-P13.MAT  correct matrix supersession/update
```

Each gate needs concrete evidence.

---

# PART M — Scientific reporting

## 25. Pairwise deltas

For each valid sequence report:
```text
B0 → P4-LC
P4-LC → P4-RC
P5-ACTIVE → P5-SENSOR-CORR
P5-SENSOR-CORR → P5-BOTH-CORR
```

No tuning after observing results.

Do not start P4 noise-model attribution automatically.

---

# 26. Suggested commits

```text
docs(prob-lio): freeze dataset covariance authorities
config(prob-lio): correct mcd and oxford covariance provenance
tools(prob-lio): add oxford lio cache parity workflow
docs(prob-lio): update prompt13 ablation matrix
```

If M3 config recovered:
```text
config(prob-lio): recover audited m3dgr outdoor config
```

No force push.

---

# 27. Final report

## State
- starting/final HEAD
- branch/upstream
- prompt registration

## Oxford authority
- official dataset repo
- FAST-LIVO2 fork/branch/commit
- config hash
- omission of dept/beam
- same-branch defaults
- resolved `0.05/0.02`
- provenance label

## Oxford evaluator
- GT/output frames
- transform
- timestamps
- alignment
- status

## Oxford cache
- original/cache hashes
- size
- message parity
- trajectory parity
- runtimes/speedup

## Oxford 6 cells

## MCD corrective
- Mid-70 authority
- dept/beam
- statistical meaning/provenance
- B0 parity
- superseded old probability cells
- corrected 5 probability cells

## M3 Outdoor
- Corridor exclusions
- Outdoor config provenance
- Avia `0.02/0.05`
- results/blocks

## Instrumentation/evidence
- heavy debug OFF
- lightweight Git evidence
- no forbidden artifacts tracked

## Matrix
- superseded/new cells

## Gates
PASS/FAIL with evidence.

## Pairwise deltas

## Commits / final clean status

## Stop
Return to Owner.
Do not start covariance tuning or P4 attribution automatically.
