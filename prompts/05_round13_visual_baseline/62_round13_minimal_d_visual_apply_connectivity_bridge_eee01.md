# Round 13 — Minimal D Visual-Apply Connectivity Bridge / eee_01

## 0. Executor / Owner Decision

You remain:

```text
agent-ds
```

Current expected HEAD:

```text
99c3c88996d0191bb41b08251d2fdb0977f9ac80
```

Previous classification:

```text
ROUND13_STOPPED_FOR_OWNER
```

Owner accepts that STOP and authorizes exactly one production corrective:

```text
ROUND13_D_VISUAL_APPLY_MINIMAL_CONNECTIVITY_BRIDGE_AUTHORIZED
```

The confirmed problem is:

```text
D_VISUAL_SHADOW
→ D_CORRECTED / imu_fullscan
→ real Visual H/b exists

but

existing production Visual Apply block
→ LidarUpdatePolicy::PARTIAL only
→ unreachable from D_CORRECTED
```

This is classified as:

```text
C_TO_D_VISUAL_APPLY_CONNECTIVITY_GAP
```

Do NOT redesign the Visual estimator.

Do NOT restore the previously quarantined Round13 placement implementation.

---

# 1. Startup / Agent State Consensus

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected HEAD:

```text
99c3c88996d0191bb41b08251d2fdb0977f9ac80
```

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -35
git diff --check
```

Report:

```text
EXPECTED_HEAD
ACTUAL_HEAD
HEAD_MATCH
WORKTREE
```

No reset/rebase/history deletion.

---

# 2. Prompt registration

Register this exact prompt.

Suggested:

```text
prompts/05_round13_visual_baseline/
62_round13_minimal_d_visual_apply_connectivity_bridge_eee01.md
```

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

State:

```text
D_VISUAL_SHADOW:
CLOSED / CANONICAL

D_VISUAL_APPLY:
CONNECTIVITY CORRECTIVE AUTHORIZED

other datasets:
BLOCKED
```

---

# 3. Skills

Use:

```text
/tdd
/diagnosing-bugs
```

Use `/grill-with-docs` if implementation evidence conflicts with:

```text
CONTEXT
ADR-001
ADR-003
normalized profile standard
FAST-LIVO2-inspired sequential-update contract
```

Report all skill use.

---

# 4. Frozen evidence — DO NOT reopen

The following are established:

```text
D_VISUAL_SHADOW producer = ACTIVE

D_VISUAL_SHADOW measurement = ACTIVE

real eee residual samples = 12,587,828

real H accumulations = 823
real H_nonzero = 823

real b accumulations = 823
real b_nonzero = 823

H/b nonfinite = 0

visual_state_apply = OFF

Shadow trajectory stable and byte-identical across Prompt59/60
```

Also established:

```text
ESKF::UpdateObserveFromPrior
```

already implements:

```text
state posterior update
covariance posterior update
reset-Jacobian covariance transform
posterior symmetrization
PosteriorSnapshot{x,P}
```

The primitive is not the current blocker.

---

# 5. Confirmed connectivity bug

Existing Apply production block is reachable only when:

```text
LidarUpdatePolicy::PARTIAL
```

while normalized D uses:

```text
D_CORRECTED
imu_fullscan
FULL_RAW_SCAN_AT_SCAN_END
```

Therefore:

```text
profile says Apply ON
but
production Apply path unreachable
```

This is a production connectivity bug.

Do not solve it by changing the normalized D profile to PARTIAL.

D remains:

```text
D_CORRECTED
FULL_RAW_SCAN_AT_SCAN_END
ONE full LiDAR Observe per raw scan
```

---

# 6. Owner architecture decision

The corrected D architecture is:

```text
latest committed posterior
        ↓
propagate to next event time
        ↓
camera measurement epoch t_c
        ↓
existing Visual producer/query/residual
        ↓
existing H,b at t_c
        ↓
D_VISUAL_SHADOW:
    stop here; no x/P commit

D_VISUAL_APPLY:
    existing UpdateObserveFromPrior
    using prior at this SAME measurement epoch
        ↓
commit Visual posterior x_V,P_V
        ↓
continue propagation from x_V,P_V
        ↓
later raw LiDAR scan end
        ↓
ONE full LiDAR geometry Observe
```

This is the normative bridge contract.

---

# 7. Critical co-location rule

The Apply bridge MUST attach to:

> the existing D Visual measurement event that produced the accepted \(H,b\).

It MUST NOT:

```text
recompute H/b at another event

