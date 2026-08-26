# Super-LIVO Round 11W-P0R3 — Binary64 Boundary-Equivalence Closure + Resume Round11W
## Owner Numeric-Semantics Decision: Preserve ROS1 Production Double Time, Classify Sub-ULP Collisions Explicitly

**Input HEAD:**

```text
e8179ba
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

Parent task to resume after this narrow closure:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

---

# 0. OWNER DECISION

Round 11W-P0R2 stopped because the exact-ns oracle found:

```text
40 symmetric point-assignment mismatches
= 20 unique physical points

readiness mismatch:
0

exact-boundary mismatch:
0

all 20 unique mismatches:
exact point time is only 5–25 ns on the future side of camera tc

production:
binary64 absolute-second arithmetic collapses point_time and tc to the same
representable value, so <= classifies current

exact-ns oracle:
point_time_ns > tc_ns, so classifies pending
```

The Owner chooses:

```text
OPTION (a), WITH A STRICT REPRESENTATION-EQUIVALENCE CLASSIFIER
```

NOT:

```text
(b) broad exact-ns production timestamp rewrite
```

and NOT:

```text
(c) arbitrary "ignore <= 1 ULP" tolerance
```

Production S0 timestamp representation remains:

```text
ROS1 timestamps converted to binary64 seconds for estimator/slicer runtime
```

The exact-ns oracle remains the physical-time diagnostic reference.

The 20 sub-resolution cases may be accepted only if they satisfy the strict representation-collision proof in this prompt.

This is an explicit Owner numeric-semantics decision.

---

# 0.1 Why production is NOT being rewritten to exact ns now

The observed difference is:

```text
20 / 8,660,315 emitted points
≈ 2.31e-6
```

and every case lies only 5–25 ns across the camera boundary.

At epoch magnitude around:

```text
1.645e9 s
```

IEEE-754 binary64 spacing is approximately:

```text
238.418579 ns
```

not the previously reported approximate `370 ns`.

So 5–25 ns is still far below one binary64 spacing.

The previous report's `~370 ns ULP` value must be corrected in evidence.

The Owner does not authorize adding exact-ns fields throughout production LiDAR/IMU state structures solely for these 20 cases, because that would be a wider representation change with new memory/runtime/blast-radius risk.

---

# 0.2 Important: this is NOT permission to weaken the oracle

Do NOT simply change:

```text
mismatch != 0
```

to:

```text
ignore anything within 1 ULP
```

The comparator must distinguish:

```text
REPRESENTATION_COLLISION
```

from:

```text
SEMANTIC_MISMATCH
```

using the exact production arithmetic and exact ROS1 ns timestamps.

Only the first category is accepted.

---

# 1. ROS1-ONLY CONTRACT

This project environment is ROS1 / Noetic.

Use ROS1 semantics only.

Python exact timestamps:

```python
msg.header.stamp.to_nsec()
# genpy.Time secs/nsecs if a test fixture requires constructing a stamp
```

C++ ROS1 exact timestamp source where needed for audit:

```cpp
ros::Time::toNSec()
```

Do NOT add:
- ROS2 compatibility branches;
- `rclcpp`;
- ROS2 `sec/nanosec` abstractions;
- generic cross-ROS timestamp wrappers.

Keep this corrective ROS1-specific and small.

---

# 2. INDEPENDENT OWNER AUDIT FINDINGS TO PRESERVE

Actual pushed code at `e8179ba` confirms:

```text
true spanning LiDAR coverage helper implemented
all-arrival ready-drain centralized
FrozenS0ReferenceOracle exists
real-bag oracle now enters via exact to_nsec / integer Livox offsets
Day10:
  lost=0
  dup=0
  wrong_side=0
  overlap=0
  camera accounting exact
  readiness mismatch=0
