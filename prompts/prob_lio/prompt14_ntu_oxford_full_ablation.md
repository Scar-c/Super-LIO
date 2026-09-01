# Prob-LIO Prompt 14 — NTU + Oxford Full Ablation Expansion with Persistent Oxford LIVO Cache

## 0. Mission

Continue the Prob-LIO ablation/generalization phase using **only NTU VIRAL and Oxford Spires**.

Owner decision:

```text
ACTIVE GENERALIZATION DATASETS:
  NTU VIRAL
  Oxford Spires

HISTORICAL / NO LONGER USED FOR FUTURE GENERALIZATION:
  MCD
  M3DGR
  all other datasets
```

Prompt14 has two coupled goals:

1. complete the remaining NTU/Oxford six-variant ablation experiments;
2. replace the temporary Oxford **LIO-only** cache concept with one persistent **LIVO-ready cache** per Oxford bag, so the same cached bag can be reused now for LIO and later when Visual is integrated.

No P0–P5 mathematics changes.
No parameter tuning.
No camera/VIO algorithm integration in this round.
The camera is cached now only to make the transport artifact future-proof.

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

Expected starting frontier is Prompt13 final state, approximately:

```text
b636054
```

Verify:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -20
```

Require:

```text
branch = prob-lio
HEAD == origin/prob-lio
worktree clean
```

Register this exact prompt:

```text
prompts/prob_lio/prompt14_ntu_oxford_full_ablation.md
```

Update prompt index.

No history rewrite.
No force push.

---

# PART A — Freeze active benchmark scope

## 2. Current active scope

Update current-authority docs so the active benchmark family is:

```text
NTU VIRAL
Oxford Spires
```

MCD/M3DGR remain preserved only as historical evidence:

```text
HISTORICAL / OUT-OF-ACTIVE-SCOPE
```

Do not delete old evidence.
Do not include MCD/M3DGR in active aggregate statistics.
Do not rerun them.

Inspect/update:

```text
spec/prob_lio/SPEC.md
spec/prob_lio/ABLATION_MATRIX.md
spec/prob_lio/HISTORY.md
spec/prob_lio/EVIDENCE_INDEX.md
results/prob_lio/README.md
prompts/prob_lio/README.md
```

### G-P14.SCOPE

No current-authority roadmap may still describe MCD or M3DGR as future active targets.

---

# PART B — Frozen covariance authority

## 3. NTU

For every NTU sequence explicitly resolve:

```text
dept_err = 0.02 m
beam_err = 0.01 deg
```

Authority:

```text
FAST-LIVO2 official NTU_VIRAL config
```

Do not re-estimate or tune these values per sequence.

Verify the new NTU sequences use the accepted same LiDAR/extrinsic/calibration family before assuming shared semantics.

---

## 4. Oxford

For every Oxford sequence explicitly resolve:

```text
dept_err = 0.05 m
beam_err = 0.02 deg
```

Authority:

```text
Oxford Spires official FAST-LIVO2 benchmark fork
branch config-used-OSD
config/oxford_spires.yaml
same-branch code defaults for omitted dept_err/beam_err
```

Do not replace these with Hesai datasheet values.

### G-P14.COV

Every canonical effective config must explicitly contain the relevant pair.
No silent fallback.

---

# PART C — Oxford official camera authority

## 5. Official FAST-LIVO2 camera

The Oxford Spires official FAST-LIVO2 benchmark config uses:

```yaml
common:
  img_topic: "/alphasense_driver_ros/cam0/color/image"