store H/b and apply it later at scan end

use a stale earlier prior

move the Visual producer

move landmark creation

change reference selection

change camera payload ownership merely to reach Apply
```

Measurement linearization and Apply prior must refer to the same estimator epoch.

---

# 8. Exact prior contract

For a Visual measurement at camera epoch:

\[
t_c
\]

define:

```text
x_c^- = latest committed estimator state propagated to t_c
P_c^- = corresponding covariance at t_c
```

The Visual normal equation:

\[
H_V,\;b_V
\]

must have been constructed around that same camera-epoch state semantics.

Then Apply uses:

```text
UpdateObserveFromPrior(
    x_c^-,
    P_c^-,
    H_V,
    b_V
)
```

or the existing API's equivalent representation.

Result:

```text
x_c^+
P_c^+
```

becomes the new committed estimator posterior.

---

# 9. Multiple camera epochs

If multiple valid camera epochs occur before a raw LiDAR scan closes:

```text
prior scan posterior
    ↓ propagate
camera_1
    ↓ Visual Apply
x_V1,P_V1
    ↓ propagate
camera_2
    ↓ Visual Apply
x_V2,P_V2
    ↓ ...
    ↓ propagate
raw scan end
    ↓ ONE LiDAR Observe
```

Every camera update consumes the posterior from the preceding committed event.

Forbidden:

```text
all camera updates reuse the same old LiDAR posterior
```

Required:

```text
LATEST_POSTERIOR_CHAIN = PASS
```

---

# 10. LiDAR contract remains unchanged

Visual Apply MUST NOT cause:

```text
partial LiDAR Observe per camera

extra geometry Observe

premature raw-scan consumption

duplicate point ownership

scan fragmentation
```

Required:

```text
raw LiDAR retained until scan end

full geometry Observe = exactly 1 per processable raw scan
```

---

# 11. Do NOT resurrect quarantined implementation

The following historical Round13 commits remain rejected as whole solutions:

```text
33c1b3d
7d9be50
ce3d1a9
```

Do NOT:

```text
git revert the revert
cherry-pick them
restore their complete hunks
```

They were based on contaminated evidence and changed more lifecycle semantics than currently required.

---

# 12. Minimal bridge rule

Implement only what is necessary for:

```text
existing D measurement H/b
        ↓
existing UpdateObserveFromPrior
        ↓
commit x/P
```

The desired code delta should be much smaller than the prior placement corrective.

No new:

```text
CameraFrame lifetime abstraction

NoPop ownership path

alternate producer path

alternate VisualMap lifecycle

parallel D estimator
```

---

# 13. Shadow must remain the same code path

Do NOT create:

```text
runVisualShadow()
runVisualApply()
```

as two independently evolving Visual pipelines.

Required architecture:

```text
same Visual measurement generation
        ↓
same H/b
        ↓
if semantic_profile.apply == false:
    no state commit
else:
    call existing Apply primitive
