# Super-LIVO — Prompt81
## FAST-LIVO2 Semantic Migration M0 + D1
### Semantic Migration Ledger + Inverse-Exposure State Plumbing

---

# 0. Role

You are the implementation executor.

Architecture authority for this round is **Origin**.

This is NOT a source-discovery round.

Origin has already completed the main FAST-LIVO2 ↔ Super-LIVO source audit.

Do not spend the round independently redesigning the Visual architecture.

Your responsibility is:

```text
implement exactly the bounded semantic migration authorized below
+
prove the implementation through production-relevant tests
+
record the migration in the canonical scorecard
+
STOP
```

Do NOT advance to D2.

---

# 1. Project-State Consensus — HARD START GATE

Repository:

```text
Scar-c/Super-LIO
branch: super-livo
```

Expected starting frontier:

```text
001c9844e6b93e1752626ef500e390b0a6678ad9
```

Before modifying anything, report:

```text
executor:
branch:
expected HEAD:
actual HEAD:
origin/super-livo HEAD:
git status --short:
git diff --check:
WIP present:
frontier verified:
```

Required:

```text
actual HEAD == expected HEAD
or
expected HEAD is proven ancestor of the exact Owner-approved frontier
```

If the remote/frontier differs unexpectedly:

```text
STOP_FOR_OWNER_FRONTIER_MISMATCH
```

Do not rebase, reset, cherry-pick, or guess.

Preserve unrelated WIP.

---

# 2. Architecture Authority

Reference implementation:

```text
FAST-LIVO2
pinned commit: 0d2c034
```

Core LIVO visual semantics authority:

```text
FAST-LIVO2
```

Geometry/map computational substrate authority:

```text
Super-LIO
```

This project is currently performing:

```text
FAST-LIVO2 semantic migration
```

NOT:

```text
new visual architecture research
parameter optimization
ATE-guided redesign
S3 ablation
FEJ development
```

---

# 3. Owner-Frozen Architecture Decisions

The following are authoritative and must NOT be reopened in this round.

## 3.1 Sequential update

```text
IMU propagation
→ LiDAR update
→ freeze LiDAR posterior
→ Camera Visual update
```

Phase A/B are already closed.

Do not redesign camera-event placement.

Do not redesign LiDAR ownership.

Do not reopen scheduler semantics without concrete contradictory production evidence.

---

## 3.2 Visual update operator

Origin source audit conclusion:

```text
Super-LIVO UpdateObserveFromPrior()
pose-only prior-correction algebra
=
FAST-LIVO2 visual pose-update algebra

classification:
BACKEND_TRANSLATION_EQUIVALENT
```

Do NOT rewrite this merely because the code looks different.

Future migration will change:

```text
measurement model
exposure state
pyramid
iteration control
rollback
covariance commit timing
```

but not gratuitously replace the valid prior-correction structure.

---

# 4. Two Owner-Authorized Deviations

These are deliberate Super-LIVO backend differences.

They are NOT parity bugs.

## DEV-POS-REP — centroid-relative Visual landmark representation

FAST-LIVO2 semantics require a stable physical world landmark.

Super-LIVO is authorized to store it as:

```text
p_W = centroid_W + offset_W
```

instead of requiring a standalone absolute VisualPoint position field.

Hard invariant:

```text
centroid changes
→ offset must change consistently
→ reconstructed p_W must remain physically invariant
```

unless the landmark itself is intentionally updated by authorized lifecycle semantics.

The centroid is storage/reference representation.

It must NOT silently drag the Visual landmark through world space.

---

## DEV-POINT-COV — no point-position uncertainty model

Do NOT reproduce FAST-LIVO2 point-position covariance / point uncertainty machinery.

Forbidden:

```text
VisualPoint position covariance
pointWithVar-style visual covariance propagation
new per-landmark covariance matrices
new uncertainty weighting derived from point covariance
```

unless explicitly authorized by Owner in a future round.

Visual measurement covariance is a different concept and will be handled later.

---

# 5. Semantic Migration Roadmap

Record this roadmap in the canonical migration ledger:

