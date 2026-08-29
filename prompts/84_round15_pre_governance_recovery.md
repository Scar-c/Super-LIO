# Super-LIVO — Prompt84
## Pre-Governance Recovery + Correct Camera-Epoch Sequential LIVO Reconstruction
### Recover the last clean production architecture, remove Prompt59+ governance from the active development line, and restore FAST-LIVO2-consistent camera-epoch / reference lifecycle semantics

---

# 0. Role

You are the implementation executor.

Origin has already completed the architecture/source-level decision.

Your job is:

```text
inspect git history mechanically
recover a clean production frontier
reconstruct only the approved production semantics
run minimal real seam validation
commit the reconstruction cleanly
STOP
```

You are NOT authorized to redesign the architecture.

You are NOT authorized to reconstruct the previous supervisor/profile/validator/testing framework.

You are NOT authorized to restart D1/D2 exposure migration in this round.

---

# 1. Why This Recovery Exists

The historical `super-livo` branch accumulated excessive engineering-governance infrastructure beginning around the Prompt59/60 period:

```text
transaction supervisor
normalized semantic profile
generic validator layer
round-specific runner semantics
large test/mutation frameworks
test-facing production seams
state-machine governance
```

These layers eventually began to constrain or reinterpret estimator semantics.

This reconstruction changes the project rule to:

```text
FAST-LIVO2 / Super-LIO source semantics
        ↓
production architecture
        ↓
small tests verify semantics
```

and explicitly rejects:

```text
tests / profiles / supervisor
        ↓
dictate estimator architecture
```

---

# 2. Historical Branch Preservation — HARD GATE

The existing branch:

```text
super-livo
```

is now historical evidence.

Do NOT:

```text
reset it
rewrite it
delete its tests
rebase it
force-push it
clean it up destructively
```

Record mechanically:

```text
git rev-parse super-livo
git rev-parse origin/super-livo
git status --short
git log -10 --oneline super-livo
```

Also record the latest historical frontier.

Do not assume the SHA written in this prompt is still the latest if additional Prompt82 delivery commits exist.

Report:

```text
HISTORICAL_SUPER_LIVO_HEAD=<full SHA>
```

---

# 3. New Active Development Branch

Create a new branch:

```text
super-livo-rebuild
```

The new branch must NOT start from the current historical `super-livo` HEAD.

The intended recovery candidate is:

```text
8321586
```

because historical Round13 evidence explicitly identified it as the:

```text
accepted production frontier
```

before the quarantined placement/payload/apply changes.

However, before creating the branch, perform the mechanical contamination audit in §4.

---

# 4. Recover the Last Pre-Governance Production Frontier

Do NOT decide from Prompt number alone.

Audit `8321586` directly.

At minimum inspect:

```bash
git show --stat 8321586
git ls-tree -r --name-only 8321586
git log --oneline --decorate --graph -30 8321586
```

Search at that revision for governance-layer artifacts such as:

```text
run_superlivo_transaction.sh
semantic_profiles.py
normalized semantic profile
transaction supervisor
profile-specific validator
generic result validator
D_VISUAL_SHADOW profile
D_VISUAL_APPLY profile
round-specific transaction state machine
```

Also inspect whether any of these already influence estimator production code.

Classification must be exactly one:

```text
8321586 = CLEAN_PRE_GOVERNANCE_FRONTIER
```

or

```text
8321586 = PRODUCTION_CLEAN_BUT_RUNNER_GOVERNANCE_PRESENT
```

or

```text
8321586 = GOVERNANCE_CONTAMINATED
```

If `8321586` contains only incidental scripts but estimator production is still independent, it may still be used as the production reconstruction base, provided those scripts are NOT carried forward into the new development architecture.

If governance already materially changes estimator ownership/API:

```text
STOP_FOR_OWNER_RECOVERY_FRONTIER_AMBIGUOUS
```

