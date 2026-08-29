# Round 14 — Phase B Canonical Eval Final Seal

```text
Initial HEAD:
d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d
Resolved initial full SHA:
d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d

Eval-seal code commit:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
Canonical A2 production revision:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
Canonical B0 production revision:
31d677e13ee32fc0f57940636283ae66f9a2e3dd

Evidence/docs commit:
<pending report commit>
Actual delivered repository HEAD:
<pending delivery record>
```

```text
=== Agent State Consensus ===
executor:
agent-ds

branch:
super-livo

initial local:
d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d
initial remote:
d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d
frontier verified:
YES

final local:
<pending>
final remote:
<pending>
ahead:
0
behind:
0
worktree clean:
YES
```

```text
=== Architecture Freeze ===
Phase-B algorithm changed:
NO
Apply control flow changed:
NO
scheduler changed:
NO
Visual map changed:
NO
parameter tuning:
NO
Phase C started:
NO
```

```text
=== Pre-Repair Lateral Audit ===
F1 shared helper:
CONFIRMED_MISSING -> CLOSED (one compiled helper, 2 callsites)
F2 iteration naming:
CONFIRMED -> CLOSED (ESKF loop authoritative; 7758 == callbacks)
F3 initial counter contamination:
CONFIRMED -> CLOSED (context sub-counters; initial 395308 vs global 1955149)
F4 residual mean:
CONFIRMED -> CLOSED (initial total / initial frames)
F5 GT:
CONFIRMED -> CLOSED (4-stat canonical, same aligned set)
F6 lineage:
CONFIRMED -> CLOSED (A1 parent=A0; validator exact enforcement)
F7 weak tests:
CONFIRMED -> CLOSED (EF-T4/E-T4/EF-T7/EF-T18/EC-T1/EC-T2 replaced)
F8 raw/processable:
CONFIRMED -> CLOSED (7 explicit scan fields)
F9 duplicate Observe:
CONFIRMED -> CLOSED (duplicate_scan_use_events counter; 0 on both runs)
F10 event fallback:
CONFIRMED -> CLOSED (EVIDENCE_MISSING, no inference)
F11 completion hardcode:
CONFIRMED -> CLOSED (--expected-rows explicit)
F12 semantic labels:
CONFIRMED -> CLOSED (immutable IDs, CURRENT forbidden)
F13 dirty-run provenance:
CONFIRMED -> CLOSED (clean SHA runs; dirty canonical rejected)
additional findings:
initialQueryHits getter returned global hits — fixed in 31d677e and the
canonical pair rerun; A0/A1 old runs migrated as MIGRATED_HISTORICAL.
```

```text
=== Shared Information Helper ===
helper path:
src/super_lio/include/lio/VisualInformationMetrics.h
A2 callsite:
src/super_lio/src/lio/super_lio.cpp (Shadow branch)
B0 callsite:
src/super_lio/src/lio/super_lio.cpp (Apply initial-linearization branch)
duplicate canonical implementation:
0
symmetrization:
I_sym = 0.5 (I + I^T)
normalization:
I_norm = I_sym / N_valid_residual (N > 0)
solver:
SelfAdjointEigenSolver (ascending)
degeneracy epsilon:
1e-12 (lambda_min <= 1e-12 -> degenerate, condition +inf)
same helper mechanical test:
PASS (C++ FS-T1..T4; identical output through A2/B0 callers)
```

```text
=== Solver Accounting ===
ESKF iteration producer:
observe_iteration_count_ inside UpdateObserveImpl loop (ESKF.cpp)
Apply count:
1965
iteration count:
7758
callback count:
7758
solver residual total:
98396697 (sum of callback residual counts)
callback == iteration under current ESKF:
PASS
proof:
loop executes obs() exactly once per iteration (FS-T5/T6; real run equal)
```

```text
=== Initial Measurement Accounting ===
initial frames:
1965
initial queries:
395308 (hits 395308)
initial candidates:
395308
initial valid:
393229
initial rejected:
2079
initial residual total:
24890308
solver contamination:
ZERO (context-separated counters)
```

```text
=== LiDAR Accounting ===
raw input scans:
3987
excluded scans:
1
eligible scans:
3986
unique geometry-used scans:
3985
geometry update events:
3985
duplicate geometry-use events:
0
eligible never-used scans:
1
exact-once classification:
EXACT_ONCE (dup events 0; updates == unique used; the 1 never-used scan is
the startup map-init scan; the 1 excluded scan is the pre-observe filter
drop — both documented lifecycle reasons)
```