```

The only algorithmic semantic branch is state application.

---

# 14. Profile-driven gate

The Apply condition must derive from the normalized semantic profile:

```text
visual_state_apply
```

not from:

```text
LidarUpdatePolicy::PARTIAL
dataset name
Round13 mode
launch filename
legacy D0/DV0 alias
```

Legacy policy may continue elsewhere if needed historically, but canonical D Apply reachability must not depend on PARTIAL.

---

# 15. Preserve legacy behavior

Do not accidentally alter historical PARTIAL/C execution.

Add tests showing:

```text
legacy PARTIAL behavior remains unchanged
```

unless an explicit dead path is being removed.

No broad cleanup/refactor.

---

# 16. Required bridge TDD

Use strict RED → GREEN.

## B-T1 — reproduce current failure

At starting HEAD:

```text
D_CORRECTED
+
visual_state_apply=true
+
valid nonzero H/b
```

must reproduce:

```text
Apply unreachable
```

RED evidence mandatory.

---

## B-T2 — D Shadow remains non-applying

```text
D_CORRECTED
visual_state_apply=false
valid nonzero H/b
```

must:

```text
produce measurement
NOT call UpdateObserveFromPrior
NOT mutate x
NOT mutate P
```

---

## B-T3 — D Apply reaches existing primitive

```text
D_CORRECTED
visual_state_apply=true
valid nonzero H/b
```

must invoke:

```text
UpdateObserveFromPrior
```

exactly once for that measurement event.

---

## B-T4 — same-epoch prior

Capture:

```text
measurement timestamp
prior timestamp
```

Required:

```text
same estimator epoch semantics
```

No delayed stale prior.

---

## B-T5 — state posterior

Informative synthetic Visual measurement:

```text
x_after != x_before
```

for expected state components.

---

## B-T6 — covariance posterior

Informative measurement:

```text
P_after != P_before
```

and:

```text
finite
symmetric within existing tolerance
```

---

## B-T7 — zero information

Zero H/b:

```text
no meaningful x/P change
```

according to existing zero-information semantics.

---

## B-T8 — nonfinite rejection

Nonfinite measurement/solve must not commit invalid x/P.

---

## B-T9 — exact-once Apply

One accepted Visual measurement event:

```text
Apply invocation = 1
state commit = 1
covariance commit = 1
```

No duplicate apply.

---

## B-T10 — multiple camera chaining

Two synthetic camera epochs before scan end:

```text
camera2 prior
=
posterior committed by camera1 propagated onward
```

not the earlier LiDAR prior.

---

## B-T11 — one LiDAR Observe

Multiple Visual Apply events inside one raw scan still produce:

```text
ONE
```

full LiDAR Observe for that raw scan.

---

## B-T12 — raw point ownership

Visual Apply does not change:

```text
duplicate ownership = 0
```

and legal point accounting semantics.

---

## B-T13 — Shadow parity

Apply OFF after bridge reproduces pre-bridge Shadow result in deterministic synthetic/canonical seam.

---

## B-T14 — legacy PARTIAL regression

Previously valid PARTIAL path behavior remains intact.

---

# 17. Camera payload rule

The accepted Shadow run already proves the current camera lifecycle can generate:

```text
landmarks
queries
residuals
H/b
```

Therefore:

> Do not modify camera payload lifetime unless B-T tests mechanically prove it is necessary for the Apply call itself.

Expected outcome:

```text
CAMERA_PAYLOAD_CHANGE_REQUIRED = NO
```

If not:

STOP_FOR_OWNER before changing ownership.

---

# 18. Producer lifecycle rule

The producer is already proven healthy.

Do NOT modify:

```text
VisualMap insertion
landmark producer
reference creation
candidate query
patch evaluator
```

Any change there is out of scope.

---

# 19. Measurement math freeze

Do NOT change:

```text
H = Σ ω J J^T
b = Σ -(ω J r)
```

or current equivalent implementation.

No changes to:

```text
J
residual
omega_photo
variance
robust/outlier gates
```

---

# 20. Existing Apply primitive reuse

Prefer direct reuse of:

```text
ESKF::UpdateObserveFromPrior
```

Do NOT duplicate its Kalman/information update math in mapper code.

Do NOT write a second covariance update.

---

# 21. Covariance is mandatory

Canonical Apply means:

```text
x committed
AND
P committed
```

A bridge that only writes state is invalid.

Required:

```text
POSTERIOR_STATE_COMMIT = YES
POSTERIOR_COVARIANCE_COMMIT = YES
```

---

# 22. Instrumentation

Reuse existing measurement instrumentation.

Add only lightweight Apply counters if missing:

```text
visual_apply_attempts
visual_apply_accepts
visual_apply_rejects

visual_solve_attempts
visual_solve_success
visual_solve_nonfinite

visual_state_commit_count
visual_covariance_commit_count
```

And applied correction norms:

```text
||δθ||
||δp||
```

bounded aggregate statistics only.

---

# 23. No heavy diagnostics

Default OFF:

```text
Gate-M FD
HB oracle
per-residual dumps
full per-frame matrices
sanitizers
heavy profiling
```

Only escalate if a concrete anomaly appears.

---

# 24. Static code review gate before bag

Before playback produce:

```text
docs/super_livo/evidence/
round13_minimal_d_visual_apply_bridge_audit.md
```

It must show:

```text
old unreachable condition
new minimal condition

measurement location
Apply location

prior source
posterior commit

Shadow branch
Apply branch

LiDAR Observe location
```

---

# 25. Production diff budget

Before full run inspect:

```bash
git diff <pre-bridge-HEAD> -- \
  src include scripts config
