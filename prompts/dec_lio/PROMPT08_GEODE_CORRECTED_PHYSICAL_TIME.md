# Dec-LIO Prompt08 — GEODE Corrected Physical-Time / Exact Scan-Input Closure

## 0. Owner authority

Repository:

```text
https://github.com/Scar-c/Super-LIO
```

Workspace:

```text
/home/lc/dec_lio/src/Super-LIO
```

Branch:

```text
Dec-LIO
```

Expected starting HEAD:

```text
b323bb27a23d15f937be36f084fe9cde5b7d5047
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build policy:

```text
-j4
```

No CPU-state control is required.

Prompt08 remains:

```text
SHADOW / INPUT-SEMANTICS CORRECTIVE ONLY

NO D2 estimator gate
NO H modification
NO b modification
NO P modification
NO IESKF equation modification
NO map algorithm modification
NO gamma
NO PCG
NO Prob-LIO
```

---

# 1. Prompt07R supersession

Prompt07R reported:

```text
CLOSED
B — FINITE_STRIDE_ORDER_MATERIALLY_CHANGES_WEAK_SUBSPACE
D2-OBS-R2
```

These conclusions are NOT authoritative.

Production review found a concrete bug in the newly added:

```text
g_geode_finite_then_stride == true
```

VELO16 branch.

Current faulty behavior:

```cpp
if (d2 > g_blind2) ++after_blind;
if (d2 > g_blind2 && d2 < g_maxrange2)
    ++after_upper_range;

lidar_data.pc->emplace_back(...);
```

The production code counts the blind/range gate but does not actually reject the point.

Therefore the previous:

```text
S
A*
```

arms did NOT implement their declared blind/range semantics.

Create:

```text
evidence/dec_lio/prompt08/PROMPT07R_SUPERSESSION.md
```

Record:

```text
Prompt07R status is superseded to:

PARTIAL — FINITE_THEN_STRIDE PRODUCTION BLIND/RANGE GATE BUG

Invalidated scientific authority:
- old S trajectory
- old S weak-subspace attribution
- old A* trajectory
- old A* persistence
- classification B
- D2-OBS-R2

Still valid:
- GEODE point-time branch audit
- official timestamp-unit bug discovery
- Super physical-seconds point-time authority
- raw-index Jaccard audit
- canonical 0.10 s evaluator authority
- native compatibility-OFF parity
```

Do NOT delete historical Prompt07R evidence.

---

# 2. External GEODE authority remains pinned

Use exactly:

```text
https://github.com/thisparticle/GEODE_Evaluation
commit:
1f008a7249e36393a1752622de50660b77b5b7f4
```

Do not update to current external HEAD.

Prompt07R established:

```text
Tunnel2:
given_offset_time TRUE = 2750/2750

Stairs:
given_offset_time TRUE = 3437/3456
FALSE = 19/3456
```

and for TRUE frames GEODE's official Alpha declaration causes:

```math
t_{\rm physical}
=
t_{\rm bag}\times10^{-6}.
```

The approximately:

```text
0.1 s
```

physical scan interval becomes approximately:

```text
1e-7 s.
```

Therefore the original GEODE Alpha FAST-LIO config effectively disables physical motion deskew on almost every frame.

This is now considered a confirmed external configuration bug for these released bags.

---

# 3. Corrected GEODE semantics authority

Prompt08 does NOT reproduce that time bug.

Define:

```text
GEODE_ALPHA_CORRECTED_PHYSICAL
```

as:

```text
VLP16
10 Hz
feature extraction OFF
remove non-finite XYZ first
stride = 3 on compact finite cloud
blind = 1.5 m
scan voxel = 0.3 m
no effective raw upper range cutoff
physical point time = bag field in seconds
full approximately 0.1 s deskew
Super native KF max iterations = 4
Super native map architecture
```

This is NOT:

```text
GEODE official FAST-LIO execution
```

because the published GEODE config has the timestamp-unit bug.

It is also NOT:

```text
full FAST-LIO equivalence
```

because map, estimator and IMU semantics remain Super-LIO.

Preferred label:

```text
GEODE_SCAN_SELECTION_ALIGNED_WITH_CORRECT_PHYSICAL_TIME
```

---

# 4. Corrected external config record

Create:

```text
evidence/dec_lio/prompt08/GEODE_ALPHA_TIMESTAMP_CORRECTIVE.md
```

Audit the GEODE timestamp-unit enum from source.

Determine the exact enum/value corresponding to:

```text
SEC
```

Do NOT assume its numeric value.

Document the minimal conceptual config correction:

```text
original:
timestamp_unit = US