```

Therefore the camera authority for the future Oxford LIVO path is:

```text
CAMERA = AlphaSense cam0
```

Do **not** cache cam1/cam2 for the canonical LIVO transport unless a later Owner decision changes the visual architecture.

The current local raw Oxford bags contain:

```text
/alphasense_driver_ros/cam0/debayered/image/compressed
/alphasense_driver_ros/cam1/debayered/image/compressed
/alphasense_driver_ros/cam2/debayered/image/compressed
```

For the persistent LIVO cache, preserve the **cam0 compressed source stream**:

```text
/alphasense_driver_ros/cam0/debayered/image/compressed
```

exactly as recorded.

Do not decode/re-encode images while constructing the cache.

Reason:
- preserving the original `sensor_msgs/CompressedImage` bytes gives exact transport provenance;
- avoids unnecessarily expanding the persistent cache;
- avoids introducing image-decoding/re-encoding as part of cache generation;
- the existing/authorized CompressedImage→sensor_msgs/Image adapter can later expose the official FAST-LIVO2 raw-image interface.

Future LIVO input bridge must map:

```text
source:
  /alphasense_driver_ros/cam0/debayered/image/compressed
  sensor_msgs/CompressedImage

deterministic adapter output:
  /alphasense_driver_ros/cam0/color/image
  sensor_msgs/Image
```

with:
- same header timestamp;
- no resize;
- no crop;
- no geometric rectification;
- no exposure manipulation;
- no color conversion beyond the exact decode required by the encoded source;
- no algorithm-side semantic change.

Do not run Visual estimation in Prompt14.

### G-P14.CAM

Before cache construction prove:
1. official Oxford FAST-LIVO2 config selects cam0;
2. source bag contains cam0 compressed stream;
3. cache retains exactly cam0, not cam1/cam2;
4. future adapter topic/type contract is documented;
5. no image conversion occurs during cache creation.

---

# PART D — Persistent Oxford LIVO cache

## 6. Cache naming/location

For every Oxford source bag:

```text
<Sequence>.bag
```

create a persistent cache **in the same sequence directory**:

```text
<Sequence>_LIVO.bag
```

Examples:

```text
bag/OXFORD/Quarter_01/Quarter_01_LIVO.bag
bag/OXFORD/Church_05/Church_05_LIVO.bag
bag/OXFORD/College_03/College_03_LIVO.bag
bag/OXFORD/Palace_01/Palace_01_LIVO.bag
```

This cache is a dataset artifact, not Git evidence.

Never `git add` these bags.

---

## 7. Canonical cache contents

Each Oxford `_LIVO.bag` must contain **exactly these sensor streams**:

```text
/hesai/pandar
  sensor_msgs/PointCloud2

/alphasense_driver_ros/imu
  sensor_msgs/Imu

/alphasense_driver_ros/cam0/debayered/image/compressed
  sensor_msgs/CompressedImage
```

Do not include:
- cam1;
- cam2;
- unrelated diagnostics;
- old estimator outputs;
- GT topics unless there is a concrete transport need.

GT remains an external evaluation file (`gt-tum.txt`).

The cache must be usable for:
- present LIO-only runs by ignoring the camera stream;
- future LIVO runs by using cam0 via the deterministic compressed-image adapter.

Thus **do not create a second LIO-only cache and later another LIVO cache**.

One canonical Oxford transport artifact:

```text
*_LIVO.bag
```

only.

---

## 8. Cache compression

Choose a transport format optimized for repeated offline use.

Preferred:

```text
LZ4
```

or uncompressed if empirical read throughput is clearly better and disk capacity is acceptable.

Do not use bz2 for the cache.

Record:
```text
source compression
cache compression
source size
cache size
build time
```

Cache generation cost is one-time and cache is persistent.

---

## 9. Byte-level/message-level parity

For all retained source topics verify:

```text
message count
ROS topic/type
bag record ordering
message/header timestamps
serialized message payload identity
```

The retained LiDAR/IMU/cam0 messages must be unchanged.

The cache transformation is only:

```text
topic subset + new bag compression/container
```

not message transformation.

### G-P14.CACHE1

For each Oxford sequence produce a compact cache manifest:

```text
source_bag
source_bag_sha256
cache_bag
cache_bag_sha256
source_size
cache_size
source_compression
cache_compression

lidar_topic
lidar_count
lidar_payload_parity

imu_topic
imu_count
imu_payload_parity