```

Classify every production hunk.

Expected production delta:

```text
minimal Apply reachability/connectivity only
```

Unexpected changes to:

```text
producer
residual
scheduler ownership
payload lifecycle
LiDAR math
IMU math
```

=> STOP.

---

# 26. Full-run authorization gate

A single eee Apply run is authorized only after:

```text
B-T1 RED reproduced

B-T2..B-T14 GREEN

production diff minimal

camera payload unchanged

producer lifecycle unchanged

measurement math unchanged

state + covariance posterior proven

latest-posterior chaining proven

one-LiDAR-Observe invariant proven
```

Otherwise STOP_FOR_OWNER.

---

# 27. Canonical Apply profile

Only:

```text
D_VISUAL_APPLY
```

with:

```text
scheduler_family = D_CORRECTED

camera = ON
camera_epoch = ON

frontend = ON
producer = ON
measurement = ON

visual_state_apply = ON

raw LiDAR =
FULL_RAW_SCAN_AT_SCAN_END

full LiDAR Observe/raw scan = 1
```

---

# 28. Shadow → Apply semantic diff

Use machine validation.

Required protected diff:

```text
visual_state_apply:
false → true
```

ONLY.

Allowed metadata:

```text
profile name
RUN_ID
output directory
instrumentation Apply counters
```

Required:

```text
SHADOW_TO_APPLY_ALGORITHM_DELTA =
ONE FIELD
```

---

# 29. Dataset

Only:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

No other dataset.

---

# 30. Transaction contract

Use existing accepted GTP transaction supervisor.

Preflight:

```text
active transaction = NONE
conflicting rosbag = NONE
conflicting estimator = NONE
lock = ACQUIRED
semantic profile = PASS
bridge TDD = PASS
producer gates = PASS
validator = READY
```

---

# 31. One full Apply run

Authorized:

```text
N = 1
profile = D_VISUAL_APPLY
dataset = NTU eee_01
```

No repeated run automatically.

---

# 32. Runtime hard gates

Require nonzero:

```text
landmarks

query hits

valid observations

residual samples

H_nonzero

b_nonzero

solve attempts

solve success

apply attempts

apply accepts

state commits

covariance commits

nonzero applied corrections
```

Require:

```text
H/b nonfinite = 0

solve nonfinite = 0

state nonfinite = 0

P nonfinite = 0
```

---

# 33. Exact-once runtime bridge accounting

For each accepted Visual Apply:

```text
measurement accepted
→ exactly one solve
→ exactly one state/covariance commit
```

Aggregate conservation:

```text
visual_apply_attempts
=
visual_apply_accepts
+ visual_apply_rejects
```

and where accepted:

```text
visual_state_commit_count
=
visual_covariance_commit_count
=
visual_apply_accepts
```

unless code has a mechanically justified explicit exception.

---

# 34. Sequential-prior runtime evidence

Persist bounded representative events.

For several camera events record:

```text
camera timestamp

prior estimator timestamp

prior source/event type

H norm
b norm

delta norm

posterior estimator timestamp
```

For consecutive camera epochs verify:

```text
next camera prior
descends from previous Visual posterior
```

Required:

```text
LATEST_POSTERIOR_RULE = PASS
```

---

# 35. D scheduler runtime invariants

Required:

```text
raw scans
processable scans
full LiDAR Observe
```

with:

```text
full LiDAR Observe
=
processable scans
```

and:

```text
duplicates = 0
```

No Visual-induced geometry Observe.

---

# 36. Fail-closed validator

Canonical Apply valid only if:

```text
semantic profile PASS

measurement active

solver active

Apply active

state commit active

covariance commit active

latest-posterior rule PASS

D scheduler invariants PASS

transaction SUCCESS