corrected for released Alpha bag:
timestamp_unit = SEC
```

and prove from source that:

```text
SEC → curvature milliseconds = raw seconds × 1e3
→ physical seconds recovered downstream
```

Do NOT modify or push to the external GEODE repository.

This document is our corrected experimental authority only.

---

# 5. Startup gate

Run:

```bash
cd /home/lc/dec_lio/src/Super-LIO

git fetch --all --prune
git status --porcelain=v1
git rev-parse HEAD
git rev-parse origin/Dec-LIO
git merge-base HEAD origin/ros1
```

Required:

```text
HEAD == origin/Dec-LIO
HEAD == b323bb27a23d15f937be36f084fe9cde5b7d5047
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT08_START_STATE_MISMATCH
```

Archive this prompt:

```text
prompts/dec_lio/PROMPT08_GEODE_CORRECTED_PHYSICAL_TIME.md
```

---

# 6. HARD GATE A — repair production finite→stride filtering

Repair only the VELO16 compatibility branch.

Required logical order:

```text
raw typed cloud
→ finite XYZ compaction
→ finite_indices[0,3,6,...]
→ blind/range validity
→ accepted lidar_data.pc
```

After selecting:

```cpp
i = finite_indices[compact_i]
```

the actual production path MUST reject points failing:

```math
d^2 > blind^2
```

and:

```math
d^2 < maxrange^2.
```

Use the same strict inequality semantics as native `validPoint`.

Preferred implementation:

```cpp
if (!validPoint(pt.x, pt.y, pt.z))
    continue;
```

or an exactly equivalent shared helper.

Do NOT merely increment counters.

---

# 7. Production invariant

Immediately after the VELO16 input-selection loop, enforce/test the invariant:

```text
lidar_data.pc->size()
==
stage.after_upper_range
```

for both:

```text
native raw-stride mode
finite→stride compatibility mode
```

provided `after_upper_range` is defined as actual accepted point count.

If stage semantics require renaming, rename the counter clearly rather than weakening the invariant.

The previous bug must make this invariant fail in an adversarial reproduction.

---

# 8. Production-seam regression test

The old Python-only S1–S6 tests are insufficient.

Add a test that exercises the same production selection helper/seam used by ROSWrapper.

At minimum use finite selected points:

```text
range = 0.5 m
range = 1.0 m
range = 1.6 m
range = 3.0 m
range > maxrange
```

With:

```text
blind=1.5
```

required accepted set:

```text
1.6 m
3.0 m
```

only.

With:

```text
blind=2.0
```

required accepted set:

```text
3.0 m
```

only.

The test MUST fail against the pre-fix Prompt07R production behavior.

Call this:

```text
P1 — PRODUCTION_BLIND_RANGE_ENFORCEMENT
```

---

# 9. Preserve native mode exactly

With:

```text
geode_finite_then_stride=false
```

run canonical native parity.

Required SHAs:

Stairs:

```text
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

Tunnel2:

```text
3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
```

Require:

```text
cmp = 0
```

Otherwise:

```text
STOP — PROMPT08_NATIVE_PARITY_FAILURE
```

---

# 10. HARD GATE B — correct physical time

For Prompt08 corrected arms:

```text
point_time_scale = 1.0
```

The raw GEODE bag field remains:

```text
seconds
```

Do NOT emulate:

```text
×1e-6
```

GEODE bug-compatible timing.

For every selected point:

```math
t_i = t_{\rm header}+t_{{\rm raw},i}.
```

This is the physical point acquisition time authority.

---

# 11. Time-anchor audit

The bag uses approximately:

```text
raw point time ∈ [-0.101, +0.0013] s
```

so `header.stamp` is a time anchor, not necessarily scan-start time.

Do NOT infer meaning from the variable name:

```text
start_time
```

alone.

For both datasets and both new arms report per frame:

```text
header timestamp
minimum accepted offset
maximum accepted offset
last accepted point offset
minimum query timestamp
maximum query timestamp
configured lidar.end_time
```

Check:

```math
\Delta t_{\rm scan}
=
\max_i t_i-\min_i t_i
\approx0.1s.
```

---

# 12. HARD time synchronization invariant

The IMU synchronization boundary must cover every accepted LiDAR point.

Require:

```math
t_{\rm lidar,end}
\ge
\max_i(t_{\rm header}+t_{{\rm offset},i})
```

within floating-point tolerance.

Also report:

```text
last_selected_offset - max_selected_offset
```

distribution.

If the current implementation:

```cpp
end_time = header + pc->points.back().offset_time;
```

fails to cover the true maximum accepted point time on any frame, classify:

```text
TIME_END_ANCHOR_BUG
```

and apply a minimal correction:

```cpp
end_time =
header + max accepted point offset
```

for VELO16 physical-time semantics.

Do NOT change point query-time semantics.

If the invariant already holds on all frames, do not modify it.

---

# 13. Deskew proof

For representative beginning/middle/end frames in both bags, prove that corrected Super deskew spans physical motion over approximately 100 ms.

Report:

```text
accepted scan offset span
number of IMU states overlapping scan
earliest point query time
latest point query time
interpolated-point count
beyond-propagation fallback count
```

The desired result is NOT an exact number of IMU states.

The required semantic conclusion is:

```text
PHYSICAL_0P1S_DESKEW_ACTIVE
```

not:

```text
NEAR_ZERO_DESKEW
```

---

# 14. Do not emulate the official GEODE timestamp bug

No primary Prompt08 run may use:

```text
point_time_scale = 1e-6
```

except an optional unit/synthetic negative test.

Do not create a full trajectory arm with broken time unless separately needed for debugging.

Owner authority is:

```text
correct the GEODE config bug
```

not preserve it.

---

# 15. New S_correct arm

The old Prompt07R S is INVALID.

Define:

```text
S_correct
```

as exactly:

```text
geode_finite_then_stride = true
blind = 2.0
stride = 3
scan voxel = 0.5
maxrange = 150
point_time_scale = 1.0
KF max iterations = 4
```

This differs from N in exactly one semantic:

```text
raw-stride-before-finite
→
finite-compaction-before-stride
```

No other effective point filter may differ.

---

# 16. New A_correct arm

The old Prompt07R A* is INVALID.

Define:

```text
A_correct
```

as:

```text
geode_finite_then_stride = true
blind = 1.5
stride = 3
scan voxel = 0.3
maxrange = large sentinel with zero rejected observed points
point_time_scale = 1.0 physical seconds
KF max iterations = 4
```

This is the primary:

```text
GEODE_SCAN_SELECTION_ALIGNED_WITH_CORRECT_PHYSICAL_TIME
```

arm.

---

# 17. Dataset scope

Run only:

```text
Stairs_Alpha
Tunneling_tunnel2_alpha
```

Do NOT run:

```text
Bridge
FlatSurfacesS
Shield
Oxford
NTNU
Gamma
M3DGR
```

in Prompt08.

---

# 18. Runs

Run:

```text
S_correct Stairs ×2
S_correct Tunnel2 ×2

A_correct Stairs ×2
A_correct Tunnel2 ×2
```

Total new science runs:

```text
8
```

Require within each scene/config:

```text
RC=0
same row count
same trajectory SHA
same observation-stage CSV SHA
```

If not:

```text
STOP — PROMPT08_NONDETERMINISM
```

---

# 19. Canonical evaluator

Use only:

```text
association max_diff = 0.10 s
one global SE(3)
no scale
no crop
no per-window realignment
```

Do not use 0.05 s.

Tunnel2:

```text
position-only GT
GT attitude claim = MUST BE NO
```

Stairs:

```text
official full-pose reference
```

---

# 20. Reuse historical valid arms

Do NOT rerun:

```text
N
V
B
R
A_old
```

unless native parity requires it.

Reuse their corrected:

```text
0.10 s
```

evaluation from Prompt07R.

Do NOT reuse old invalid:

```text
S
A*
```

science results.

Label those:

```text
INVALID_PRE_FIX
```

in all final tables.

---

# 21. O_P authority