Do not guess another base automatically.

---

# 5. Important Historical Fact — 8321586 Is NOT Yet Full Correct Camera-Epoch Visual Semantics

Origin has already audited the relevant historical D path.

At the accepted frontier, the camera event performs approximately:

```text
camera event t_c
    ↓
statePropagateOnly
    ↓
PropagateTo(t_c)
    ↓
CommitPropagationOnlyEpoch
    ↓
return
```

Thus:

```text
IMU propagation to camera epoch:
YES
```

but the actual Visual measurement historically remained in a later full-LiDAR Observe callback.

Therefore:

```text
Visual update exactly at camera epoch:
NO / NOT YET CLOSED
```

Do NOT report `8321586` itself as a finished LIVO2-compatible scheduler.

It is the clean production base from which the placement must be corrected.

---

# 6. FAST-LIVO2 Authority — Event Semantics

Pinned visual architecture authority:

```text
FAST-LIVO2
commit: 0d2c0346107b75b59934975adec9a6eeeb913c64
```

Origin has already established the sequential Bayesian semantic:

```text
IMU-propagated prior
        ↓
LiDAR update
        ↓
x_L, P_L
        ↓
Camera update
        ↓
x_LC, P_LC
```

At a valid LIVO camera event:

```text
Visual prior = converged LiDAR posterior
```

not:

```text
an arbitrary later LiDAR state
```

and not:

```text
a Visual iterate recaptured as a new prior
```

Do NOT re-audit this architecture from scratch.

Use it as authoritative implementation guidance.

---

# 7. Target Camera-Epoch Semantics

The recovered production path shall ultimately express:

```text
camera event t_c
        ↓
consume required IMU interval
        ↓
construct the LiDAR measurement group associated with t_c
        ↓
LiDAR update
        ↓
freeze x_L(t_c), P_L(t_c)
        ↓
Visual update at the SAME camera event
        ↓
x_LC(t_c), P_LC(t_c)
        ↓
post-Visual lifecycle
        ↓
later propagation starts from x_LC/P_LC
```

Important:

Merely doing:

```text
IMU DR → t_c → Visual
```

is NOT enough to claim FAST-LIVO2 parity if the corresponding LiDAR posterior at that event has not been established.

The semantic target is:

```text
LIO @ event
→ VIO @ same event
```

---

# 8. Historical Commits To Inspect — READ ONLY

Historical evidence identified these quarantined production commits:

```text
33c1b3d
7d9be50
ce3d1a9
```

They contained some mixture of:

```text
camera-epoch placement
payload lifetime
no-pop accounting
D-family Apply gating
```

Inspect them using:

```bash
git show --stat <sha>
git show <sha> -- src/super_lio/
git diff <sha>^..<sha> -- src/super_lio/
```

Your task is to build a table:

| Commit | Production files | Placement logic | Payload lifetime | Apply gate | Test/governance coupling | Reuse decision |
|---|---|---|---|---|---|---|

Classification per hunk:

```text
REUSE_SEMANTICALLY
REFERENCE_ONLY
REJECT_GOVERNANCE
REJECT_TEST_COUPLING
OBSOLETE
```

Do NOT cherry-pick these commits wholesale.

---

# 9. Historical TB Semantics To Preserve

The older tracer-bullet sequence already established several useful concepts.

Relevant semantics:

## TB-2 class

```text
camera/LiDAR epoch splitting
current vs future measurement ownership
camera-epoch propagation
```

## TB-3 class

Explicit state checkpoints conceptually including:

```text
x_prop
x_lio
P_lio
```

and zero-information second-update equivalence.

## TB-9 class

First actual MODE-A sequential Visual state feedback:

```text
Visual prior = x_L, P_L
Visual measurement
→ state correction
```

This is the key historical production semantic to recover.

## TB-13 class

Final high-level ordering conceptually:

```text
IMU
→ LiDAR
→ Visual
→ resulting state
→ map/lifecycle
```