cam_topic
cam_count
cam_payload_parity

ordering_parity
timestamp_parity
```

All parity fields must PASS.

---

## 10. Estimator parity

For each Oxford sequence perform one bounded smoke proving:

```text
original bz2 bag
vs
persistent *_LIVO.bag
```

produces the same LIO estimator input/result.

At minimum use B0; P4 parity can be reused where already proven if implementation is unchanged.

Require:
- same LiDAR/IMU sequence;
- equal trajectory rows/timestamps;
- byte-identical trajectory when deterministic, otherwise strict numerical identity;
- same primary metric.

The camera stream is present in the cache but ignored in current LIO runs.

### G-P14.CACHE2

Report:
```text
original-bag smoke runtime
LIVO-cache smoke runtime
speedup
trajectory parity
```

Once parity is proven, all six Oxford ablation variants must use the persistent `_LIVO.bag`.

---

# PART E — Existing valid active rows

Do not rerun unless required for a cache migration/parity smoke:

### NTU
```text
eee_01
nya_01
sbs_01
```

### Oxford
```text
Quarter_01
```

However, `Quarter_01` must now receive a persistent:

```text
Quarter_01_LIVO.bag
```

and cache parity evidence.

Its already-valid six metrics may be reused if the new cache is proven trajectory-identical to the old accepted transport cache/source.

No need to rerun all six Quarter_01 cells solely because the cache now additionally contains cam0.

---

# PART F — New NTU sequences

## 11. Targets

```text
eee_02
eee_03
nya_02
nya_03
sbs_02
sbs_03
```

Before each sequence:
- bag/hash;
- topics/types/counts/duration;
- calibration;
- Leica/GT semantics;
- evaluator;
- effective config;
- extrinsic semantics.

Reuse the accepted NTU VIRAL evaluator only after proving the same GT/frame/time contract.

---

# PART G — New Oxford sequences

## 12. Targets

```text
Church_05
College_03
Palace_01
```

Expected files:

```text
bag/OXFORD/Church_05/Church_05.bag
bag/OXFORD/Church_05/gt-tum.txt

bag/OXFORD/College_03/College_03.bag
bag/OXFORD/College_03/gt-tum.txt

bag/OXFORD/Palace_01/Palace_01.bag
bag/OXFORD/Palace_01/gt-tum.txt
```

Shared calibration authority:

```text
bag/OXFORD/Calibration/
```

Verify the same Oxford calibration/GT semantics independently for each sequence.

For each, first build and validate:

```text
<Sequence>_LIVO.bag
```

then run experiments from that cache.

---

# PART H — Frozen six variants

## 13. Variants

Run exactly:

```text
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
```

### B0
```text
fixed_1000
Super legacy association
probability covariance inactive in estimator
```

### P4-LC
```text
cov_enable=ON
cov_validation_mode=light
map_pose_cov_model=livo2_compat
map_cov_storage_precision=double
qr_plane_cov_enable=ON
p2p_weight_mode=prob_livo2
association_mode=super_legacy
prob_assoc_shadow_enable=OFF
```

### P4-RC
Only:
```text
map_pose_cov_model=super_right_consistent
```

### P5-ACTIVE
Map side fixed to P4-LC:
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
Only:
```text
association_pose_cov_model=super_right_consistent
```

No new variants.

---

# PART I — Exact A/B isolation

## 14. G-P14.AB

Programmatically diff effective configs.

Require:

```text
P4-LC → P4-RC:
  only map_pose_cov_model

P5-ACTIVE → P5-SENSOR-CORR:
  only association_sensor_cov_model

P5-SENSOR-CORR → P5-BOTH-CORR:
  only association_pose_cov_model