```text
M0
semantic migration scorecard infrastructure

D1
inverse-exposure filter-state plumbing

D2
FAST-LIVO2 exposure-aware photometric residual/J/weight

D3
pyramid + coarse-to-fine iteration
relinearization
accept/rollback
single final covariance commit

D4
FAST-LIVO2 normal/plane-aware warp semantics

D5
within-frame Visual semantic closure

E1
VisualPoint generation / creation semantics

E2
observation + reference selection/update semantics

E3
Visual map update / normal refinement / lifetime / raycast lifecycle

E4
full LIVO2_COMPAT_BASELINE closure

F
S3 spatial-balanced map ablation

G
final map policy selection
```

This prompt authorizes only:

```text
M0 + D1
```

---

# 6. M0 — Canonical Semantic Migration Scorecard

Create the canonical semantic migration record.

Recommended canonical paths:

```text
docs/super_livo/evidence/livo2_semantic_migration_scorecard.md
docs/super_livo/evidence/livo2_semantic_migration_scorecard.json
```

If the repository has a better existing canonical evidence hierarchy, use it consistently and report the exact paths.

The Markdown and machine-readable representation must agree.

---

# 7. Required Scorecard Schema

Every migration stage must have at least:

```text
stage_id
date
starting_commit
ending_commit

fast_livo2_reference_commit
fast_livo2_target_functions/files

semantic_target

super_livo_before
super_livo_after

classification_before
classification_after

authorized_deviations

production_path_changed
production_files

test_evidence
real_seam_evidence
negative_mutation_evidence

dataset
sequence
ATE
ATE_evaluator

visual_apply_count
visual_residual_per_frame
visual_iterations_per_frame
visual_rollback_rate
inverse_exposure_statistics
visual_cpu_ms

visual_map_points
memory

regressions
open_gaps

verdict
next_authorized_stage
```

Fields that do not apply must explicitly say:

```text
N/A
```

Never silently omit them.

---

# 8. Scorecard Classification Vocabulary

Use only explicit classifications such as:

```text
EXACT_PARITY
BACKEND_TRANSLATION_EQUIVALENT
OWNER_AUTHORIZED_DEVIATION
PARTIAL_MIGRATION
MISSING_FAST_LIVO2_SEMANTIC
LEGACY_SUPER_LIVO_SEMANTIC
BLOCKED
UNRESOLVED
CLOSED
```

Do not use vague labels such as:

```text
mostly same
seems okay
probably equivalent
close enough
```

---

# 9. M0 Historical Baseline Entry

The scorecard must contain the current pre-migration state.

At minimum record:

```text
A:
camera-event Shadow
CLOSED

B:
camera-event Apply
CLOSED
```

Current Visual semantic state before D migration:

```text
LiDAR posterior → Visual prior:
BACKEND_TRANSLATION_EQUIVALENT

pose-only iterative prior correction:
BACKEND_TRANSLATION_EQUIVALENT

photometric residual:
LEGACY_SUPER_LIVO_SEMANTIC
DC / zero-mean patch residual

inverse exposure state:
MISSING_FAST_LIVO2_SEMANTIC

exposure-aware residual:
MISSING_FAST_LIVO2_SEMANTIC

pyramid:
MISSING / LEGACY

FAST-LIVO2 photometric rollback:
MISSING_FAST_LIVO2_SEMANTIC

FAST-LIVO2 covariance timing:
NOT YET REPRODUCED

normal/plane warp:
DIFFERENT SEMANTIC

reference lifecycle:
DIFFERENT SEMANTIC

visual-map lifecycle:
DIFFERENT SEMANTIC
```

Do NOT claim:

```text
LIVO2_COMPAT_BASELINE
```

---

# 10. Historical Accuracy Evidence

Carry forward, do not rerun merely for this scorecard:

```text
A2 official NTU APE RMSE:
0.104098 m

corrected B0 official NTU APE RMSE:
0.133707 m
```

Evaluator authority:

```text
NTU official-style prism-compensated evaluator
```

The old naive ~3 mm result is NOT canonical.

Record:

```text
B accuracy regression:
OBSERVED

causal attribution:
NOT ESTABLISHED
```

Do not claim one specific D gap caused the regression without evidence.

---

# 11. D1 Semantic Target

FAST-LIVO2 includes inverse exposure as a real filter state.