Do NOT copy TB tests or debug infrastructure.

Recover only production semantics that remain valid under the current FAST-LIVO2 authority.

---

# 10. New Production Architecture Must Stay Minimal

Prefer one clear production path.

Do NOT create:

```text
C-family scheduler
D-family scheduler
Shadow scheduler
Apply scheduler
legacy scheduler
camera-event scheduler variant
```

as parallel implementations.

There shall be one active semantic path.

Configuration may enable/disable camera usage, but configuration does not own algorithm architecture.

---

# 11. No Transaction Supervisor

The rebuild branch must NOT introduce:

```text
transaction supervisor
semantic profile resolver
algorithm-mode validator
profile-driven estimator ownership
```

Experiment scripts may launch processes.

They may NOT decide:

```text
where Visual runs
whether LiDAR is re-used
which estimator callback owns the measurement
which prior the camera update uses
```

Those are production estimator semantics.

---

# 12. Runner Boundary

If a runner is required, its responsibilities are only:

```text
resolve paths
set ROS parameters
launch estimator
play one bag
capture stdout/stderr
capture return code
clean only owned processes
```

It must not contain algorithm semantics.

Forbidden runner concepts:

```text
D_VISUAL_SHADOW
D_VISUAL_APPLY
normalized protected fields
semantic profile
algorithm-state validator
```

---

# 13. Visual Map Semantics — Important FAST-LIVO2 Clarification

Origin has re-audited FAST-LIVO2 Issue #263.

The issue asked why reference-patch calculation appears in BOTH:

```text
retrieveFromVisualSparseMap()
```

and:

```text
updateReferencePatch()
```

FAST-LIVO2 member response explains:

```text
updateReferencePatch() executes only after >5 observations.

Therefore some VisualPoints do not yet have a reference patch.

retrieveFromVisualSparseMap() needs safeguard/bootstrap behavior
for those points.
```

This is authoritative guidance for our migration.

---

# 14. Do NOT Misclassify This As “Build Visual Map Twice”

The FAST-LIVO2 lifecycle is NOT:

```text
build visual map
build visual map again
```

and it is NOT equivalent to:

```text
pre-solve
pre-solve again
```

within one optimizer transaction.

Correct conceptual separation:

```text
PRE-VISUAL UPDATE
retrieve/use existing VisualPoint
ensure a usable reference exists
        ↓
Visual solve
        ↓
POST-VISUAL UPDATE
generate/update observations
conditionally maintain mature reference
```

---

# 15. FAST-LIVO2 Reference-Patch Bootstrap / Maintenance Semantics

The future lifecycle interface must preserve room for two semantically different operations.

## A. Bootstrap / safeguard during retrieval

At:

```text
retrieveFromVisualSparseMap()
```

if a VisualPoint does not yet have a persistent reference:

```text
select/provide a usable reference
so the current frame can perform Visual alignment
```

This handles young VisualPoints with too few observations.

This is not a second map construction.

---

## B. Mature reference maintenance

At:

```text
updateReferencePatch()
```

only sufficiently observed VisualPoints are eligible.

Key upstream condition:

```text
observations > 5
```

plus the other upstream eligibility conditions.

This stage occurs after the current frame may have contributed a new observation.

Its purpose is long-term reference maintenance.

---

# 16. Current Round Scope For Reference Semantics

This recovery round does NOT require full E1–E4 Visual lifecycle migration.

However, the recovered architecture must NOT make future implementation impossible.

Therefore any VisualPoint/reference API reconstructed now must distinguish conceptually:

```text
bootstrap reference acquisition
```

from:

```text
mature reference refresh
```

Do NOT collapse both into a single “reference must always exist at construction” invariant.

Do NOT require `obs > 5` before a point can participate in current-frame retrieval.

Do NOT run mature `updateReferencePatch()` twice to emulate bootstrap behavior.