```

No dataset parameter may change after seeing metrics.

---

# PART J — NTU execution

## 15. Order

```text
eee_02
eee_03
nya_02
nya_03
sbs_02
sbs_03
```

Run all six variants for each.

If accepted calibration gives:

```text
R_LI = I
```

require:

```text
P5-ACTIVE == P5-SENSOR-CORR
```

trajectory-identical/numerically identical.

Unexpected difference is a regression signal, not a tuning opportunity.

### G-P14.NTU

All 36 new NTU cells require:
- clean committed source;
- explicit `dept_err=0.02`, `beam_err=0.01`;
- accepted evaluator;
- exact A/B isolation;
- heavy debug OFF;
- manifest;
- effective config;
- trajectory hash;
- metric/unit;
- runtime/classification.

---

# PART K — Oxford execution

## 16. Order

For each:

```text
Church_05
College_03
Palace_01
```

perform:

```text
source bag preflight
→ persistent LIVO cache generation
→ cache message parity
→ estimator parity smoke
→ 6 variants using *_LIVO.bag
```

All Oxford runs use explicit:

```text
dept_err=0.05
beam_err=0.02
```

and the accepted Oxford evaluator.

### G-P14.OX

All 18 new Oxford cells require:
- official Oxford covariance semantics;
- verified GT/evaluator;
- valid persistent LIVO cache;
- cache parity;
- clean source;
- exact A/B isolation;
- heavy diagnostics OFF;
- compact evidence;
- metric/unit/runtime/classification.

---

# PART L — Production diagnostics stay light

## 17. Freeze

Every canonical run:

```text
cov_validation_mode=light
prob_assoc_shadow_enable=false
full eigensolver diagnostics=false
per-point dump=false
per-candidate dump=false
FD diagnostics=false
sanitizers=false
heavy profiler=false
verbose debug=false
```

If abnormal behavior occurs:
- create a separate minimal diagnostic run;
- do not use diagnostic run as canonical evidence.

---

# PART M — Git evidence hygiene

## 18. Track only lightweight evidence

Track:

```text
run_manifest
evaluation result
effective config
variant diff
cache manifest
hashes
small summary
```

Do NOT track:

```text
*.bag
*_LIVO.bag
node.log
roscore/roslaunch logs
duplicate GT
large trajectories by default
raw dumps
build/devel
stdout/stderr logs
```

The persistent `_LIVO.bag` lives under:

```text
~/super_livo/bag/OXFORD/<Sequence>/
```

outside the Super-LIO Git repository.

---

# PART N — Metadata

## 19. Every new run

Record:

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
trajectory_rows
matched_rows where applicable
runtime
primary_metric
unit
classification
```

For Oxford runs, the canonical input bag is now the cache:

```text
input_bag = <Sequence>_LIVO.bag
input_bag_sha256 = ...
source_original_bag = <Sequence>.bag
source_original_bag_sha256 = ...
cache_parity_manifest = ...
```

This makes provenance explicit.

---

# PART O — Active ablation document

## 20. Update `spec/prob_lio/ABLATION_MATRIX.md`

Active rows:

### NTU
```text
eee_01
eee_02
eee_03
nya_01
nya_02
nya_03
sbs_01
sbs_02
sbs_03
```

### Oxford
```text
Quarter_01
Church_05
College_03
Palace_01
```

Each row contains:

```text
GT type
primary metric
evaluator
covariance/config authority
canonical input artifact
B0
P4-LC
P4-RC
P5-ACTIVE
P5-SENSOR-CORR
P5-BOTH-CORR
status/notes
```

For Oxford, canonical input artifact should explicitly identify:

```text
*_LIVO.bag
```

and its cache manifest/hash.

MCD/M3DGR move/remain under:

```text
Historical / Out-of-Active-Scope
```

Do not include them in active summaries.

---

# PART P — Descriptive summaries

## 21. Active family summaries

Report per family:

```text
sequence count
P4-LC better/worse/equal vs B0
median absolute delta
median relative delta
P4-RC better/worse/equal vs P4-LC
P5-SENSOR-CORR better/worse/equal vs P5-ACTIVE
```

Do not tune parameters from these results.

Do not average incompatible metric semantics across families unless explicitly proven compatible.

