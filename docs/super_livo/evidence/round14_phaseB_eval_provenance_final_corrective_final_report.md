# Round 14 — Phase B Eval Provenance Final Corrective

```text
Initial HEAD:
c8f9ac21d4ce104b7babded988f98a713535b4cf
Functional corrective commit:
b1434fb0095a60bcdec64effbf57619a8f0043a4
Evidence/report parent commit:
8da2daf719d02a5f04642e5489b0b14ca345bc63
Actual delivered remote HEAD:
8da2daf719d02a5f04642e5489b0b14ca345bc63
```

```text
=== Agent State Consensus ===
executor:
agent-ds

expected:
c8f9ac21d4ce104b7babded988f98a713535b4cf

actual initial:
c8f9ac21d4ce104b7babded988f98a713535b4cf
branch:
super-livo
worktree:
/home/lc/super_livo/src/Super-LIO
origin:
https://github.com/Scar-c/Super-LIO.git
frontier verified:
YES (HEAD == origin == expected, tree clean except loose Prompt76)
```

```text
=== Architecture Freeze ===
Phase-B algorithm changed:
NO
scheduler changed:
NO
Apply changed:
NO
payload changed:
NO
ESKF numerical semantics changed:
NO
Visual map changed:
NO
parameter tuning:
NO
bag rerun:
NO
Phase C started:
NO
```

```text
=== Starting Defects ===
P1 semantic snapshot evaluator authority:
CONFIRMED -> CLOSED (explicit semantic_snapshot_v0.yaml file source)
P2 registry stage semantic inference:
CONFIRMED -> CLOSED (registry columns from scorecard actual semantics)
P3 event validator incomplete:
CONFIRMED -> CLOSED (§42/§43 hard gates enforced)
P4 stale FS-T28:
CONFIRMED -> CLOSED (clean canonical 052357Z artifact)
additional provenance findings:
A0/A1 legacy manifests predate the policy-ID snapshot — migrated as
MIGRATED_HISTORICAL in LEGACY mode, never Phase-C parents.
```

```text
=== Run Semantic Authority ===
authoritative source:
resolved_experiment_semantics.yaml + semantic_snapshot_v0.yaml
path:
scripts/super_livo/evaluation/semantic_snapshot_v0.yaml
hash:
187b8dc6ed057fb3c90b930878f1b1e11c5b2e1c44001d8c7a9fdcb467fafbab
actual-vs-expected separated:
PASS (scorecard.actual_semantics vs expected_stage_semantics)
```

```text
=== Canonical A2 Actual Semantics ===
semantic profile:
D_VISUAL_SHADOW
VisualEvent:
CAMERA_EPOCH
timestamp semantics:
CAMERA_EPOCH_PROPAGATED_STATE
VisualApply:
false
Apply connectivity:
NOT_ESTABLISHED
payload ownership:
RETAIN_THROUGH_MEASUREMENT
VisualMapPolicy:
S3_SPATIAL_BALANCED_V0
Normalize:
NOT_IMPLEMENTED_V0
Exposure:
NOT_IMPLEMENTED_V0
Normal:
NOT_IMPLEMENTED_V0
Patch:
SUPER_LIVO_PRE_PHASEC_PATCH_V0
Residual:
SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0
Iteration:
SUPER_LIVO_PRE_PHASEC_ITERATION_V0
semantic provenance:
PASS (complete=true, CANONICAL mode)
```

```text
=== Canonical B0 Actual Semantics ===
semantic profile:
D_VISUAL_APPLY
VisualEvent:
CAMERA_EPOCH
timestamp semantics:
CAMERA_EPOCH_PROPAGATED_STATE
VisualApply:
true
Apply connectivity:
ESTABLISHED
payload ownership:
RETAIN_THROUGH_MEASUREMENT
VisualMapPolicy:
S3_SPATIAL_BALANCED_V0
Normalize:
NOT_IMPLEMENTED_V0
Exposure:
NOT_IMPLEMENTED_V0
Normal:
NOT_IMPLEMENTED_V0
Patch:
SUPER_LIVO_PRE_PHASEC_PATCH_V0
Residual:
SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0
Iteration:
SUPER_LIVO_PRE_PHASEC_ITERATION_V0
semantic provenance:
PASS (complete=true, CANONICAL mode)
```

```text
=== Expected Stage Contracts ===
A2:
event CAMERA_EPOCH / ts CAMERA_EPOCH_PROPAGATED_STATE / apply false /
connectivity NOT_ESTABLISHED / payload RETAIN_THROUGH_MEASUREMENT
B0:
same except apply true / connectivity ESTABLISHED
actual == expected:
PASS (both runs; mismatches raise CANONICAL_STAGE_SEMANTIC_MISMATCH)
```