---

# 17. Critical Exact-Once Rule

The FAST-LIVO2 Issue #263 behavior does NOT authorize duplicated measurement construction.

For one camera Visual transaction:

```text
Visual residual system is constructed once per intended
relinearization/optimizer iteration according to algorithm semantics.
```

Do not introduce:

```text
preSolve()
preSolve()
```

or:

```text
build H/b
build H/b again
```

merely because reference handling has two lifecycle sites.

These are separate concepts.

---

# 18. Camera Payload Lifetime

A camera payload required for current camera-epoch Visual processing must remain valid until:

```text
Visual measurement completes
+
required post-solve lifecycle using that frame completes
```

Do not pop/release the image before the last production consumer.

But do not retain it indefinitely.

Required ownership diagram in final report:

```text
arrival
→ accepted
→ epoch processing
→ Visual
→ post lifecycle
→ release
```

One owner at each stage.

No test/supervisor ownership.

---

# 19. Frozen LiDAR Posterior

Create/recover a clean production object or local transaction concept that represents:

```text
x_L
P_L
t_event
```

Name is implementation-dependent.

Historical `SequentialPrior` may be consulted but does not have to be copied.

Hard invariant:

```text
Visual iterations for one camera transaction use
the same LiDAR posterior prior.
```

If Visual later iterates:

```text
x_work changes
```

but:

```text
x_prior = x_L
P_prior = P_L
```

remain fixed for that transaction.

---

# 20. ESKF Update API

Historical:

```text
UpdateObserveFromPrior
```

may be inspected as a reference.

Do not blindly port test-driven interfaces.

Implement the smallest clean production API necessary to express:

```text
measurement update
using explicitly supplied frozen prior
```

Do not redesign the full Super-LIO ESKF.

---

# 21. Current Visual Residual Policy

This round is NOT a photometric-model migration round.

Do not invest time rebuilding the old DC residual architecture if it is absent at the selected clean recovery point.

If a historical residual already exists and is required only to prove the event placement seam, it may be temporarily retained unchanged.

But classify it:

```text
LEGACY_VISUAL_MEASUREMENT
NOT_FAST_LIVO2_PARITY
TO_BE_REPLACED_IN_D2
```

Do NOT tune it.

---

# 22. No D1/D2/D3/D4/E Migration Yet

Forbidden in this round:

```text
inverse exposure state migration
exposure Jacobian
FAST-LIVO2 raw exposure residual
img_point_cov migration
pyramid optimizer
photometric rollback
homography warp rewrite
point covariance
full VisualPoint lifecycle parity
S3
FEJ
```

Those resume only after clean recovery is accepted.

---

# 23. Tests — Minimal Policy

Old tests are not architecture authority.

The new branch should contain only a small set of durable tests.

Recommended categories:

## T1 — Clean LIO smoke

Camera disabled:

```text
baseline LIO still runs
state finite
map finite
```

## T2 — Camera event ordering seam

Prove:

```text
LiDAR posterior event
precedes
Visual transaction
```

## T3 — Frozen prior identity

Prove:

```text
camera Visual prior x == event LiDAR posterior x
camera Visual prior P == event LiDAR posterior P
```

## T4 — No premature image release

Prove payload valid through Visual/lifecycle consumer.

## T5 — Exact-once Visual transaction

Prove one accepted camera event does not trigger an accidental second independent Visual measurement transaction.

## T6 — Future reference lifecycle contract

Small structural test only:

```text
young VisualPoint may lack mature reference
retrieval path can bootstrap/safeguard
mature refresh remains separate
```

Do NOT implement full FAST-LIVO2 lifecycle merely for this test.

Target:

```text
small, readable test suite
```

not hundreds of cases.

---

# 24. Forbidden Test Architecture

Do NOT recreate:

```text
semantic profile TDD
transaction-supervisor TDD
round-number gates
mutation framework
fake algorithm producer
test-only state setter forest
test-only estimator modes
private-implementation lock-in
```