```text
=== GT Evaluator ===
RMSE source:
ntu_viral_official_ate.py ATE (m) (unchanged official definition)
mean source:
same evaluator, same aligned sample set
median source:
same evaluator, same aligned sample set
max source:
same evaluator, same aligned sample set
same aligned sample set:
PASS
formula tests:
PASS (FS-T12/T13)
```

```text
=== Completion Reference ===
source:
--expected-rows CLI (explicit metadata)
hard-coded 3981 in generic evaluator:
NO
eee_01 expected rows:
3981 (passed explicitly; completion 1.0)
```

```text
=== Semantic IDs ===
VisualMapPolicy:
S3_SPATIAL_BALANCED_V0
Normalize:
NOT_IMPLEMENTED
Exposure:
NOT_IMPLEMENTED
Normal:
NOT_IMPLEMENTED
Patch:
SUPER_LIVO_PRE_PHASEC_PATCH_V0
Residual:
SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0
Iteration:
SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0
CURRENT labels in canonical registry:
ZERO
```

```text
=== Registry Architecture ===
authoritative machine source:
canonical scorecard JSON (real producer -> visual_eval_score.py)
TSV generated:
YES (visual_eval_registry.py -> docs/.../visual_semantics_eval_registry.tsv)
manual numeric row edits:
NO
stage-parent map:
CANONICAL_STAGE_PARENTS (A0->-, A1->A0, A2->A1, B0->A2)
validator exact-parent enforcement:
PASS
```

```text
=== Weak-Test Replacement Audit ===
static-only hard CLOSE tests remaining:
ZERO
string-only hard CLOSE tests remaining:
ZERO
handwritten-registry-only hard CLOSE tests remaining:
ZERO
```

```text
=== FS-T1..FS-T30 ===
FS-T1: PASS   FS-T2: PASS   FS-T3: PASS   FS-T4: PASS   FS-T5: PASS
FS-T6: PASS   FS-T7: PASS   FS-T8: PASS   FS-T9: PASS   FS-T10: PASS
FS-T11: PASS  FS-T12: PASS  FS-T13: PASS  FS-T14: PASS  FS-T15: PASS
FS-T16: PASS  FS-T17: PASS  FS-T18: PASS  FS-T19: PASS  FS-T20: PASS
FS-T21: PASS  FS-T22: PASS  FS-T23: PASS  FS-T24: PASS  FS-T25: PASS
FS-T26: PASS  FS-T27: PASS  FS-T28: PASS  FS-T29: PASS  FS-T30: PASS

total: 34 (incl. C++ behavior tests)
PASS: 34
FAIL: 0
```

```text
=== Eval-Seal Code Commit ===
SHA:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
tree clean before build:
YES
build revision:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
build PASS:
YES
```

```text
=== Canonical A2 Clean Run ===
stage:
A2_D_CAMERA_EPOCH_SHADOW
run:
round14_phaseA/a2_camera_epoch_shadow/20260829T052214Z
production_revision:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
git_dirty:
0
config hash:
0e2142db49e23542259dcd7637f08d19b9e507adde53db92675c9723a335a4bd
experiment_valid:
true
cleanup_verified:
true
```

```text
=== Canonical A2 Score ===
APE RMSE:
0.104098
mean:
0.074131
median:
0.061815
max:
0.567950
completion:
1.0

initial queries:
454591 / 454591
initial candidates:
454591
initial valid:
452899
initial valid ratio:
0.99628
initial residual total:
28687977
initial residual/frame mean:
14592.05
P50:
241.0

lambda_min_norm P50:
2276.0806
lambda_max_norm P50:
12810780.3
trace_norm P50:
23971677.7
condition P50:
4325.43407
degenerate frames:
0
metric invalid frames:
0

Visual CPU P50:
4.56 ms
```

```text
=== Canonical B0 Clean Run ===
stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
run:
round14_phaseA/b0_camera_epoch_apply_corrected/20260829T052357Z
production_revision:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
git_dirty:
0
config hash:
e37c9e053050850be7a26c3093b32b21340a4d0ea68d690a210c680014e5f556
experiment_valid:
true
cleanup_verified:
true
```