B0 MD5 unchanged
```

Do NOT regress these.

Production `sliceLidarAt()` still uses binary64:

```cpp
abs_t = origin + offset_time;
if (abs_t <= t_c) current;
```

This is now the explicitly retained runtime representation.

---

# 3. PREEMPTION / WIP SAFETY

Pause resumed Round11W at a safe command boundary.

Before edits:

```bash
git status --short
git rev-parse HEAD
git diff --stat
git diff
```

If functional Round11W WIP exists:
- preserve it;
- record exact paths;
- SHA256 the starting diff;
- do not stash/reset automatically.

This P0R3 should primarily touch:
- comparison/audit tooling;
- tests;
- evidence/docs;
not estimator math.

---

# 4. PROMPT REGISTRATION

Expected next canonical prompt number:

```text
#43
```

Path:

```text
prompts/04_v1_implementation/43_round11w_p0r3_binary64_boundary_equivalence.md
```

Append P0R3 evidence to the active Round11W tracker:

```text
.scratch/super-livo-v1/issues/34-s0-lineage-accounting-ready-camera-drain.md
```

Do not create a new tracker unless repository convention requires it.

If prompt #43 already exists:

```text
STOP
report collision
do not silently renumber
```

---

# 5. FIX THE ULP EVIDENCE CALCULATION

Do not hard-code a decimal ULP estimate.

For each mismatch camera time `tc_double`, compute binary64 spacing from the actual runtime value.

Python:

```python
import math
ulp_s = math.ulp(tc_double)
ulp_ns = ulp_s * 1e9
```

or equivalently with `nextafter`.

C++ diagnostic if needed:

```cpp
nextafter(tc, +inf) - tc
```

Report for the 20 unique mismatches:

```text
exact_delta_ns = point_time_ns - tc_ns
tc_double
point_double as evaluated by production arithmetic
tc_double hex / point_double hex if useful
ULP(tc)_ns
abs(delta)/ULP
```

Required:

```text
corrected ULP evidence recorded
```

Do not keep the unsupported `~370 ns` number.

---

# 6. DEFINE REPRESENTATION_COLLISION EXACTLY

A unique point mismatch is a `REPRESENTATION_COLLISION` only if ALL are true:

```text
R1 exact physical timestamps disagree in ownership:
   exact point_time_ns > exact tc_ns
   while production emits current

R2 exact delta is strictly positive:
   delta_ns = point_time_ns - tc_ns > 0

R3 production's actual binary64 comparison cannot distinguish them:
   production-computed point_abs_double <= production tc_double

R4 the collision is explainable by binary64 representation:
   delta_ns <= ULP(tc_double) expressed in ns

R5 no future-data/readiness mismatch exists for this epoch

R6 point identity is otherwise conserved exactly:
   no duplicate
   no missing
   no overlap

R7 mismatch is only epoch ownership at the local cut;
   it does not change camera epoch sequence/readiness
```

Do NOT classify as representation collision if:
- delta exceeds one ULP;
- production point is multiple epochs early;
- camera readiness differs;
- a point is lost/duplicated;
- order/accounting differs.

Those remain `SEMANTIC_MISMATCH`.

---

# 6.1 Why one ULP is only a classification bound, not a tolerance

The implementation must NOT alter slicing to:

```text
point <= tc + ULP
```

or:

```text
abs(point-tc) <= epsilon
```

No new runtime tolerance.

Production remains its native double comparison.

The ULP is used only by the audit tool to explain exact-ns vs production-representation disagreement.

---

# 7. IMPROVE `compare_s0_exact.py`

Current tool double-counts a point whose epoch differs because it scans both maps.

Keep the symmetric count if useful for backward evidence, but add canonical unique metrics:

```text
unique_point_assignment_mismatch
representation_collision_unique
semantic_mismatch_unique
```

Required final primary gate:

```text
semantic_mismatch_unique == 0
```

and:

```text
all unique point mismatches
=
representation_collision_unique
+
semantic_mismatch_unique
```

For the current Day10 evidence, expected after classification:

```text
unique point mismatch = 20
representation collision = 20
semantic mismatch = 0
```

Do not force this expectation if the recomputation differs.

---

# 7.1 Ordered camera epoch sequence comparison

Current readiness comparison must not rely only on set intersection.

Compare exact ordered epoch sequence:

```text
production epoch_tcs_ns
vs
oracle epoch_tcs_ns
```

Required:

```text
same length
same timestamp at every index
```

Report:

```text
ordered_epoch_sequence_mismatch_count
first mismatch index if any
```

Hard:

```text
0
```

This prevents duplicate/order differences from being hidden by sets.

---

# 7.2 Retained/emitted identity comparison

Require explicit unique identity accounting:

```text
production emitted IDs
oracle emitted IDs
production final retained IDs
oracle final retained IDs
```

Report:
- emitted-only-prod;
- emitted-only-oracle;
- retained symmetric difference.

After accounting for representation-collision epoch reassignment, there must be:

```text
no missing identity
no extra identity
```

---

# 8. TDD — REPRESENTATION CLASSIFIER

Use `/tdd`.

Minimum tests:

## R3-T1 exact equal

```text
point_ns == tc_ns
```

No mismatch.

## R3-T2 exact +5ns but double collapses equal

Must classify:

```text
REPRESENTATION_COLLISION
```

## R3-T3 exact +25ns collapses equal

Same.

## R3-T4 delta larger than one ULP

If production/oracle disagree:

```text
SEMANTIC_MISMATCH
```

## R3-T5 previous epoch emission

Point assigned to an earlier different camera epoch beyond local collapse:

```text
SEMANTIC_MISMATCH
```

## R3-T6 readiness difference

Even if point delta is sub-ULP:

```text
SEMANTIC_MISMATCH / HARD FAIL
```

## R3-T7 duplicate camera epoch hidden by set comparison

New ordered comparator must catch it.

## R3-T8 reordered camera epochs

Must catch.

## R3-T9 unique mismatch counting

One point assigned to different epochs must report:

```text
unique mismatch = 1
```

even if legacy symmetric count is 2.

## R3-T10 real-value ULP calculation

At representative:

```text
tc ≈ 1.645e9 s
```

verify computed binary64 ULP is around:

```text
2.384185791015625e-7 s
≈238.418579 ns
```

Do not hard-code 370 ns.

---

# 9. RE-RUN DAY10 PRODUCTION VS EXACT ORACLE

Use the committed production/audit runners.

Do not change estimator/scheduler semantics.

Required full report:

```text
point input
point emitted
point retained
lost
dup
wrong_side
overlap