cleanup_verified = true
```

Architecture gate precedes ATE.

---

# 37. Shadow read-only ATE evaluation

After Apply architecture validity passes, evaluate the immutable canonical Shadow trajectory:

```text
SHA256 =
539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e
```

using the same validated NTU VIRAL evaluator as Apply.

Do NOT rerun Shadow.

Record:

```text
ATE_SHADOW
```

---

# 38. Apply ATE

Only if Apply architecture validator passes:

evaluate:

```text
ATE_APPLY
```

with identical:

```text
GT
association
alignment
metric
```

---

# 39. Scientific expectation

Because Shadow and Apply now differ only by state application:

the Apply run should exhibit:

```text
real nonzero Visual posterior corrections
```

without destabilizing the D scheduler.

At this baseline stage, Visual improvement is desirable but not required to prove connectivity.

However significant degradation is meaningful.

---

# 40. Ratio

Define:

\[
R_{\mathrm{apply}}
=
\frac{\mathrm{ATE}_{\mathrm{APPLY}}}
     {\mathrm{ATE}_{\mathrm{SHADOW}}}.
\]

Classify only after architecture PASS.

GREEN:

```text
R_apply <= 1.10
```

AMBER:

```text
1.10 < R_apply <= 1.30
```

RED:

```text
R_apply > 1.30
```

---

# 41. Optional historical context — no tuning

Also report, only as contextual evidence if already available:

```text
historical C-family visual A0/A1 eee results
pristine Super-LIO eee result
```

Clearly label them:

```text
NOT SAME SEMANTIC CONTROL
```

Do not use them as denominator for `R_apply`.

The canonical denominator is `D_VISUAL_SHADOW`.

---

# 42. If Apply GREEN

Classify:

```text
ROUND13_EEE_D_VISUAL_APPLY_GREEN
```

STOP.

Do not run nya.

---

# 43. If Apply AMBER

Classify:

```text
ROUND13_EEE_D_VISUAL_APPLY_AMBER
```

Perform only bounded attribution:

```text
correction norm distribution
P health
first trajectory degradation interval
measurement density
```

No tuning.

---

# 44. If Apply RED

Classify:

```text
ROUND13_EEE_D_VISUAL_APPLY_RED
```

No feature additions.

No parameter sweep.

STOP_FOR_OWNER.

---

# 45. If divergence occurs

If infrastructure/config/architecture are valid:

```text
EXECUTION_VALID = YES
ALGORITHM_OUTCOME = VISUAL_DIVERGENCE
```

Do not misclassify as experiment invalid.

---

# 46. If bridge cannot be minimal

If enabling Apply requires any of:

```text
new camera payload lifetime

producer relocation

scan splitting

additional LiDAR Observe

large scheduler rewrite

restoring quarantined Round13 lifecycle patches
```

STOP:

```text
ROUND13_MINIMAL_BRIDGE_NOT_POSSIBLE
```

Return to Owner.

---

# 47. Canonical ledger

If Apply architecture is valid, add:

```text
normalized_semantic_profile =
D_VISUAL_APPLY
```

Preserve:

```text
D_VISUAL_SHADOW
historical D0/D-S1/D-S3
historical C/A rows
```

Do not reuse historical `DV0` as the new canonical name.

---

# 48. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 49. Final report format

Use:

```text
Round 13 — Minimal D Visual-Apply Connectivity Bridge / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Production changes:
Instrumentation changes:
Execution deviations:

=== Agent State Consensus ===
executor:
expected HEAD:
actual HEAD:
frontier verified:

=== Owner Decision Applied ===
authorized:
MINIMAL D_VISUAL_APPLY CONNECTIVITY BRIDGE

quarantined placement commits restored:
NO

other datasets:
NOT AUTHORIZED

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Pre-Fix Connectivity RED ===
D profile:
H/b:
old Apply condition:
solver reachable:
NO

B-T1:
RED/PASS reproduction

=== Minimal Bridge Design ===
Visual measurement event:
timestamp semantics:

prior x:
prior P:

H/b source:

Apply primitive:
UpdateObserveFromPrior / exact equivalent

posterior x:
posterior P:

next-event propagation source:

LiDAR scan-end Observe:
unchanged

camera payload changes:
NONE

producer changes:
NONE

measurement math changes:
NONE

=== Production Diff Audit ===
files changed:
hunks:
reason for each:

33c1b3d restored:
NO

7d9be50 restored:
NO

ce3d1a9 restored:
NO

minimality:
PASS/FAIL

=== Bridge TDD ===
B-T1:
B-T2:
B-T3:
B-T4:
B-T5:
B-T6:
B-T7:
B-T8:
B-T9:
B-T10:
B-T11:
B-T12:
B-T13:
B-T14:

=== Shadow→Apply Semantic Delta ===
Shadow:
D_VISUAL_SHADOW

Apply:
D_VISUAL_APPLY

protected differences:
visual_state_apply false→true

other algorithm differences:
NONE / list

gate:
PASS/FAIL

=== Effective Config ===
pre_node:
post_resolve:
resolved profile:

camera:
camera epoch:
frontend:
producer:
measurement:
state apply:

D scheduler:
raw LiDAR:
Observe/raw scan:

=== Transaction Preflight ===
active transaction:
conflicting rosbag:
conflicting estimator:
lock:
profile:
bridge TDD:
producer:
validator:

=== Visual Measurement ===
landmarks:
query attempts:
query hits:
valid observations:
residual samples:

H accumulations:
H nonzero:
H zero:
H nonfinite:

b accumulations:
b nonzero:
b zero:
b nonfinite:

VISUAL_MEASUREMENT_ACTIVE:
YES/NO

=== Visual Solve ===
attempts:
success:
rejected:
nonfinite:

VISUAL_SOLVE_ACTIVE:
YES/NO

=== Visual Apply ===
attempts:
accepts:
rejects:

state commits:
covariance commits:

delta rotation:
 count:
 nonzero:
 nonfinite:
 median:
 P95:
 P99:
 max:

delta translation:
 count:
 nonzero:
 nonfinite:
 median:
 P95:
 P99:
 max:

VISUAL_STATE_APPLY_ACTIVE:
YES/NO

=== Exact-Once Apply Accounting ===
measurement accepted:
solve success:
apply accepts:
state commits:
covariance commits:

conservation:
PASS/FAIL

=== Sequential Prior Evidence ===
representative event 1:
...
representative event 2:
...

multiple camera chaining:
PASS/FAIL

latest-posterior rule:
PASS/FAIL

=== Covariance Health ===
posterior P committed:
YES/NO

finite:
PASS/FAIL

symmetry:
PASS/FAIL

nonfinite count:

diagonal sanity:
PASS/FAIL

=== D Scheduler Invariants ===
raw scans:
processable:
full LiDAR Observe:
duplicate:
never-used:
legal exclusions:
camera propagation:

status:
PASS/FAIL

=== Fail-Closed Validator ===
profile:
measurement:
solver:
Apply:
state commit:
covariance commit:
sequential prior:
D scheduler:
transaction:

final:
PASS/FAIL

=== Shadow Evaluation ===
trajectory SHA256:
539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e

ATE_SHADOW:
evaluator:
validity:

=== Apply Evaluation ===
rows:
coverage:

ATE_APPLY:

execution valid:
YES/NO

algorithm outcome:

=== Shadow vs Apply ===
ATE_SHADOW:
ATE_APPLY:

R_apply:

architecture:
PASS/FAIL

numeric:
GREEN/AMBER/RED/N/A

=== Historical Context Only ===
pristine Super-LIO eee:
...

historical C Visual:
...

same-semantic comparator:
NO

=== Canonical Ledger ===
Shadow preserved:
YES

Apply added:
YES/NO

legacy labels preserved:
YES

=== Heavy Diagnostics ===
enabled:
NO / reason

=== Tests ===
repository:
bridge:
measurement:
solver:
state/covariance:
LiDAR ownership:
transaction:
snapshot:
evaluator:
cleanup:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Final Classification ===

Choose exactly one:

ROUND13_EEE_D_VISUAL_APPLY_GREEN

ROUND13_EEE_D_VISUAL_APPLY_AMBER

ROUND13_EEE_D_VISUAL_APPLY_RED

ROUND13_EEE_D_VISUAL_APPLY_DIVERGENCE

ROUND13_EEE_D_VISUAL_APPLY_CONNECTIVITY_FAIL

ROUND13_EEE_D_VISUAL_APPLY_POSTERIOR_SEMANTICS_FAIL

ROUND13_MINIMAL_BRIDGE_NOT_POSSIBLE

ROUND13_STOPPED_FOR_OWNER

=== Next Step Recommendation Only ===

If GREEN:
recommend next dataset/profile validation only.

Do NOT execute it.

If non-GREEN:
give first-order attribution only.
No tuning or new FAST-LIVO2 feature.
```

Full 40-character Final HEAD mandatory.

---

# 50. Core Owner rule

Do not solve:

```text
D Apply unreachable
```

by restoring the old `PARTIAL` architecture.

Do not solve it by rebuilding camera ownership.

The Shadow run has already proven that the D pipeline reaches:

```text
camera
→ producer
→ landmarks
→ query
→ residual
→ nonzero finite H,b
```

Therefore the authorized fix is only:

```text
that existing H,b
        ↓
existing UpdateObserveFromPrior
        ↓
commit x,P
```

at the SAME D Visual measurement epoch.

Everything before H/b stays frozen.

Everything in LiDAR ownership stays frozen.

That is the minimal C→D Apply connectivity correction.
