# Round 15 — Prompt81 — M0 + D1 (FAST-LIVO2 Semantic Migration)

```text
Round:
Prompt81 — M0 + D1

Initial HEAD:
001c9844e6b93e1752626ef500e390b0a6678ad9
Final HEAD:
<pending>
```

```text
=== Agent State Consensus ===
executor:
agent-ds
branch:
super-livo
expected:
001c9844e6b93e1752626ef500e390b0a6678ad9
actual:
001c9844e6b93e1752626ef500e390b0a6678ad9
origin:
001c9844e6b93e1752626ef500e390b0a6678ad9
frontier verified:
YES
WIP:
NONE (only the loose Prompt81 copy)
```

```text
=== Architecture Freeze ===
FAST-LIVO2 authority:
0d2c0346107b75b59934975adec9a6eeeb913c64 (visual semantics)
Super-LIO geometry authority:
preserved (LiDAR/IMU substrate untouched)
Phase A reopened:
NO
Phase B reopened:
NO
D2 started:
NO
E started:
NO
point covariance added:
NO
```

```text
=== Owner-Authorized Deviations ===
DEV-POS-REP:
intact (centroid + offset world-point representation untouched)
DEV-POINT-COV:
intact (no point-position covariance added)
```

```text
=== M0 Scorecard ===
markdown path:
docs/super_livo/evidence/livo2_semantic_migration_scorecard.md
machine-readable path:
docs/super_livo/evidence/livo2_semantic_migration_scorecard.json
schema:
§7 fields (all present; N/A explicit)
legacy baseline entered:
YES (M0 entry: A/B CLOSED; DC LEGACY; exposure MISSING; NOT
LIVO2_COMPAT_BASELINE)
Prompt80 status recorded:
ABORTED_BEFORE_EXECUTION / SUPERSEDED_BY_ORIGIN_SOURCE_AUDIT
```

```text
=== D1 State Layout ===
old dimension:
18
new dimension:
19
inv_expo canonical index:
18 (kInvExpoIndex, single source)
initial value:
1.0 (inv_expo_initial_; deterministic, P(18,18)=0)
state plus/minus:
BoxPlus/BoxMinus static operators (single canonical implementation;
ESKF::Update() refactored to use BoxPlus)
copy:
SetX/GetSysState preserve inv_expo
snapshot:
SequentialPrior/PosteriorSnapshot carry inv_expo
```

```text
=== D1 Covariance ===
P dimension:
19x19
initial exposure variance:
0.0 (deterministic init)
process covariance parameter:
options_.inv_expo_cov_ (distinct from initial state and enable flag)
enabled propagation equation:
P(18,18) += inv_expo_cov_ * dt^2 (repository bias-noise convention)
disabled behavior:
injection = 0 (exact)
symmetry:
P symmetric (0.5*(P+P^T) post-update; tests verify < 1e-4)
finite:
YES (tested)
```

```text
=== LiDAR Isolation ===
direct exposure Jacobian:
ZERO structurally (HTRH 6x6 block only; exposure row/col zero)
physical-state regression:
NONE (enabled-vs-disabled physical covariance block identical)
physical covariance regression:
NONE
```

```text
=== Sequential Prior ===
full state captured:
YES (19D via prior.x)
exposure captured:
YES (prior.x.inv_expo)
full P captured:
YES (prior.P 19x19 incl. P(18,18))
production seam exercised:
YES (UpdateObserveFromPrior on the production ESKF: posterior == prior for
zero information; exposure replaced by the new prior)
```

```text
=== Hard Gates ===
D1-A State Algebra:
PASS (copy/plus/minus/snapshot; exposure-only delta isolated;
pose-only delta isolated; reference==production)
D1-B Covariance:
PASS (dims/finite/symmetric/canonical-index read-write/non-negative)
D1-C Process Noise:
PASS (enabled: +cov*dt^2 exact; disabled: 0; physical block identical)
D1-D Physical Isolation:
PASS (deterministic IMU fixture; dormant exposure; finite state/P)
D1-E Sequential Prior:
PASS (physical state/inv_expo/P identities)
D1-F Scope Guard:
PASS (dim 19, index 18; no D2 semantics in the ESKF)
```

```text
=== Negative Mutations ===
wrong exposure index:
DETECTABLE (reference mutation at 17 breaks the comparison)
missing plus/minus:
DETECTABLE (omission breaks plus/minus equality)
missing covariance dimension:
DETECTABLE (state dim assertion)
wrong dt power:
DETECTABLE (linear-vs-quadratic expectation split)
noise while disabled:
DETECTABLE (disabled variance unchanged)
```

```text
=== Tests ===
build:
PASS (catkin build super_lio)
unit:
round15_d1_exposure_test ALL PASS (gates A-F + negative mutations)
regression:
269 python tests PASS; y_segmentation/s1_prior/v4c/v4r0/hb0/tb1/
round14_final_seal C++ ALL PASS
bounded runtime seam:
d1_bounded_production/20260829T093236Z — node rc=0, 19D filter through
kf_init/propagation/LiDAR/Visual (trajectory 1194 rows; 120s window is
Visual warmup — 0 residuals; validator evidence-volume failure only)
bag run:
ONE bounded run (authorized §27); NO full dataset run
ATE:
N/A — not required for the state-plumbing semantic gate
```

```text
=== Semantic Migration Scorecard Delta ===
inverse exposure state:
MISSING_FAST_LIVO2_SEMANTIC -> PARTIAL_MIGRATION (REPRODUCED_STATE_PLUMBING)
exposure covariance:
MISSING -> REPRODUCED
exposure propagation:
MISSING -> REPRODUCED
exposure-aware residual:
MISSING -> STILL MISSING (D2)
DC residual:
LEGACY_SUPER_LIVO_SEMANTIC -> UNCHANGED
pyramid:
UNCHANGED
rollback:
UNCHANGED
normal warp:
UNCHANGED
reference lifecycle:
UNCHANGED
map lifecycle:
UNCHANGED
```

```text
=== Changed Files ===
<git diff --name-status 001c984..HEAD — see commit>
```

```text
=== Commits ===
<git log --oneline 001c984..HEAD — see commit>
```

```text
=== Architecture Deviations ===
NONE beyond the two Owner-authorized deviations (DEV-POS-REP, DEV-POINT-COV)
```

```text
=== Final Classification ===
ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED
```

```text
=== Next Stage ===
D2 AUTHORIZED:
NO

Await Owner review.
```
