# Round 15 — Prompt82 — D1 Source-Parity Corrective Closure

```text
Round:
Round15 Prompt82 — D1 Source-Parity Corrective Closure

Initial HEAD:
c8c958465f3c19ad17b7d4a26f14a2956a6ed85d
Final HEAD:
9557f71b5c3d12cf447523a6d94bb36fe80dbbc8
```

```text
=== Agent State Consensus ===
executor:
agent-ds
branch:
super-livo
expected HEAD:
c8c958465f3c19ad17b7d4a26f14a2956a6ed85d
actual HEAD:
c8c958465f3c19ad17b7d4a26f14a2956a6ed85d
origin/super-livo:
c8c958465f3c19ad17b7d4a26f14a2956a6ed85d
ahead/behind:
0/0
worktree:
clean (only the loose Prompt82 copy at start)
frontier verified:
YES
```

```text
=== Source Authority ===
FAST-LIVO2 baseline commit:
0d2c0346107b75b59934975adec9a6eeeb913c64
inv_expo initial source:
inv_expo_time initial 1.0 (FAST-LIVO2)
P_expo initial source:
P_expo_expo = 1e-5 (FAST-LIVO2)
inv_expo_cov source/default:
vio/inv_expo_cov default 0.2 (FAST-LIVO2 source default)
NTU effective source:
vio/inv_expo_cov = 0.1 (NTU_VIRAL official; eee_01_tb0_offline.yaml
/lio/kf/kf_inv_expo_cov)
camera exposure_time_init source/meaning:
image capture timestamp adjustment — NOT PRESENT in Super-LIO as a migrated
parameter (/camera/time_offset fills the timestamp-adjustment role and is
mechanically separated from the inv_expo state)
```

```text
=== Prompt81 Preservation ===
Prompt81 modified:
NO (historical exact issued contract preserved verbatim)
Prompt80 status:
ABORTED_BEFORE_EXECUTION / SUPERSEDED_BY_ORIGIN_SOURCE_AUDIT (unchanged)
Prompt82 registration path:
prompts/07_round15_livo2_migration/82_round15_d1_source_parity_corrective_closure.md
```

```text
=== Corrective C1 — Initialization ===
inv_expo initial before:
1.0
inv_expo initial after:
1.0
P_expo before:
0.0
P_expo after:
1e-5 (kInitialInvExposureVariance, named constant, single init point)
production producer:
ESKF::SetInitialConditions (the real estimator initialization path)
test:
P82-T1/T2 (state 1.0, P 1e-5, finite, symmetric)
```

```text
=== Corrective C2 — Disabled Full Update ===
exposure enabled:
false
process noise:
0 (exact)
LiDAR update producer:
ESKF::UpdateObserveFromPrior (real production path, nonzero 6x6 HTRH)
finite posterior:
YES (P82-T5)
P symmetric:
YES (P82-T5)
P finite:
YES (P82-T5)
LiDAR exposure sensitivity:
ZERO — P_expo posterior == prior variance (nothing observes it);
inv_expo value unmodified by the update (P82-T6)
```

```text
=== Corrective C3 — Timing Separation ===
camera timing parameter:
g_camera_time_offset (/camera/time_offset) — timestamp adjustment only
timestamp consumer:
ROSWrapper::HandleImage (frame.timestamp = msg.stamp + offset)
inv_expo producer:
ESKF::SetInitialConditions/SetX/Update/Predict — consumes only
options_.inv_expo_initial_/inv_expo_cov_ (never the timing parameter)
cross-wiring found:
NONE (T7/T8 scans)
fix required:
NO
exposure_time_init parameter:
NOT PRESENT / NOT MIGRATED (reported per Prompt82 §10; not added this round)
```

```text
=== Corrective C4 — Config Provenance ===
generic/default inv_expo_cov:
0.2 (params.cpp g_kf_inv_expo_cov default; FAST-LIVO2 source default)
NTU effective inv_expo_cov:
0.1 (eee_01_tb0_offline.yaml /lio/kf/kf_inv_expo_cov: 0.1)
load producer:
ROSWrapper::LoadParamFromRos (nh.getParam /lio/kf/kf_inv_expo_cov)
runtime field:
g_kf_inv_expo_cov -> super_lio.cpp kf_init -> ESKF::Options::inv_expo_cov_
propagation consumer:
ESKF::Predict: P(18,18) += options_.inv_expo_cov_ * dt * dt
hard-coded override:
NONE (T9/T10: producer consumes the runtime field; literals absent)
```

```text
=== Corrective C5 — Point Cov Policy ===
old scorecard classification:
DEV-POINT-COV (no point-position uncertainty model; permanent deviation)
new classification:
POINT_COV_POLICY = DEFERRED_MIGRATION_TO_E3
D1-D5:
point covariance not implemented
E1:
interface/ownership reservable
E3:
FAST-LIVO2 point covariance active migration target (3-sigma plane
compat / normal refresh / uncertainty compatibility)
E4:
point-covariance parity reviewed before LIVO2_COMPAT_BASELINE
Prompt81 history preserved:
YES (verbatim; Prompt82 formally supersedes)
```

```text
=== Centroid-Offset Representation ===
changed:
NO
world-point identity contract:
p_W = centroid_W + offset_W; reconstructed identity must not drift
(OWNER_AUTHORIZED_BACKEND_REPRESENTATION)
```