For each matched N-vs-S_correct and N-vs-A_correct frame:

\[
O_P
=
\frac{\operatorname{tr}(P_NP_X)}
{\min(r_N,r_X)}.
\]

Meaning:

```text
Does the preprocessing change which rotational subspace LiDAR considers weak?
```

For rank1/rank1 it is effectively:

\[
O_P=\cos^2\theta.
\]

Report:

```text
median
P5
P95
minimum
rank1 principal-angle median/P95/max
rank mismatch fraction
```

Do not use only median.

---

# 22. O_course authority

Reuse Prompt06 definition:

\[
O_{\rm course}
=
u_{\rm course}^TP_{\rm weak}u_{\rm course}.
\]

Meaning:

```text
Is rotation about the trajectory/course direction inside the weak rotational subspace?
```

It is NOT:

```text
failure probability
harmfulness score
confidence score
```

Report:

```text
median
P5
P95
```

plus weak-rank distribution.

---

# 23. Critical causal distinction

Prompt08 must separately answer:

### Q1

Does finite→stride change the selected raw points?

Already known:

```text
YES
Jaccard ≈0.2
```

### Q2

Does it change the healthy estimator weak subspace?

This is UNKNOWN until S_correct.

### Q3

Does it change final trajectory?

This is UNKNOWN until S_correct.

Do not infer Q2/Q3 from Q1.

---

# 24. S_correct failure interpretation

If Stairs S_correct remains stable:

```text
APE in ordinary sub-meter regime
```

then compare its weak subspace normally.

If Stairs S_correct again catastrophically diverges:

first prove:

```text
blind/range production invariant PASS
point-time invariant PASS
input population sane
no near-range leak
```

before attributing divergence to stride semantics.

A deterministic divergence is not automatically a scientific causal result.

---

# 25. Required observation populations

For N, S_correct, A_old, A_correct report:

```text
N_raw
N_finite
N_after_stride
N_after_blind
N_after_upper_range
N_undistorted
N_after_voxel
N_candidate
N_used
```

Hard invariant:

```text
N_undistorted
==
number of accepted input points
```

for the corresponding scan before VoxelGrid.

---

# 26. Re-evaluate weak-subspace robustness

For both scenes compare:

```text
N
S_correct
A_old
A_correct
```

Report:

```text
lambda_R min
lambda_t min
kappa_R median/P95
kappa_t median/P95
weak-rank distribution
O_P
principal angles
O_yaw
O_course
C_L
G/N
```

---

# 27. Tunnel2 A_correct persistence

Use frozen onset:

```text
1706584541.828
..
1706584579.030
```

Use:

```text
Stairs A_correct P95
```

as negative-control thresholds.

For:

```text
weak_chi_R
weak_psi_R
Psi_weak_R
A_weak_R
C_L
G/N
```

report Tunnel2 onset:

```text
median
P90
P95
P99
max
count above Stairs-A_correct P95
fraction above Stairs-A_correct P95
longest consecutive run
duration
```

Also report:

```text
all-six-above fraction
all-six longest run
all-six duration
```

---

# 28. Prompt06 mechanism re-check

Answer:

```text
Tunnel course-axis weak rotation survives corrected exact preprocessing:
YES / NO
```

```text
Stairs course-axis weak rotation survives:
YES / NO
```

```text
same-axis Stairs/Tunnel relationship survives:
YES / NO
```

```text
forcing remains nonpersistent:
YES / NO
```

Use only corrected valid arms for the final authority.

---

# 29. Time bug scientific interpretation

Prompt08 must explicitly distinguish:

```text
GEODE_OFFICIAL_ALPHA_TIME
```

from:

```text
GEODE_CORRECTED_PHYSICAL_TIME
```

Record:

```text
GEODE official:
seconds-scale bag field interpreted under US declaration
→ approximately 1e-7 s scan span
→ near-zero effective deskew

Prompt08 corrected:
seconds-scale bag field consumed as seconds
→ approximately 0.1 s scan span
→ physical deskew active
```

Do not mix these results.

---

# 30. No claim that GEODE benchmark is invalid

Do not overstate.

Allowed:

> The published Alpha FAST-LIO configuration is inconsistent with the released bag's point-time field and causes near-zero effective deskew in the audited source path.