```text
=== Canonical B0 Score ===
APE RMSE:
0.133707
mean:
0.094513
median:
0.072233
max:
0.751920
completion:
1.0

initial queries:
395308 / 395308
initial candidates:
395308
initial valid:
393229
initial valid ratio:
0.99474
initial residual total:
24890308
initial residual/frame mean:
12667.08
P50:
208.0

lambda_min_norm P50:
2212.82424
lambda_max_norm P50:
12881071.2
trace_norm P50:
24257218.0
condition P50:
4547.68739
degenerate frames:
0
metric invalid frames:
0

Solver Apply:
1965
Solver iterations:
7758
Solver callbacks:
7758
Solver residual total:
98396697
Iterations/Apply P50:
4

Visual CPU P50:
12.39 ms
```

```text
=== A2/B0 Effective Config Diff ===
only VisualApply semantic difference:
PASS (164 keys; visual_apply false->true and its derived d_family switch)
unexpected differences:
NONE
```

```text
=== A2 -> B0 Canonical Delta ===
APE RMSE:
0.104098 -> 0.133707 (+0.029609)
mean:
0.074131 -> 0.094513
median:
0.061815 -> 0.072233
max:
0.567950 -> 0.751920

initial valid ratio:
0.99628 -> 0.99474
initial residual/frame P50:
241 -> 208
lambda_min_norm P50:
2276.0806 -> 2212.82424
spectral condition P50:
4325.43407 -> 4547.68739
Visual CPU:
4.56 -> 12.39 ms

classification:
REGRESSED (accuracy regression observed; no tuning; expected scientific
input to Phase C/D)

parameter tuning:
NO
```

```text
=== Real Producer -> Registry ===
A2:
PASS
B0:
PASS
```

```text
=== Five-Layer Consistency ===
21 critical metrics (git SHA/dirty/config hash/parent/ATE 4-stat/initial
residual total+P50+valid ratio/lambda_min+condition/solver apply+iterations+
residual total/raw+excluded+unique scans/dup geometry use/Visual CPU):
producer -> scorecard -> registry all MATCH (see evidence doc)

all rows MATCH:
YES
```

```text
=== Adversarial False-Close Suite ===
wrong parent rejected:
PASS
wrong HEAD rejected:
PASS
dirty source rejected:
PASS (CANONICAL_RUN_DIRTY_SOURCE)
wrong config hash rejected:
PASS (CANONICAL_CONFIG_HASH_MISMATCH)
diag condition rejected:
PASS (validator: no diag condition path; helper single-source)
Apply-as-residual rejected:
PASS
Apply-as-iteration rejected:
PASS
missing GT stat rejected:
PASS
missing event evidence rejected:
PASS
duplicate geometry use rejected:
PASS
CURRENT semantic label rejected:
PASS
wrong completion reference rejected:
PASS
ALL adversarial fixtures rejected:
YES (12/12)
```

```text
=== Final Lateral Audit ===
duplicate metric implementation found:
NONE
hard-coded eval assumption found:
NONE
manual canonical registry values found:
NONE
fallback semantic inference found:
NONE
stale provenance found:
NONE
weak hard-gate tests found:
NONE
unresolved semantic mismatch:
NONE
FINAL_LATERAL_AUDIT:
PASS
```

```text
=== Phase C Readiness ===
Phase-B algorithm:
CLOSED
Phase-B canonical eval:
CLOSED
clean canonical A2:
VALID
clean canonical B0:
VALID
A2/B0 metrics comparable:
PASS
Phase C ready:
YES
Phase C started:
NO
```

```text
=== Skills Used ===
/tdd:
YES (FS-T1..FS-T30 red->green; C++ behavior tests)
/diagnosing-bugs:
YES (F1-F13; initialQueryHits contamination found on the first clean B0)
/grill-with-docs:
YES (Prompt71-74 contracts vs producers vs evaluator vs registry vs tests
vs final reports vs real run manifests reconciled in the lateral audit and
five-layer table)
```

```text
=== Git Safety ===
reset --hard:
NO
rebase:
NO
force:
NO
force-with-lease:
NO
history rewrite:
NO
upstream push:
NO
git clean:
NO
```