D1 shall reproduce that **state ownership and covariance plumbing**.

D1 shall NOT yet change the Visual photometric measurement model.

At the end of D1:

```text
inverse exposure exists as a first-class state
+
participates in covariance
+
has process uncertainty semantics
+
can be copied/frozen/differenced/applied consistently
```

but:

```text
current DC Visual residual remains unchanged
```

until D2.

Therefore D1 is intentionally:

```text
PARTIAL_MIGRATION
```

---

# 12. D1 State Requirement

Introduce a first-class inverse exposure state:

```text
inv_expo_time
```

Semantically:

```text
tau = inverse exposure
```

The backend memory/index layout does NOT have to use the same numeric index as FAST-LIVO2.

Prefer the least invasive layout for Super-LIO.

However all state operations must agree on one canonical index.

Required operations include all relevant equivalents of:

```text
construction / initialization
copy
assignment
boxplus / state addition
boxminus / state difference
prior snapshot
posterior state
covariance
propagation
reset where relevant
serialization/debug access where relevant
```

No split-brain representation is allowed.

There must not be:

```text
one inv_expo in Visual code
+
another inv_expo in filter state
```

with ambiguous authority.

---

# 13. D1 Initial Semantics

Use a clearly defined initial inverse exposure.

Do not invent dataset-specific tuning.

The initial-value semantics must be documented.

FAST-LIVO2-compatible migration must support the equivalent concept of:

```text
inv_expo_time initialized to a defined value
```

If an existing exposure-time initialization parameter is introduced, distinguish:

```text
initial state
vs
process covariance
vs
measurement estimation enable
```

Do not overload one parameter for all three.

---

# 14. D1 Covariance Ownership

Inverse exposure must have a real covariance entry.

It must not be a constant scalar floating outside the ESKF covariance.

Required invariant:

```text
P_expo_expo >= 0
finite
```

and exposure cross-covariance dimensions must be structurally valid.

Do not create point-position covariance.

---

# 15. D1 Propagation Semantics

Implement the FAST-LIVO2 semantic intent:

```text
inverse exposure behaves as a random-walk state
```

During propagation, when exposure estimation is enabled, its process covariance shall receive the equivalent of:

```text
Q_expo ∝ inv_expo_cov * dt^2
```

using the repository's canonical covariance convention.

When exposure estimation is disabled:

```text
exposure process-noise injection = 0
```

unless existing architecture requires an exactly documented alternative.

No coupling into physical IMU kinematics is authorized.

Exposure state must NOT alter:

```text
rotation propagation
position propagation
velocity propagation
gyro bias propagation
accel bias propagation
gravity propagation
```

in D1.

---

# 16. Preserve Existing Physical-State Semantics

D1 is state plumbing only.

The original physical states must keep their existing meanings.

Do not shift indexes casually without updating every consumer.

Preferred implementation strategy:

```text
central canonical state-layout definition
+
compile-time/static dimension checks where practical
```

Avoid magic numeric state indices scattered through production code.

If changing the full filter dimension exposes hard-coded dimensions, fix them systematically.

Do not paper over them with casts or truncated blocks.

---

# 17. D1 Visual Measurement Must Stay Legacy

Hard requirement:

```text
DO NOT change runVisualResidual()
photometric residual semantics
Jacobian semantics
patch semantics
normal warp semantics
pyramid semantics
reference semantics
visual-map lifecycle
```

in this round except minimal mechanical adaptations needed for the new state dimension.

Current DC residual remains the production measurement for this round.

No exposure Jacobian yet.

No 7-column Visual measurement yet.

That is D2.

---

# 18. D1 LiDAR Must Ignore Exposure Measurement Dimension

LiDAR update remains the current Super-LIO geometry estimator.

Do not add fake LiDAR sensitivity to exposure.

Required:

```text
∂r_lidar / ∂inv_expo = 0
```

structurally.

The exposure state may carry covariance through the filter, but LiDAR geometry residual does not directly observe it.

---

# 19. SequentialPrior Must Include the New State Correctly

The Phase-B frozen LiDAR posterior remains the Visual prior.

Therefore the existing equivalent of:

```text
SequentialPrior
```

must preserve:

```text
full state
full covariance
including inverse exposure
```

after D1.

No partial 18D prior snapshot followed by a 19D Visual update is allowed.

---

# 20. D1 Hard Gate A — State Algebra

Semantic invariant:

```text
state copy / plus / minus / snapshot
correctly preserve inv_expo
```

Authoritative path:

```text
actual production state type and ESKF operators
```

Required deterministic tests:

1. construct state with nontrivial pose/velocity/bias/gravity/exposure;
2. copy preserves all components;
3. state + delta updates exposure at the correct component;
4. state difference recovers exposure delta;
5. pose-only delta does not modify exposure;
6. exposure-only delta does not modify physical state.

Negative mutations that test logic must catch include at least:

```text
wrong exposure index
exposure omitted from state difference
exposure omitted from state addition
```

Gate must fail under those mutations.

---

# 21. D1 Hard Gate B — Covariance Dimension / Identity

Semantic invariant:

```text
inverse exposure is part of the real estimator covariance
```

Required proof:

```text
full state dimension consistent
P rows == state dimension
P cols == state dimension
finite
symmetric within numerical tolerance
```

and:

```text
exposure variance is readable/writable through canonical state index
```

Forbidden substitute:

```text
separate standalone exposure variance variable
```

that is not part of production P.

Negative mutation:

```text
remove exposure covariance dimension
```

must fail the gate.

---

# 22. D1 Hard Gate C — Process Noise

Semantic invariant:

```text
exposure random-walk uncertainty is propagated only when enabled
```

Use a deterministic propagation fixture with known:

```text
P0
dt
inv_expo_cov
```

Verify the exposure variance increment matches the implemented convention.

Required two cases:

```text
enabled
disabled
```

Negative mutations:

```text
wrong dt power
wrong covariance index
noise injected while disabled
```

must fail.

Do not accept a test that only checks:

```text
P changed
```

It must check the correct state/index and expected value.

---

# 23. D1 Hard Gate D — Physical-State Isolation

Semantic invariant:

```text
adding dormant exposure state does not alter existing physical-state propagation/update semantics
```

Required deterministic test:

same initial physical state
same IMU input
same LiDAR measurement fixture
same config

Compare:

```text
legacy physical-state result
vs
D1 exposure-disabled result
```

At minimum compare:

```text
R
p
v
bg
ba
g
physical covariance block
```

Tolerance must be justified from numeric implementation.

Prefer exact/bitwise where architecture permits.

If changing matrix dimension prevents bitwise equality, report numerical differences explicitly and use a tight, principled tolerance.

Do not silently loosen tolerance until PASS.

---

# 24. D1 Hard Gate E — Sequential Prior Integrity

Semantic invariant:

```text
Visual prior snapshot contains the exact LiDAR posterior
including exposure state/covariance
```

Required real-code seam test:

```text
produce LiDAR posterior
→ snapshot SequentialPrior
→ inspect state/P identities
```

Required identities:

```text
prior.x physical state == LiDAR posterior physical state
prior.inv_expo == LiDAR posterior inv_expo
prior.P == LiDAR posterior P
```

within the relevant exact/numeric representation.

Forbidden substitute:

```text
constructing an unrelated synthetic SequentialPrior
```

without exercising the production producer.

---

# 25. D1 Hard Gate F — No Premature D2

Automated/static checks shall demonstrate that D1 did NOT silently introduce:

```text
FAST-LIVO2 raw exposure residual
exposure Jacobian column
pyramid solve
photometric rollback
FAST-LIVO2 plane homography
new reference scoring
map lifecycle rewrite
point covariance
```

If any appeared unintentionally:

```text
FAIL_SCOPE_VIOLATION
```

---

# 26. No ATE-Guided Tuning

D1 is not a precision experiment.

Do NOT tune:

```text
img_point_cov
inv_expo_cov
outlier threshold
patch size
pyramid levels
IESKF iterations
LiDAR params
camera params
```

to recover ATE.

Use reference semantics, not performance feedback, to implement the state.

---

# 27. Runtime Policy

Do not run a full dataset merely because code changed.

First execute:

```text
unit tests
state algebra tests
covariance tests
propagation tests
production seam test
existing relevant regression suite
```