Not allowed without dedicated reruns:

> Therefore all GEODE paper benchmark results are invalid.

Prompt08 does not audit all algorithms or all GEODE devices.

---

# 31. Scientific classification

Choose exactly one.

```text
A — WEAK_SUBSPACE_ROBUST_TO_CORRECTED_EXACT_GEODE_PREPROCESSING
```

Meaning:

```text
S_correct and A_correct keep the same qualitative weak direction,
while scale/trajectory may change.
```

```text
B — FINITE_STRIDE_ALONE_CHANGES_HEALTHY_WEAK_SUBSPACE
```

Only allowed if S_correct:

```text
production filtering PASS
physical time PASS
estimator remains scientifically interpretable
```

and weak-subspace change remains substantive.

```text
C — FINITE_STRIDE_CHANGES_TRAJECTORY_BUT_NOT_WEAK_DIRECTION
```

```text
D — CORRECTED_GEODE_ALIGNMENT_CHANGES_PHYSICAL_AXIS_INTERPRETATION
```

```text
E — TIME_ANCHOR_OR_END_TIME_REQUIRED_ADDITIONAL_CORRECTION
```

```text
F — MULTIPLE_INPUT_SEMANTICS_REMAIN_NONSEPARABLE
```

```text
G — INCONCLUSIVE
```

---

# 32. D2 observation authority

Choose one:

```text
D2-OBS-08A
Weak-subspace orientation is preprocessing-robust,
but magnitude/thresholds are preprocessing-dependent.
```

```text
D2-OBS-08B
Weak-subspace orientation materially depends on exact point-selection semantics.
```

```text
D2-OBS-08C
Correct time synchronization changes prior interpretation and requires further rebase.
```

```text
D2-OBS-08D
Evidence remains insufficient.
```

None authorize an estimator gate.

---

# 33. Threshold portability

Using valid:

```text
N
S_correct
A_correct
```

revisit:

```text
raw lambda
kappa
weak chi
Psi
```

Choose:

```text
YES_WITH_NORMALIZATION
NO_CONFIG_SPECIFIC
NOT_YET_KNOWN
```

Do not force a conclusion from only two scenes.

---

# 34. Required evidence files

Create:

```text
evidence/dec_lio/prompt08/
```

At minimum:

```text
PROMPT08_START_STATE.txt
PROMPT07R_SUPERSESSION.md

PRODUCTION_BLIND_RANGE_FIX.md
PRODUCTION_SELECTION_TESTS.txt

GEODE_ALPHA_TIMESTAMP_CORRECTIVE.md
PHYSICAL_TIME_ANCHOR_AUDIT.md
PHYSICAL_DESKEW_PROOF.md

NATIVE_PARITY.md

S_CORRECT_SUMMARY.md
A_CORRECT_SUMMARY.md

WEAK_SUBSPACE_CORRECTED.md
PHYSICAL_AXIS_CORRECTED.md
TUNNEL2_ACORRECT_PERSISTENCE.md

CANONICAL_TRAJECTORY_COMPARISON.md
D2_OBSERVATION_PROMPT08_AUTHORITY.md

PROMPT08_SOURCE_DIFF.txt
PROMPT08_CLOSURE.txt
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt08/
```

Large logs remain runtime-only.

---

# 35. Mandatory negative/adversarial tests

Must include:

### N1

Pre-fix finite→stride logic accepts a selected point with:

```text
range < blind
```

Test must demonstrate this would fail the new invariant.

### N2

Post-fix production seam rejects it.

### N3

Selected finite point:

```text
range > maxrange
```

must be rejected.

### N4

A point exactly:

```text
range == blind
```

must be rejected because native inequality is strict.

### N5

A point exactly:

```text
range == maxrange
```

must be rejected.

### N6

`geode_finite_then_stride=false` preserves native SHA.

### N7

A wrong:

```text
point_time_scale=1e-6
```

must produce approximately:

```text
1e-7 s
```

scan span and be classified:

```text
BROKEN_GEODE_TIME_NEGATIVE_CONTROL
```

### N8

Correct:

```text
point_time_scale=1.0
```

must recover approximately:

```text
0.1 s
```

scan span.

### N9

If `last selected offset < max selected offset`, the end-time invariant must catch it.