```text
=== Registry Generator ===
semantic values from scorecard:
YES
stage-based VisualApply inference:
ZERO
stage-based VisualEvent inference:
ZERO
stage-based VisualMapPolicy inference:
ZERO
```

```text
=== Validator Hardening ===
A2:
CameraEvent >0: PASS (1966)
LiDAR callback =0: PASS
duplicate=0: PASS
payload missing=0: PASS
early release=0: PASS
Apply=0: PASS
B0:
CameraEvent >0: PASS (1966)
LiDAR callback=0: PASS
duplicate=0: PASS
payload missing=0: PASS
early release=0: PASS
Apply>0: PASS (1965)
attempt identity: PASS (1965 == 1965 + 0)
iteration==callback: PASS (7758 == 7758); iteration != apply: PASS
```

```text
=== Stale Test Audit ===
FS-T28 before:
040348Z artifact — SolverIterationCount 1965 (old Apply-count semantics)
after:
052357Z clean canonical artifact — Apply 1965 / Iteration 7758 /
Callbacks 7758 / iteration==callback / iteration!=apply
historical positive fixtures remaining:
ZERO
```

```text
=== PS-T1..PS-T20 ===
PS-T1: PASS   PS-T2: PASS   PS-T3: PASS   PS-T4: PASS   PS-T5: PASS
PS-T6: PASS   PS-T7: PASS   PS-T8: PASS   PS-T9: PASS   PS-T10: PASS
PS-T11: PASS  PS-T12: PASS  PS-T13: PASS  PS-T14: PASS  PS-T15: PASS
PS-T16: PASS  PS-T17: PASS  PS-T18: PASS  PS-T19: PASS  PS-T20: PASS

PASS: 20
FAIL: 0
```

```text
=== AFC-T1..AFC-T20 ===
AFC-T1..AFC-T20: all REJECTED (wrong parent/HEAD/config-hash/apply/event/
timestamp/map-policy/patch/residual; lidar-callback>0; duplicate event;
payload missing/early-release; A2 apply>0; B0 apply=0; apply identity fail;
missing GT stat; apply-as-iteration; historical invalid B0)

20/20 rejected:
YES
```

```text
=== A2 Real Artifact E2E ===
artifact:
20260829T052214Z
estimator rerun:
NO
semantic source:
resolved_experiment_semantics.yaml + semantic_snapshot_v0.yaml
scorecard:
out/visual_eval_score.json (actual_semantics complete)
validator:
CANONICAL_SCORECARD_VALID
registry:
A2_D_CAMERA_EPOCH_SHADOW row VALID (parent A1)
A2_PROVENANCE_E2E:
PASS
```

```text
=== B0 Real Artifact E2E ===
artifact:
20260829T052357Z
estimator rerun:
NO
semantic source:
resolved_experiment_semantics.yaml + semantic_snapshot_v0.yaml
scorecard:
out/visual_eval_score.json (actual_semantics complete)
validator:
CANONICAL_SCORECARD_VALID
registry:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED row VALID (parent A2)
B0_PROVENANCE_E2E:
PASS
```

```text
=== Canonical Numeric Regression Check ===
A2 RMSE:
0.104098 (unchanged)
mean:
0.074131
median:
0.061815
max:
0.567950
B0 RMSE:
0.133707 (unchanged)
mean:
0.094513
median:
0.072233
max:
0.751920
A2 λmin_norm P50:
2276.0806
B0 λmin_norm P50:
2212.82424
A2 cond P50:
4325.43407
B0 cond P50:
4547.68739
B0 Apply:
1965
B0 iterations:
7758
B0 callbacks:
7758
B0 iterations/apply P50:
4
numeric results changed unexpectedly:
NO
```

```text
=== Five-Layer Semantic Consistency ===
(13 fields x A2/B0: profile/event/timestamp/apply/connectivity/payload/
map/normalize/exposure/normal/patch/residual/iteration — resolved run ->
scorecard actual_semantics -> registry -> validator contract -> report)
ALL_MATCH:
YES
```

```text
=== Five-Layer Event Consistency ===
(8 fields x B0: camera-event 1966 / lidar-callback 0 / duplicate 0 /
payload missing 0 / early release 0 / apply attempts 1965 / success 1965 /
failures 0 — producer -> scorecard -> registry -> validator -> report)
ALL_MATCH:
YES
```

```text
=== Historical Invalid Evidence ===
first invalid B0 canonical:
NO
old provenance-mixed B0 canonical:
NO
used as Phase C parent:
NO
```