A short bounded production-like run is authorized ONLY if needed to prove:

```text
new state dimension does not break the real ROS/offline production path
```

If performed:

```text
one bounded run
no parameter sweep
no repeated ATE search
```

Heavy diagnostics OFF by default.

---

# 28. Scorecard Update After D1

After D1, append a new scorecard entry.

Expected conceptual result:

```text
inverse exposure filter state:
MISSING_FAST_LIVO2_SEMANTIC
→ PARTIAL_MIGRATION / REPRODUCED_STATE_PLUMBING

exposure covariance:
MISSING
→ REPRODUCED

exposure propagation:
MISSING
→ REPRODUCED

exposure-aware visual residual:
MISSING
→ STILL MISSING / D2

DC residual:
LEGACY_SUPER_LIVO_SEMANTIC
→ UNCHANGED

pyramid:
UNCHANGED

rollback:
UNCHANGED

normal warp:
UNCHANGED

map lifecycle:
UNCHANGED
```

Do not write:

```text
FAST-LIVO2 Visual parity achieved
```

---

# 29. Scorecard Owner Deviations

Every scorecard snapshot must carry:

```text
OWNER_AUTHORIZED_DEVIATIONS:

DEV-POS-REP
Visual world point stored as centroid + offset.
World-point identity must remain invariant.

DEV-POINT-COV
No FAST-LIVO2 per-point / VisualPoint position covariance model.
```

These must remain visible through D/E so later agents do not "fix" them.

---

# 30. Migration Scorecard Performance Fields

Populate all fields.

For D1, likely:

```text
ATE:
N/A — not required for state-plumbing semantic gate

visual residual/frame:
carry-forward / N/A

visual iterations/frame:
carry-forward / N/A

rollback rate:
N/A — D3 not implemented

inverse exposure:
initial value
variance
process-noise test values

CPU:
N/A unless bounded runtime seam was executed
```

Never fabricate values.

---

# 31. Prompt Registration

Register this exact Owner prompt in canonical prompt history.

This is:

```text
Prompt81
```

Record that the earlier proposed broad Phase-C Prompt80 was:

```text
ABORTED_BEFORE_EXECUTION
SUPERSEDED_BY_ORIGIN_SOURCE_AUDIT
```

Do not treat Prompt80 as an executed round.

Update:

```text
Super-LIO/prompts/
prompts/README.md
active .scratch/super-livo-v1/issues/ tracker
```

with exact canonical paths.

---

# 32. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
```

Use `/grill-with-docs` only if repository/source documentation conflicts materially with production behavior.

Do not report a skill as used unless actually used.

---

# 33. TDD Contract

Write failing tests before the corresponding production implementation where practical.

At minimum cover:

```text
exposure state index
state plus/minus
copy/snapshot
covariance dimension
exposure variance
enabled propagation
disabled propagation
physical-state isolation
SequentialPrior integrity
scope guard
```

Do not write tests that merely duplicate the implementation line-for-line.

Prefer independent invariants.

---

# 34. Production-Code Review Before Commit

Before committing, inspect every modified production path for:

```text
hard-coded state dimensions
hard-coded state offsets
Eigen block ranges
serialization layouts
covariance blocks
process-noise dimensions
reset/init paths
prior snapshot paths
LiDAR observation paths
Visual observation paths
```

Search the entire repository for old state-dimension assumptions.

Do not patch only the first compiler error.

---

# 35. Forbidden Changes

This round must NOT implement:

```text
D2 residual migration
D3 pyramid
D3 rollback
D4 plane warp
D5 closure
E visual-map lifecycle
S3
FEJ
new map selection
point covariance
ATE tuning
parameter sweep
```

No new research feature.

---

# 36. Commit Structure

Prefer separate commits:

```text
1. Prompt81 + M0 scorecard schema
2. D1 tests
3. D1 production state/covariance plumbing
4. D1 scorecard/evidence closure
```

Do not mix unrelated refactors.

Every persistent artifact must be committed.

No `/tmp`-only canonical evidence.

---

# 37. Spinner-Safe Execution Hygiene

One bounded build/test/experiment per shell invocation.

When piping:

```bash
set -o pipefail
```

Always preserve actual return code.

Use explicit completion sentinels.

Do NOT rerun just because the OpenCode UI is still spinning.

First inspect whether the real process still exists.

No uncontrolled duplicate rosbag/ROS/test processes.

---

# 38. STOP Conditions

Immediately STOP_FOR_OWNER on:

```text
unexpected branch/frontier mismatch