---

# 36. CLOSE criteria

Only report:

```text
PROMPT08 CLOSED
```

if all hold.

### Production fix

```text
finite→stride blind/range actually filters points
production invariant PASS
N1–N9 PASS
```

### Timing

```text
GEODE SEC corrective semantics proven
Super physical seconds proven
physical ~0.1 s deskew active
lidar.end_time covers max selected point time
```

### Native

```text
canonical Stairs SHA exact
canonical Tunnel2 SHA exact
```

### New runs

```text
S_correct Stairs x2 deterministic
S_correct Tunnel2 x2 deterministic
A_correct Stairs x2 deterministic
A_correct Tunnel2 x2 deterministic
```

### Evaluation

```text
all canonical comparison metrics use 0.10 s association
old invalid S/A* excluded from scientific authority
```

### Science

```text
O_P comparison complete
O_course comparison complete
weak rank complete
persistence complete
Prompt06 robustness re-evaluated
scientific classification selected
D2 authority selected
threshold portability answered
```

### Boundary

```text
H modified: NO
b modified: NO
P modified: NO
map algorithm modified: NO
gamma: NO
PCG: NO
Prob-LIO: NO
```

### Git

```text
HEAD == origin/Dec-LIO
worktree clean
```

---

# 37. Mandatory final report

```text
PROMPT08 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Prompt07R supersession:
- previous status:
- corrected status:
- invalid old arms:
- invalid old classification:
- invalid old D2 authority:

Production bug:
- exact missing gate:
- fix:
- pc-size / accepted-count invariant:
- adversarial production test:
- native OFF parity:

GEODE timestamp correction:
- official timestamp_unit:
- actual bag unit:
- corrected timestamp_unit enum/value:
- official effective scan span:
- corrected physical scan span:
- GEODE official bug-compatible deskew:
- Prompt08 corrected deskew:

Time anchor:
- raw offset min/median/max Stairs:
- raw offset min/median/max Tunnel:
- accepted offset span:
- last-selected minus max-selected distribution:
- end_time invariant:
- end_time code correction required: YES/NO

Physical deskew:
- Stairs representative IMU overlap:
- Tunnel representative IMU overlap:
- PHYSICAL_0P1S_DESKEW_ACTIVE: YES/NO

Native parity:
- Stairs expected/actual/cmp:
- Tunnel expected/actual/cmp:

S_correct — Stairs:
- run1 SHA:
- run2 SHA:
- deterministic:
- N_after_stride:
- N_after_blind:
- N_after_voxel:
- N_used:
- lambda:
- kappa:
- weak rank:
- O_P:
- principal angle:
- O_yaw:
- O_course:
- C_L:
- G/N:
- APE @0.10:

S_correct — Tunnel:
- same fields:

A_correct — Stairs:
- run1 SHA:
- run2 SHA:
- deterministic:
- N_after_stride:
- N_after_blind:
- N_after_voxel:
- N_used:
- lambda:
- kappa:
- weak rank:
- O_P:
- principal angle:
- O_yaw:
- O_course:
- C_L:
- G/N:
- APE @0.10:

A_correct — Tunnel:
- same fields:
- GT attitude claim: MUST BE NO

Prompt06 mechanism:
- Tunnel course-axis conclusion survives:
- Stairs course-axis conclusion survives:
- same-axis relationship survives:
- forcing nonpersistent survives:

Tunnel A_correct persistence:
- Stairs-A_correct P95 thresholds:
- onset median/P90/P95/P99/max:
- fraction above thresholds:
- longest run:
- duration:
- all-six fraction/run/duration:

Scientific classification:
- A/B/C/D/E/F/G:
- reasoning:

D2:
- D2-OBS-08A/B/C/D:
- threshold portability:
- estimator gate authorized: MUST BE NO

Boundary:
- H modified:
- b modified:
- P modified:
- map algorithm modified:
- gamma:
- PCG:
- Prob-LIO:

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Origin reminder:

> Prompt08 uses the GEODE authors' scan-selection policy but intentionally corrects their audited Alpha timestamp-unit mismatch. The scientific authority is therefore “GEODE scan-selection aligned with physically correct approximately 0.1 s deskew,” not bug-compatible GEODE FAST-LIO execution.