If production semantics are correct and an old test disagrees:

```text
old test is not migrated
```

---

# 25. Optional Runtime Validation

After build/unit/seam validation, one bounded NTU `eee_01` smoke run is authorized if needed.

Purpose only:

```text
prove actual camera events execute
prove no crash
prove LIO→Visual event chain exists
```

Do NOT use ATE as an optimization objective.

Do NOT tune parameters.

Do NOT run multiple datasets.

Do NOT run old transaction supervisor.

---

# 26. Runtime Evidence Required If Bag Is Run

Report only simple production counters:

```text
camera messages received
camera events accepted
LiDAR event updates
Visual transactions
payload releases
```

No giant instrumentation system.

Hard identities should be interpretable directly.

For example:

```text
accepted camera Visual event
→ exactly one Visual transaction
```

and:

```text
each Visual transaction
→ has a valid preceding event LiDAR posterior
```

---

# 27. Existing Historical Tests

Do NOT delete historical tests from the old `super-livo` branch.

They remain evidence.

The new reconstruction branch simply does not inherit governance-era tests unless explicitly justified.

This is:

```text
historical preservation
+
active architecture reset
```

not destructive history cleanup.

---

# 28. Reconstruction Documentation

Create one lightweight document:

```text
docs/super_livo/rebuild/
prompt84_recovery_architecture.md
```

It should contain:

```text
selected recovery base
why it is clean enough
what governance was intentionally left behind
camera-event semantic before
camera-event semantic after
historical commits inspected
production hunks conceptually reused
FAST-LIVO2 Issue #263 clarification
remaining D/E migration
```

No giant scorecard framework.

---

# 29. Commit Contract

Critical persistent work must be separated.

Do NOT use `git add .` or `git add -A` blindly.

Stage deliberately.

---

## Commit 84-1 — Recovery decision / Prompt

Contains:

```text
Prompt84 canonical copy
prompts/README.md
rebuild tracker
recovery frontier audit
```

Suggested:

```text
docs(rebuild): register Prompt84 pre-governance recovery
```

No estimator production changes.

---

## Commit 84-2 — Minimal clean tests

Contains only:

```text
small build/smoke/sequential seam tests
minimal CMake registration if required
```

Suggested:

```text
test(rebuild): add minimal camera-event sequential seam checks
```

No production algorithm fix hidden here.

---

## Commit 84-3 — KEY CAMERA-EPOCH PRODUCTION COMMIT

This is the critical functional commit.

Contains only the clean production reconstruction required to establish:

```text
event LiDAR posterior
→ frozen x_L/P_L
→ camera Visual transaction at same intended event
→ correct payload lifetime
→ later state continues from resulting posterior
```

Suggested:

```text
fix(rebuild): restore camera-epoch sequential LiDAR-to-Visual update
```

Final report MUST state:

```text
KEY CAMERA-EPOCH PRODUCTION COMMIT:
<full SHA>
```

Origin will independently inspect this commit.

---

## Commit 84-4 — Reference Lifecycle Contract

Only if production data structures/interfaces require a small change to avoid encoding the wrong “reference always exists” invariant.

Contains only minimal lifecycle-interface semantics:

```text
bootstrap reference may be absent/pending
mature refresh is a separate operation
```

Suggested:

```text
refactor(rebuild): separate visual reference bootstrap from mature refresh
```

Do NOT implement full E lifecycle.

If no production code change is required:

```text
Commit 84-4 = N/A
```

and document the contract only.

---

## Commit 84-5 — Evidence / Recovery Closure

Contains:

```text
reconstruction document
historical commit classification
minimal runtime evidence
changed-files audit
```

Suggested:

```text
docs(rebuild): close Prompt84 recovery evidence
```

---

## Commit 84-6 — Final Report / Tracker Close

Suggested:

```text
docs(rebuild): close Prompt84 camera-epoch recovery
```

If repository convention requires one post-push SHA-only metadata commit, it is allowed and must be identified separately.

---

# 30. Prompt Registration

Store exact Prompt84 text in canonical prompt history.

Use the repository's current prompt directory convention.

Update:

```text
prompts/README.md
```

and active tracker.

Record historical Prompt59+ governance work as:

```text
HISTORICAL / NOT MIGRATED INTO ACTIVE REBUILD ARCHITECTURE
```

Do not edit old prompt text retroactively.

---

# 31. Source Provenance Rule

Final implementation report must distinguish:

```text
Origin-established source semantic
```

from:

```text
historical implementation reused
```

For example:

```text
LiDAR→Visual sequential posterior:
FAST-LIVO2 authority

camera-event old Super-LIVO implementation:
historical implementation reference only

reference bootstrap safeguard:
FAST-LIVO2 source + Issue #263 member clarification
```

Do not claim historical Super-LIVO code is authority.

---

# 32. FAST-LIVO2 Issue #263 Required Record

Record the following semantic conclusion explicitly:

```text
ISSUE_263_REFERENCE_PATCH_SEMANTIC

retrieveFromVisualSparseMap:
bootstrap/safeguard reference availability
for points that may have <=5 observations
or otherwise lack mature reference

updateReferencePatch:
mature lifecycle reference maintenance
only after sufficient observations
(upstream >5 observation condition)

classification:
TWO_LIFECYCLE_SITES
NOT_DOUBLE_VISUAL_MAP_BUILD
NOT_DOUBLE_PRESOLVE
```

This wording must survive into future E-stage migration documentation.

---

# 33. No Premature “LIVO2_COMPAT_BASELINE” Claim

Even if this round fixes camera-event scheduling:

```text
LIVO2_COMPAT_BASELINE = NO
```

because the following remain unclosed:

```text
D1 exposure state
D2 exposure-aware residual
D3 pyramid transaction
D4 homography/search-level
D5 within-frame closure
E1-E4 map lifecycle
```

This recovery only restores a clean architectural substrate.

---

# 34. STOP Conditions

STOP_FOR_OWNER if:

```text
8321586 cannot be established as a usable clean production base

camera-epoch correction requires restoring transaction supervisor

the historical placement commits are inseparable from governance changes

correct same-event LIO→Visual semantics requires redesigning
the entire Super-LIO estimator

one camera event would require duplicate full LiDAR updates
without source justification

payload ownership cannot be made explicit

Visual prior cannot be frozen without test-only estimator APIs

fix requires starting D1/D2

reference bootstrap semantics require implementing full E lifecycle

camera-disabled clean LIO materially regresses
```

Do not broaden the round.

---

# 35. CLOSE Criteria

Prompt84 closes only if ALL are true:

```text
historical super-livo preserved

recovery base mechanically audited

new rebuild branch created from the accepted recovery base

Prompt59+ governance architecture not migrated

no transaction supervisor

no semantic profile

no generic algorithm validator

camera event reaches the intended event time

event LiDAR posterior is explicitly available

Visual transaction consumes that LiDAR posterior

Visual does not silently execute only at a later unrelated full-LiDAR callback

camera payload lives through required Visual/lifecycle use

Visual transaction exact-once holds

later estimator state continues from the resulting sequential posterior

camera-disabled LIO remains sane

Issue #263 bootstrap-vs-refresh semantic recorded correctly

no “double visual map build” implementation introduced

old DC residual not treated as FAST-LIVO2 parity

D1/D2 not started

minimal tests only

KEY CAMERA-EPOCH PRODUCTION COMMIT identified

local == remote

ahead/behind == 0/0

worktree clean
```

---

# 36. Final Classification

If PASS:

```text
SUPER_LIVO_PRE_GOVERNANCE_RECOVERY_AND_CAMERA_EPOCH_SEQUENTIAL_SCAFFOLD_CLOSED
```