camera accounting
ordered epoch sequence mismatch

exact unique point mismatch
representation collision unique
semantic mismatch unique

delta_ns min/P50/P90/P99/max among representation collisions
ULP_ns min/P50/P90/P99/max
max(delta/ULP)

readiness mismatch
```

Owner acceptance gate:

```text
lost=0
dup=0
wrong_side=0
overlap=0

ordered epoch sequence mismatch=0
readiness mismatch=0
semantic mismatch unique=0
```

`representation_collision_unique` may be nonzero and is reported, not hidden.

---

# 10. P0R2 OWNER OVERRIDE CLOSURE

If Section 9 passes:

Reclassify P0R2 Q19 from:

```text
raw exact point mismatch must be 0
```

to the Owner-approved numerical gate:

```text
semantic mismatch unique = 0
AND
every raw exact mismatch is a proven representation collision
```

Then record:

```text
P0R2:
CLOSED BY OWNER NUMERIC-REPRESENTATION DECISION

Production timing representation:
binary64 seconds

Exact-ns oracle:
diagnostic physical-time reference

Observed Day10 representation collisions:
<n>

Semantic mismatches:
0
```

This is not an architecture deviation; it is the Owner's explicit numerical representation contract.

---

# 11. DO NOT CHANGE PRODUCTION SLICING REPRESENTATION IN P0R3

Forbidden in this corrective:

```text
adding exact-ns ownership fields to all LiDAR points
rewriting LidarData timestamps to integer-only
changing ESKF timestamp representation
changing IMU propagation times
changing slice boundary
adding epsilon/ULP tolerance to production
```

If new evidence shows a mismatch >1 ULP or a real semantic mismatch:

```text
STOP FOR OWNER
```

Do not escalate representation autonomously.

---

# 12. B0 ZERO-BLAST

Confirm the existing B0 evidence remains:

```text
9931f96e2a2fe2f524982edc5fe19372
```

If no production code changes occurred, do not rerun unnecessarily if the exact same committed evidence from P0R2 is still valid.

Document reuse provenance.

If any production scheduler code changes accidentally occur in P0R3:

```text
must rerun B0
```

---

# 13. RESUME ORIGINAL ROUND11W AUTOMATICALLY AFTER P0R3 PASS

Once:
- representation classifier passes;
- Day10 production-vs-oracle semantic mismatch = 0;
- commit/push succeeds;

resume:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

Do NOT stop merely because the 20 proven representation collisions remain.

Remaining Round11W work is still mandatory:

```text
1. exact camera terminal accounting closure
2. eee CAMERA-ENABLED C0 blast-radius
3. nya CAMERA-ENABLED C0 blast-radius
4. Day10 clean B0/C0
5. if Day10 C0/B0 > 1.10:
   complete cadence attribution
6. STOP FOR OWNER
```

Camera-off parity does NOT replace eee/nya C0.

---

# 14. EEE / NYA BLAST-RADIUS RULE

Run the exact current corrected S0 camera-enabled C0 configuration.

Do not compare against stale hashes from a materially older lifecycle commit without provenance.

For each dataset report:

```text
pre-S0/current-like baseline provenance
post-corrective C0 trajectory
rows
camera input/emitted
S0 accounting
whether trajectory changed due to the S0 scheduler corrections
```

If no exact comparable pre-corrective artifact exists:

```text
do not invent hash equivalence
```

Instead classify prior A0/A1 evidence conservatively as:
- canonical if the corrected C0 is proven identical by a valid like-for-like baseline;
- otherwise historical/pre-S0 and rebuild later.

---

# 15. DAY10 CLEAN B0/C0

After P0R3 and eee/nya blast work:

Run clean production-like:

```text
B0 FULL
C0 FULL
```

Heavy exact oracle OFF.

Same evaluator semantics.

Architecture gate:

```text
GREEN:
C0/B0 <= 1.10