```text
=== Hard Gates ===
C1 exact init:
PASS (P82-T1/T2)
C2 real disabled update:
PASS (P82-T5/T6)
C3 timing/state separation:
PASS (T7/T8 scans + production locations audit)
C4 config provenance:
PASS (T9/T10 chain)
C5 ledger:
PASS (scorecard D1_PROMPT82_CORRECTIVE entry)
SequentialPrior:
PASS (P82-T11 full 19D x/P)
18D physical isolation:
PASS (D1-D gates + full regression)
scope freeze:
PASS (no D2 functionality)
```

```text
=== Negative Mutations ===
M1 P_expo initial 1e-5 -> 0:
expected T1 fail; observed: P82-M1 + T1 assertion (constant > 0, == 1e-5)
M2 wrong exposure state index:
expected D1-A-NEG fail; observed: wrong-index reference breaks comparison
M3 exposure_time_init wired into inv_expo initial:
expected T7 fail; observed: T7 scan (init block never reads timing)
M4 inv_expo initial wired into image timestamp:
expected T8 fail; observed: T8 scan (HandleImage never references inv_expo)
M5 NTU config 0.1 ignored in favor of hard-coded 0.2:
expected T9/T10 fail; observed: producer consumes the runtime field;
literals absent
M6 process noise injected while exposure disabled:
expected D1-C-NEG fail; observed: disabled variance exactly unchanged
restored production state:
YES — no mutation remains in committed production code
```

```text
=== Tests ===
C++:
round15_d1_exposure_test (D1 gates A-F + P82 T1/T2/T5/T6/T9/T10/T11 + M1)
ALL PASS
Python:
test_round15_prompt82_corrective.py (T7/T8/T9/T10 + M3/M4/M5) 7 PASS
build:
PASS (catkin build super_lio)
regression:
316 python tests PASS (round14 + round13 + round15 suites; second full
run 316/316; first run had the known lock-contention flake 2/316)
production seam:
C++ production-seam: real init -> propagation -> LiDAR update (exposure
disabled) -> finite/symmetric posterior (P82-T5)
bag run:
NONE (not required; §19)
ATE:
N/A — carry-forward only (A2 0.104098 / B0 0.133707)
```

```text
=== Semantic Migration Scorecard ===
path md:
docs/super_livo/evidence/livo2_semantic_migration_scorecard.md
path json:
docs/super_livo/evidence/livo2_semantic_migration_scorecard.json
D1 original status:
ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED
Origin corrective:
D1_CORRECTIVE_REQUIRED
D1 final status:
CLOSED_AFTER_PROMPT82_CORRECTIVE
POINT_COV_POLICY:
DEFERRED_MIGRATION_TO_E3
D2 status:
NOT AUTHORIZED
```

```text
=== Key Commits ===
Prompt registration commit:
320075e0f82afd9938075a55ced4e04c2d9861db
TDD commit:
85d0de9 (test(round15): add D1 exposure parity and covariance-integrity gates)
KEY FUNCTIONAL COMMIT:
1138a02fd3edb98f59022cafb36428cf2455ed5a
ledger/evidence commit:
9557f71b5c3d12cf447523a6d94bb36fe80dbbc8
final report/tracker commit:
222da452143c06bd6537784fb282928f645bd47e
delivery metadata commit:
<pending>
```

```text
=== Changed Files ===
M	.scratch/super-livo-v1/issues/41-round14-phaseA-camera-epoch-shadow.md
M	docs/super_livo/evidence/livo2_semantic_migration_scorecard.json
M	docs/super_livo/evidence/livo2_semantic_migration_scorecard.md
A	prompts/07_round15_livo2_migration/82_round15_d1_source_parity_corrective_closure.md
M	prompts/README.md
A	scripts/super_livo/tests/test_round15_prompt82_corrective.py
M	src/super_lio/CMakeLists.txt
M	src/super_lio/include/lio/ESKF.h
M	src/super_lio/include/lio/params.h
M	src/super_lio/src/common/tests/test_round15_d1_exposure_state.cpp
M	src/super_lio/src/lio/ESKF.cpp
M	src/super_lio/src/lio/params.cpp
M	src/super_lio/src/lio/super_lio.cpp
M	src/super_lio/src/ros/ROSWrapper.cpp
```

```text
=== Commits ===
9557f71 docs(round15): close Prompt82 D1 corrective semantic ledger (P82-4)
1138a02 fix(round15): align D1 inverse-exposure initialization and config semantics (P82-3 KEY FUNCTIONAL)
85d0de9 test(round15): add D1 exposure parity and covariance-integrity gates (P82-2)
320075e docs(round15): register Prompt82 D1 source-parity corrective closure (P82-1)
```

```text
=== Architecture Deviations ===
NONE beyond the authorized backend representation (DEV-POS-REP /
centroid+offset) and the superseded-then-deferred point-covariance policy
```

```text
=== Final Classification ===
ROUND15_PROMPT82_D1_SOURCE_PARITY_CORRECTIVE_CLOSED
```

```text
=== Next Stage ===
D1 CLOSED:
CLOSED_AFTER_PROMPT82_CORRECTIVE
D2 AUTHORIZED:
NO

Await Origin independent review of the KEY FUNCTIONAL COMMIT
(1138a02fd3edb98f59022cafb36428cf2455ed5a).
```

## Delivery record (post-push)

Actual delivered remote HEAD: 222da452143c06bd6537784fb282928f645bd47e
Post-push local == remote: YES
ahead/behind: 0/0