production state layout ambiguity that cannot be resolved without
changing frozen physical-state semantics

need to modify LiDAR measurement semantics

need to alter camera scheduler / Phase-B event placement

need to add point-position covariance

source evidence contradicts Origin's frozen D1 target

D1 requires implementing D2 semantics to compile correctly
and no clean interface boundary exists

physical-state regression cannot be explained

filter covariance becomes non-finite / non-symmetric

canonical scorecard cannot mechanically identify the exact
production commit being scored
```

Do not solve these by redesigning architecture.

---

# 39. D1 CLOSE Criteria

D1 may close only when ALL are true:

```text
M0 canonical scorecard exists

Prompt81 registered

inverse exposure is first-class estimator state

state algebra handles it correctly

full covariance owns it

propagation noise semantics implemented

exposure-disabled physical state remains equivalent

SequentialPrior preserves full new state/P

LiDAR has zero direct exposure sensitivity

current DC Visual measurement remains unchanged

no point covariance added

DEV-POS-REP remains intact

all required tests PASS

negative mutations are proven detectable

production build PASS

persistent evidence committed

worktree clean

D2 NOT STARTED
```

---

# 40. Final Report — REQUIRED FORMAT

Report exactly:

```text
Round:
Prompt81 — M0 + D1

Initial HEAD:
Final HEAD:

=== Agent State Consensus ===
executor:
branch:
expected:
actual:
origin:
frontier verified:
WIP:

=== Architecture Freeze ===
FAST-LIVO2 authority:
Super-LIO geometry authority:
Phase A reopened:
Phase B reopened:
D2 started:
E started:
point covariance added:

=== Owner-Authorized Deviations ===
DEV-POS-REP:
DEV-POINT-COV:

=== M0 Scorecard ===
markdown path:
machine-readable path:
schema:
legacy baseline entered:
Prompt80 status recorded:

=== D1 State Layout ===
old dimension:
new dimension:
inv_expo canonical index:
initial value:
state plus/minus:
copy:
snapshot:

=== D1 Covariance ===
P dimension:
initial exposure variance:
process covariance parameter:
enabled propagation equation:
disabled behavior:
symmetry:
finite:

=== LiDAR Isolation ===
direct exposure Jacobian:
physical-state regression:
physical covariance regression:

=== Sequential Prior ===
full state captured:
exposure captured:
full P captured:
production seam exercised:

=== Hard Gates ===
D1-A State Algebra:
D1-B Covariance:
D1-C Process Noise:
D1-D Physical Isolation:
D1-E Sequential Prior:
D1-F Scope Guard:

=== Negative Mutations ===
wrong exposure index:
missing plus/minus:
missing covariance dimension:
wrong dt power:
noise while disabled:

=== Tests ===
build:
unit:
regression:
bounded runtime seam:
bag run:
ATE:

=== Semantic Migration Scorecard Delta ===
inverse exposure state:
exposure covariance:
exposure propagation:
exposure-aware residual:
DC residual:
pyramid:
rollback:
normal warp:
reference lifecycle:
map lifecycle:

=== Changed Files ===
<git diff --name-status INITIAL..HEAD>

=== Commits ===
<git log --oneline INITIAL..HEAD>

=== Architecture Deviations ===
NONE
or exact authorized deviation reference

=== Final Classification ===
ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED
or
STOP_FOR_OWNER_<reason>

=== Next Stage ===
D2 AUTHORIZED:
NO

Await Owner review.
```

---

# 41. Final Principle

This round is not trying to make the trajectory better.

It is migrating one known semantic block:

```text
FAST-LIVO2 exposure state ownership
```

into Super-LIVO without disturbing unrelated semantics.

The project progresses by:

```text
one semantic block
→ prove it
→ score it
→ commit it
→ Owner review
→ next semantic block
```

not by:

```text
large rewrite
→ run ATE
→ guess what went wrong
```