```text
=== Test Evidence Quality ===
production-helper tests:
FS-T1..T7/T21 (C++ helper + ESKF loop + ownership + evidence)
generator behavior tests:
FS-T14..T19/T27, PS-T8..T10
real artifact E2E tests:
FS-T28, PS-T1/T2/T3/T4/T5/T6/T7/T17/T18/T20, A2/B0_PROVENANCE_E2E
adversarial validator tests:
AFC-T1..T20 + Prompt75 adversarial (12) + FS-T17/T26
static-only hard CLOSE tests:
ZERO
```

```text
=== Final Lateral Audit ===
stage semantic inference remaining:
NONE
semantic evaluator constants remaining:
NONE
semantic fallback defaults remaining:
NONE
manual registry semantics remaining:
NONE
validator missing zero-gates:
NONE
stale positive fixtures:
NONE
Apply-as-iteration confusion:
NONE
CURRENT labels:
NONE
unresolved provenance mismatch:
NONE
FINAL_LATERAL_AUDIT:
PASS
```

```text
=== Final CLOSE Checklist ===
P1_SEMANTIC_SNAPSHOT_EVALUATOR_AUTHORITY = CLOSED
RUN_SEMANTICS = AUTHORITATIVE
ACTUAL_VS_EXPECTED_SEMANTICS_SEPARATED = PASS
P2_REGISTRY_STAGE_SEMANTIC_INFERENCE = CLOSED
REGISTRY_SEMANTICS_FROM_SCORECARD = PASS
P3_EVENT_VALIDATOR_INCOMPLETE = CLOSED
A2_EVENT_ZERO_GATES = PASS
B0_EVENT_ZERO_GATES = PASS
P4_FS_T28_STALE_SEMANTICS = CLOSED
NO_HARD_CLOSE_TEST_USES_STALE_POSITIVE_ARTIFACT = PASS
SEMANTIC_PROVENANCE_COMPLETE = PASS
VISUAL_MAP_POLICY_FROM_RUN = PASS
PATCH_POLICY_FROM_RUN = PASS
RESIDUAL_POLICY_FROM_RUN = PASS
ITERATION_POLICY_FROM_RUN = PASS
NO_CANONICAL_CURRENT_LABELS = PASS
A2_PROVENANCE_E2E = PASS
B0_PROVENANCE_E2E = PASS
A2_VALIDATOR = PASS
B0_VALIDATOR = PASS
AFC_T1_T20 = 20/20 REJECTED
PS_T1_T20 = PASS
HISTORICAL_INVALID_B0_PARENT_REJECTION = PASS
FIVE_LAYER_SEMANTIC_CONSISTENCY = ALL_MATCH
FIVE_LAYER_EVENT_CONSISTENCY = ALL_MATCH
FINAL_LATERAL_AUDIT = PASS
NO_PHASE_B_ALGORITHM_CHANGE = PASS
NO_BAG_RERUN = PASS
NO_PARAMETER_TUNING = PASS
PHASE_C_NOT_STARTED = PASS
```

```text
=== Phase C Readiness ===
Phase-B algorithm:
CLOSED
Phase-B numeric results:
CLOSED
Phase-B canonical eval:
CLOSED
Phase-B semantic provenance:
CLOSED
Phase C ready:
YES
Phase C started:
NO
```

```text
=== Skills Used ===
/tdd:
YES (PS-T1..T20 red->green; AFC-T1..T20)
/diagnosing-bugs:
YES (P1-P4 reproduction; FS-T28 stale-semantics analysis)
/grill-with-docs:
YES (Prompt75 contract vs run manifests vs scorecard vs registry vs
validator vs FS/PS tests vs final report reconciled)
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
b1434fb0095a60bcdec64effbf57619a8f0043a4
pre-push remote:
5a4a4c6f14a807b3a014133876ebaa4dd2728234

push:
5a4a4c6..8da2daf (normal push, RC 0)

post-push local:
8da2daf719d02a5f04642e5489b0b14ca345bc63
post-push remote:
8da2daf719d02a5f04642e5489b0b14ca345bc63

equal:
YES
ahead:
0
behind:
0
```

```text
=== Delivery Provenance ===
Functional corrective commit:
b1434fb0095a60bcdec64effbf57619a8f0043a4
Evidence/report parent:
8da2daf719d02a5f04642e5489b0b14ca345bc63
Actual delivered remote HEAD:
8da2daf719d02a5f04642e5489b0b14ca345bc63
internally consistent:
PASS
```

```text
=== WIP ===
present:
NO
clean:
YES
```

```text
=== Final Classification ===

ROUND14_PHASEB_CANONICAL_EVAL_PROVENANCE_FULLY_CLOSED_AND_REMOTE_READY
```

```text
=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```