AMBER:
1.10 < C0/B0 <= 1.50

RED:
C0/B0 > 1.50
```

If GREEN:
- S0 architecture closure;
- STOP FOR OWNER.

If AMBER/RED:
- complete deferred cadence attribution from Round11W;
- STOP FOR OWNER.

No A0/A1 yet.

---

# 16. SKILLS

Required:

```text
/tdd
/diagnosing-bugs if classifier/oracle behavior fails
/grill-with-docs only for genuine source semantic ambiguity
```

---

# 17. COMMIT DISCIPLINE

Suggested:

```text
test(super-livo): classify binary64 S0 boundary collisions explicitly
docs(super-livo): close P0R2 numeric representation gate
```

If only tooling/docs change, do not mix unrelated production edits.

Explicit staging only.

Never:

```bash
git add .
git add -A
git clean -fd
```

Push before resuming Round11W.

---

# 18. FINAL P0R3 REPORT FORMAT

```text
Round 11W-P0R3 Binary64 Boundary-Equivalence Closure

Starting HEAD:
e8179ba

Ending HEAD:
...

Architecture deviations:
NONE

=== Owner Numeric Decision ===
production S0 time representation:
binary64 seconds

production exact-ns rewrite:
NOT AUTHORIZED

runtime ULP tolerance:
NONE

exact-ns oracle retained:
YES

=== ROS Environment ===
ROS:
ROS1 Noetic

ROS2 compatibility code added:
NO

=== Existing Round11W WIP ===
WIP present:
YES/NO

starting diff SHA256:
...

preserved:
YES/NO

=== Skills Used ===
/tdd:
...
/diagnosing-bugs:
...
/grill-with-docs:
...

=== ULP Evidence ===
representative tc:
...

math.ulp(tc) seconds:
...

ULP ns:
...

previous ~370ns report corrected:
YES

=== Comparator Corrective ===
legacy symmetric point mismatch:
...

unique point mismatch:
...

representation collision unique:
...

semantic mismatch unique:
...

ordered epoch sequence mismatch:
...

readiness mismatch:
...

=== Representation Collision Distribution ===
delta_ns:
min/P50/P90/P99/max

ULP_ns:
min/P50/P90/P99/max

delta/ULP:
max

all satisfy classifier:
YES/NO

=== Day10 S0 ===
input:
...
emitted:
...
retained:
...
lost:
0
dup:
0
wrong_side:
0
overlap:
0

camera accounting:
PASS/FAIL

ordered epoch sequence:
PASS/FAIL

semantic mismatches:
0

representation collisions:
...

=== P0R2 Reclassification ===
Q19 old raw gate:
FAIL by exact-ns representation comparison

Owner replacement gate:
semantic mismatch=0 + all differences proven representation collisions

P0R2 status:
CLOSED / NOT CLOSED

=== Production Changes ===
scheduler production changed:
NO

slice production changed:
NO

timestamp representation changed:
NO

=== B0 Evidence ===
reused/rerun:
...

MD5:
9931f96e2a2fe2f524982edc5fe19372

PASS/FAIL

=== Commits ===
...

Push:
PASS/FAIL

=== Resume ===
Round11W:
RESUMED / NOT RESUMED

remaining:
eee camera-enabled C0
nya camera-enabled C0
Day10 clean B0/C0
cadence if non-green

Next:
RESUME ROUND11W
or
STOP FOR OWNER
```

---

# 19. STOP CONDITIONS

STOP FOR OWNER instead of resuming if ANY:

```text
semantic mismatch unique > 0
ordered camera epoch sequence mismatch > 0
readiness mismatch > 0
any exact mismatch exceeds one actual binary64 ULP
lost/dup/overlap nonzero
production code must change beyond this explicit representation decision
```

---

# 20. OWNER REMINDER

Do not chase nominal nanoseconds by rewriting the estimator's time representation unless the difference exceeds the runtime representation's actual numerical resolution or changes causal scheduling.

But also do not hide exact-vs-production differences.

The accepted contract is:

```text
exact-ns oracle tells us physical ordering

binary64 production tells us what the ROS1 estimator can numerically distinguish

sub-ULP collapses are explicitly counted as representation collisions

all larger/causal/order differences remain hard failures
```

After this numeric boundary is closed, return to the real question:

```text
does corrected camera-epoch S0 preserve C0 accuracy,
and if not, is partial-LiDAR cadence the remaining cause?
```