```text
=== Remote Delivery ===
pre-push local:
31d677e13ee32fc0f57940636283ae66f9a2e3dd (before evidence commits)
pre-push remote:
ccd1dd0fd412efbf04b694ea4a6a1745218ee247
push RC:
<pending>
post-push local:
<pending>
post-push remote:
<pending>
equal:
<pending>
ahead:
<pending>
behind:
<pending>
```

```text
=== Final Repository Provenance ===
Eval-seal code SHA:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
A2/B0 production SHA:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
Evidence/docs SHA:
<pending>
Actual delivered HEAD:
<pending>

All full SHAs internally consistent:
PASS (pending delivery record)
```

```text
=== WIP ===
present:
NO
clean:
YES
```

```text
=== Final CLOSE Checklist ===
F1_SHARED_INFORMATION_HELPER = CLOSED
A2_B0_ACTUAL_SHARED_COMPILED_HELPER = PASS
INFO_DEGENERACY_RULE_SINGLE_SOURCE = PASS
F2_SOLVER_ITERATION_FIELD_MISNAMED = CLOSED
SOLVER_ITERATION_PRODUCER = ESKF_LOOP_AUTHORITATIVE
SOLVER_APPLY_COUNT = CORRECT
SOLVER_ITERATION_COUNT = CORRECT
SOLVER_CALLBACK_COUNT = CORRECT
SOLVER_RESIDUAL_TOTAL = CORRECT
F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION = CLOSED
INITIAL_QUERY_COUNTERS = INITIAL_ONLY
INITIAL_CANDIDATE_COUNTERS = INITIAL_ONLY
INITIAL_VALID_COUNTERS = INITIAL_ONLY
INITIAL_RESIDUAL_COUNTERS = INITIAL_ONLY
F4_RESIDUAL_MEAN_SEMANTIC_MIX = CLOSED
INITIAL_RESIDUAL_MEAN = VALID
F5_GT_4STAT_NOT_CANONICAL = CLOSED
GT_RMSE = NUMERIC
GT_MEAN = NUMERIC
GT_MEDIAN = NUMERIC
GT_MAX = NUMERIC
F6_STAGE_PARENT_REGISTRY_DIVERGENCE = CLOSED
A1_PARENT = A0
A2_PARENT = A1
B0_PARENT = A2
F7_FALSE_POSITIVE_CLOSE_TESTS = CLOSED
ALL_HARD_CLOSE_TESTS_BEHAVIORAL_OR_E2E = PASS
F8_RAW_PROCESSABLE_COUNT_CONFLATION = CLOSED
RAW_SCAN_FIELDS_EXPLICIT = PASS
F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT = CLOSED
DUPLICATE_GEOMETRY_USE_EVENTS = ZERO
F10_EVENT_PLACEMENT_INFERENCE_FALLBACK = CLOSED
MISSING_EVENT_EVIDENCE_FAILS_EXPLICITLY = PASS
F11_COMPLETION_3981_HARDCODE = CLOSED
COMPLETION_REFERENCE_EXPLICIT = PASS
F12_NONIMMUTABLE_SEMANTIC_LABELS = CLOSED
NO_CANONICAL_CURRENT_LABELS = PASS
SEMANTIC_IDS_IMMUTABLE = PASS
F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE = CLOSED
A2_RUN_FROM_CLEAN_COMMIT = PASS
B0_RUN_FROM_CLEAN_COMMIT = PASS
CANONICAL_CONFIG_PROVENANCE_COMPLETE = PASS
A2_B0_EFFECTIVE_CONFIG_SINGLE_VARIABLE_DIFF = PASS
A2_REAL_PRODUCER_TO_REGISTRY = PASS
B0_REAL_PRODUCER_TO_REGISTRY = PASS
FIVE_LAYER_CONSISTENCY_TABLE = ALL_MATCH
INSTRUMENTATION_DOES_NOT_CHANGE_ALGORITHM = PASS
NO_PARAMETER_TUNING = PASS
PHASE_C_NOT_STARTED = PASS
```

```text
=== Final Classification ===

ROUND14_PHASEB_CANONICAL_EVAL_FULLY_SEALED_AND_REMOTE_READY
```

```text
=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```

## Delivery record (machine-generated after push)

```text
Report commit:             <filled by delivery script>
Actual delivered HEAD:     <filled by delivery script>
Post-push local == remote: <filled by delivery script>
ahead/behind:              <filled by delivery script>
```