Architecture status:

```text
clean sequential scaffold:
CLOSED

D1 clean migration:
NOT AUTHORIZED

D2:
NOT AUTHORIZED

LIVO2_COMPAT_BASELINE:
NO
```

Await Origin review.

---

# 37. Required Final Report

```text
Round:
Prompt84 — Pre-Governance Recovery + Camera-Epoch Sequential Reconstruction

=== Historical Branch ===
branch:
historical HEAD:
remote HEAD:
history modified:

=== Recovery Frontier Audit ===
candidate:
8321586 classification:
governance artifacts present:
estimator governed by them:
selected base:
reason:

=== Rebuild Branch ===
branch:
base:
initial HEAD:
final HEAD:
origin:
ahead/behind:
worktree:

=== Historical Commit Audit ===
33c1b3d:
  subject:
  production files:
  placement:
  payload:
  apply gate:
  reuse:

7d9be50:
  subject:
  production files:
  placement:
  payload:
  apply gate:
  reuse:

ce3d1a9:
  subject:
  production files:
  placement:
  payload:
  apply gate:
  reuse:

=== Before Recovery Event Semantics ===
camera arrival:
IMU propagation:
LiDAR update event:
Visual event:
payload release:
later full-LiDAR Observe:
Visual H/b location:

=== After Recovery Event Semantics ===
camera event:
IMU interval:
LiDAR measurement group:
x_L/P_L producer:
frozen-prior producer:
Visual consumer:
post-Visual state:
payload release:

=== Sequential Invariants ===
LIO before VIO:
same intended event:
Visual prior identity:
prior recaptured during iteration:
exact-once Visual transaction:
later propagation from Visual posterior:

=== FAST-LIVO2 Issue #263 ===
retrieve reference role:
updateReferencePatch role:
>5 observation meaning:
bootstrap safeguard:
mature refresh:
double visual map build:
double presolve:

=== Visual Map Scope ===
legacy residual:
map generation:
observation update:
reference bootstrap:
mature reference update:
full E lifecycle migrated:

=== Clean Architecture Audit ===
transaction supervisor:
semantic profiles:
generic validator:
round-specific estimator modes:
test-only production APIs:
heavy instrumentation:
duplicate scheduler ownership:

=== Tests ===
count:
build:
LIO smoke:
event-order seam:
frozen-prior seam:
payload lifetime:
exact-once:
reference lifecycle contract:
bag run:

=== Key Commits ===
Prompt/recovery audit:
minimal test commit:
KEY CAMERA-EPOCH PRODUCTION COMMIT:
reference lifecycle contract commit:
evidence commit:
final report commit:
delivery metadata commit:

=== Changed Files ===
<git diff --name-status BASE..HEAD>

=== Commits ===
<git log --oneline BASE..HEAD>

=== Architecture Deviations ===
NONE
or exact authorized backend representation

=== Final Classification ===
SUPER_LIVO_PRE_GOVERNANCE_RECOVERY_AND_CAMERA_EPOCH_SEQUENTIAL_SCAFFOLD_CLOSED
or
STOP_FOR_OWNER_<reason>

=== Next Stage ===
D1 clean migration authorized:
NO

D2 authorized:
NO

LIVO2_COMPAT_BASELINE:
NO

Await Origin independent review.
```

---

# 38. Final Rule

Do not rebuild the old bureaucracy.

The desired code should read approximately like the algorithm:

```text
synchronize event
→ propagate
→ LiDAR update
→ freeze LiDAR posterior
→ Visual update
→ lifecycle
→ continue
```

not:

```text
resolve profile
→ validate semantic manifest
→ supervisor state machine
→ adapter-selected mode
→ estimator hooks
→ post-run validator
```

The estimator owns algorithm semantics.

The runner only runs it.

Tests only verify it.