---

# PART Q — Execution order

## 22. Recommended order

```text
0. state consensus + prompt registration + scope docs
1. verify official Oxford cam0 authority
2. build/validate Quarter_01_LIVO.bag
3. preflight all 6 new NTU sequences
4. eee_02 six variants
5. eee_03 six variants
6. nya_02 six variants
7. nya_03 six variants
8. sbs_02 six variants
9. sbs_03 six variants
10. preflight Church_05
11. build/validate Church_05_LIVO.bag
12. Church_05 six variants
13. preflight College_03
14. build/validate College_03_LIVO.bag
15. College_03 six variants
16. preflight Palace_01
17. build/validate Palace_01_LIVO.bag
18. Palace_01 six variants
19. aggregate/update matrix/docs
20. commit/push
21. STOP
```

One bounded experiment at a time.
No duplicate estimator/rosbag processes.

---

# PART R — Hard gates

## 23. Required gates

```text
G-P14.SCOPE  active benchmark scope = NTU + Oxford only
G-P14.COV    frozen covariance authority
G-P14.CAM    Oxford official LIVO camera = cam0
G-P14.CACHE1 persistent LIVO cache message parity
G-P14.CACHE2 estimator parity + speedup
G-P14.AB     exact six-variant isolation
G-P14.NTU    36 new NTU cells
G-P14.OX     18 new Oxford cells
G-P14.DBG    heavy diagnostics OFF
G-P14.EVID   lightweight Git evidence
G-P14.CODE   stable production_code_oid
G-P14.MAT    complete active matrix
```

Each gate needs concrete evidence, not prose-only PASS.

---

# PART S — Commit policy

Suggested commits:

```text
docs(prob-lio): freeze active benchmark scope to ntu and oxford
tools(prob-lio): persist oxford livo cache workflow
evidence(prob-lio): add prompt14 ntu ablations
evidence(prob-lio): add prompt14 oxford ablations
docs(prob-lio): update active ablation matrix
```

No algorithm-math change expected.
No force push.

---

# 24. Final report format

## State
- starting HEAD
- final HEAD
- branch/origin
- clean worktree
- prompt registration

## Active Scope
```text
NTU ACTIVE
Oxford ACTIVE
MCD HISTORICAL
M3DGR HISTORICAL
```

## Covariance Authorities
NTU:
```text
0.02 m / 0.01 deg
```

Oxford:
```text
0.05 m / 0.02 deg
```

## Oxford Camera Authority
- official FAST-LIVO2 config source/commit
- `img_topic=/alphasense_driver_ros/cam0/color/image`
- source bag cam0 compressed topic
- future adapter mapping contract

## Persistent Oxford LIVO Caches

For:
```text
Quarter_01_LIVO.bag
Church_05_LIVO.bag
College_03_LIVO.bag
Palace_01_LIVO.bag
```

report:
- source/cache path
- source/cache SHA256
- compression
- size
- lidar/imu/cam0 counts
- payload/order/timestamp parity
- estimator parity
- runtime speedup

Confirm cam1/cam2 are absent.

## NTU Results
For six new sequences:
- preflight
- six metrics
- pairwise deltas

## Oxford Results
For three new sequences:
- preflight
- six metrics
- pairwise deltas

## Active Matrix Summary
Expected:
```text
13 active sequences
78 active six-variant cells
24 reused existing cells
54 new Prompt14 cells
```

Report valid/failed/blocked counts and descriptive statistics.

## Historical Scope
Confirm MCD/M3 retained but excluded from active aggregate.

## Instrumentation/Evidence
- heavy debug OFF
- no bag/log/large trajectory committed
- `_LIVO.bag` files persist under bag/OXFORD sequence dirs only

## Gates
PASS/FAIL with evidence.

## Commits
- SHA list
- final HEAD
- origin HEAD
- clean worktree
- no force push

## Stop
Return to Owner.
Do not start stochastic noise calibration or parameter tuning automatically.
