# Super-LIVO Round 11 Corrective — Freeze Architecture, Repair Visual Frontend Before MODE-A

## 0. STOP V-4 FIRST

Current reported HEAD:

```text
5bba1cf
```

Architecture Owner has reviewed the public branch and the read-only FAST-LIVO2 reference.

**Do NOT start V-4 yet.**

This is a corrective round, not a redesign round.

Goal:

```text
preserve validated work
→ remove/disable unapproved heuristics
→ restore intended visual frontend semantics
→ complete missing hard verification
→ ONLY THEN allow V-4 + ATE
```

Do not use this corrective as permission to redesign additional modules.

## 1. KEEP validated work

Do not wholesale revert Round 11.

Preserve:

```text
19c6e2d
G-1VR:
centroid = coordinate origin only
P_patch identity invariant
3° support reparameterization semantics

ad1278f
S-0:
camera-epoch split/pending/stale handling
double-pop crash correction
legacy path isolation
conservation tests

d01344b
S-1:
explicit role-typed sequential prior API
zero-information identity

09ae79e
V-0/V-1 STRUCTURAL CORE ONLY:
VisualLandmark
3 bounded observation slots
uint8[64] persistent patch
Sparse VisualMap side table
1 parent : N landmarks
3° geometry support sync

5bba1cf
V-2/V-3 MATHEMATICAL SCAFFOLD ONLY:
DC residual formulation
streaming 6x6 / 6x1 accumulation
state-off path
```

But mark:

```text
V-0/V-1/V-2/V-3 = CORRECTIVE REQUIRED
```

until this prompt's gates pass.

## 2. Verified problems before V-4

### P0-A — Unapproved frontend heuristics

Current branch/evidence introduced:

```text
scan stride N/300 -> N/60
per-parent cap 4 -> 8
30 s unseen eviction
```

These are not Architecture Owner decisions.

Disable/remove them as decision mechanisms:

```text
NO scan-index stride as visual feature selector
NO arbitrary parent cap 4/8 as feature-quality selector
NO 30-second unseen timer as geometry lifetime policy
```

Do not replace them with another invented number.

### P0-B — Fixed P_patch is NOT the visibility bug

Do NOT re-anchor or move a VisualLandmark to keep it visible.

Frozen invariant remains:

\[
P_{patch}=\mu_{sync}+\delta_{sync}
\]

and reparameterization preserves it.

A fixed 3D landmark naturally leaves the FOV. The frontend must keep retrieving existing visible landmarks and creating other currently visible landmarks.

Correct evidence wording:

```text
current implementation produced insufficient fresh/currently-visible landmarks;
feature selection/retrieval must be audited.
```

Do not call fixed physical identity inherently incompatible with visual tracking.

### P0-C — Active reference must be reselectable

Current evidence freezes active reference forever.

Frozen Owner semantics:

```text
individual sampled observation patch = immutable
landmark has <=3 real observations
active reference may be reselected at solve boundaries
```

Implement bounded reference reselection.

Never switch reference during nonlinear visual iterations.

### P0-D — FAST-LIVO2 observation trigger was only partially inherited

Use all three reference-inherited OR terms:

```text
delta translation > 0.5 m
OR
delta rotation > 0.3 rad
OR
pixel distance > 40 px
```

Record as P-B reference inherited, NO SWEEP.

Super-LIVO stays at Owner-approved 3 observations, not FAST-LIVO2's 30.

### P0-E — V-2 6DOF FD is not established

Current evidence only documents FD for translation x.

Before V-4 validate all:

```text
rx ry rz tx ty tz
```

on multiple landmarks and multiple frames on:

```text
eee_01
nya_01
```

Any failed column => STOP, NO V-4.

### P0-F — Parent eviction/generation coupling is incomplete

Required:

```text
OctVox parent eviction
→ corresponding VisualMap landmarks erase/deactivate

generation mismatch
→ old landmark cannot be reused

parent plane invalid
→ support unavailable
```

30s unseen eviction is not a substitute.

Same spatial key later reappears => new generation, old landmarks not silently reused.

## 3. FAST-LIVO2 reference facts to re-audit from source

Read:

```text
refs/FAST-LIVO2/src/vio.cpp
refs/FAST-LIVO2/src/LIVMapper.cpp
```

Document actual current source.

Required facts:

1. VisualPoint uses fixed 3D `pos_`; current observations project that same point.
2. `generateVisualMapPoints` is image-grid driven: project eligible 3D candidates, select highest Shi-Tomasi candidate per available image cell.
3. Existing point observation update trigger is:
   `translation >0.5m OR rotation >0.3rad OR pixel_dist >40px`.
4. FAST-LIVO2 observation list is capped at 30; correct old “unbounded-ish” wording.
5. Super-LIVO intentionally remains hard-capped at 3 observations.

## 4. Register this corrective

Canonical prompt:

```text
prompts/04_v1_implementation/17_round11_visual_frontend_corrective_before_mode_a.md
```

Do not overwrite existing #16 S-0 corrective.

Update `prompts/README.md`.

Create tracker corrective item:

```text
#21 V-0C
21-v0c-visual-frontend-corrective-before-mode-a.md
```

Graph:

```text
V-0/V-1/V-2/V-3 [corrective required]
→ V-0C
→ V-4
→ ATE
→ OWNER REVIEW
```

No V-5.

## 5. Phase A — architecture audit before source edits

Create:

```text
docs/super_livo/evidence/v0c_visual_frontend_architecture_audit.md
```

Required table:

| Topic | Frozen Super-LIVO | Current implementation | FAST-LIVO2 reference | Action |
|---|---|---|---|---|
| physical anchor | fixed | | fixed VisualPoint | |
| feature creation selector | image-space sparse selection | stride N/60 | image grid + Shi-Tomasi | |
| parent landmark cap | not Owner-frozen | 8 | not primary selector | |
| observation cap | 3 | 3 | 30 | keep 3 |
| observation add trigger | reference inherited | partial | 0.5m OR 0.3rad OR 40px | |
| reference reselection | bounded allowed | frozen active | reference may change | |
| geometry invalidation | parent lifecycle | incomplete + 30s timer | separate map semantics | |
| FD | all 6DOF required | tx only | — | repair |

After audit, continue automatically with only the authorized corrections below.

## 6. Phase B — image-grid visual point selector

Do NOT redesign Scheme-B geometry.

Input candidates remain:

```text
currently available LiDAR/map-supported 3D candidates
+ valid parent plane support
+ camera FOV
+ patch border valid
```

Patch anchor remains the true fixed 3D point represented by coordinate-origin Scheme-B.

Implement minimal FAST-LIVO2-inspired selection:

```text
project eligible candidate anchors into current camera
→ assign to image grid
→ existing usable landmark occupies grid
→ for unoccupied cells choose best new candidate by Shi-Tomasi
→ create new landmark only for selected candidates
```

No scan-order stride.

### Grid provenance

Inspect FAST-LIVO2 current defaults. Current official source uses:

```text
vio/grid_size default 5
vio/grid_n_height default 17
if grid_size <= 10:
    effective grid_size = image_height / grid_n_height
```

For first Super-LIVO implementation:

```text
inherit grid_n_height = 17
derive grid size from the actual image used by the photometric frontend
```

Record P-B reference inherited.

NO grid sweep.

## 7. Existing retrieval and new creation are distinct

Every camera epoch:

1. Project existing active VisualLandmarks.
2. If in-frame, generation-valid, plane-valid, warp-valid, they are current residual candidates.
3. Mark their image grid cells occupied.
4. Only unoccupied cells may receive newly selected candidates.

Do not require old landmarks to stay visible forever.

Do not interpret leaving FOV as failure.

## 8. Remove unapproved lifetime heuristics

Disable/remove:

```text
N/60
N/300
cap4
cap8
30s unseen eviction
```

If a safety bound appears necessary, DO NOT invent a replacement.

Instead:

- avoid duplicate landmark creation for already represented anchor/grid opportunities;
- couple lifetime to parent eviction/generation;
- measure growth on eee/nya.

If landmark count still grows without bound, STOP for Owner review instead of adding another number.

## 9. Correct parent lifecycle coupling

Use the actual OctVox/LRU eviction event already exposed for geometry sidecars where possible.

Tests:

```text
small map capacity
force parent eviction
→ visual landmarks removed/deactivated

same key re-created
→ generation differs
→ old landmarks not reused

plane valid -> invalid
→ visual support unavailable
```

No time-based replacement.

## 10. Observation cap stays exactly 3

Keep:

```text
uint8[64] immutable patch
3 observation slots:
active
best alternate
latest candidate
```

No cap sweep.

## 11. Observation add trigger uses all inherited terms

For an existing visible landmark, compare against latest stored observation:

```text
delta_translation > 0.5 m
OR delta_rotation > 0.3 rad
OR pixel_distance > 40 px
```

If fired and current projection is valid, sample a new immutable patch from the current image.

3° geometry sync NEVER samples a camera patch.

## 12. Bounded active-reference reselection

At a visual solve boundary only, choose best reference among <=3 valid observation slots.

Requirements:

```text
1 valid obs -> use it
2-3 valid obs -> deterministic score and choose best
tie -> keep current active
```

Ground scoring in FAST-LIVO2 behavior:

```text
photometric consistency among stored observations
+ current viewing geometry
```

Do not invent a tunable hysteresis threshold.

If exact FAST-LIVO2 scoring cannot be reused because Super-LIVO has only 3 slots, document the bounded adaptation.

Record `reference_switch_count`.

## 13. Measure frontend coverage per camera epoch

Primary metrics are NOT total landmarks created over the full bag.

Record per camera epoch:

```text
eligible projected geometry candidates
grid cells total
grid cells with valid candidates
grid cells occupied by reusable existing landmarks
grid cells filled by newly created landmarks

visible existing landmarks
new landmarks
invalidated landmarks

accepted photometric landmarks
accepted samples
```

Report P10/P50/P90 for:

```text
visible landmarks/frame
accepted photometric landmarks/frame
occupied image-grid ratio
new/reused landmarks/frame
```

No invented hard minimum count.

## 14. Re-run eee_01 first

Compare:

```text
C0 = camera epoch ON, visual state apply OFF
V0C = corrected frontend + V2/V3 equations ON, state apply OFF
```

Required:

```text
trajectory == C0
```

Prefer bitwise.

Compare old “1 frame with >=32 samples” against new full-bag distribution.

## 15. Re-run nya_01 second

Repeat all frontend metrics.

Use nya to exercise:

```text
rotation observation trigger
3° geometry sync if present
multi-frame FD
```

Do not alter thresholds because eee is static.

## 16. Full 6DOF FD hard gate

Mark V-2 `CORRECTIVE REQUIRED` until pass.

For each test bundle perturb:

```text
rx ry rz tx ty tz
```

Prefer central difference:

\[
J_j^{FD}=(r(x+\epsilon e_j)-r(x-\epsilon e_j))/(2\epsilon)
\]

Freeze during FD:

```text
valid overlap semantics
active reference
observation set
geometry snapshot
landmark creation
geometry sync
```

Required coverage:

```text
eee:
>=5 camera epochs
>=10 distinct landmarks total if available

nya:
>=5 camera epochs
>=10 distinct landmarks total if available
```

If frontend cannot supply this, STOP before V-4. Do not reduce the requirement by inventing more heuristics.

For all six directions report:

```text
sample count
max abs error
median relative error
max relative error
```

Existing threshold:

```text
max relative error < 1e-2
```

must pass for all six directions.

## 17. V-3 state-off parity rerun

After all frontend corrections and full FD:

```text
equations ON
state apply OFF
```

Run eee + nya.

Required:

```text
trajectory == C0
```

Any state effect => STOP.

## 18. R12/R20 local-shift diagnostic

Complete postponed diagnostic only.

After S-0 and corrected reference selection:

```text
R=12
```

If >10% hit search boundary, use cached diagnostic patches with:

```text
R=20
```

Do not feed shift into estimator.

Report whether old R=5 saturation was timing/reference related or persists.

## 19. S-0 and S-1: audit, do not redesign again

S-0:
rerun existing conservation/stale/pending/legacy tests and full eee/nya.

Do not rewrite synchronization unless a new concrete failing test exists.

S-1:
rerun zero-info identity and informative update.

No FEJ.

## 20. Pre-V4 hard gate

V-4 is allowed ONLY if all pass:

```text
A no N/60/N/300 selector
B no cap4/cap8 feature selector
C no 30s unseen eviction
D image-grid selector active
E P_patch invariant preserved
F parent eviction/generation coupling PASS
G all 3 observation-trigger terms used
H bounded active-reference reselection works
I 3-observation hard cap preserved
J all-6DOF FD PASS on eee + nya
K V-3 state-off parity PASS on eee + nya
L frontend provides enough multi-frame observations to satisfy FD coverage
```

If any gate is ambiguous/fails:

```text
STOP FOR OWNER REVIEW
DO NOT RUN V-4
DO NOT INVENT ANOTHER HEURISTIC
```

## 21. ONLY after all gates: V-4 MODE-A

Frozen order:

```text
IMU propagation
→ LiDAR update
→ LiDAR posterior
→ Visual MODE-A update
→ camera posterior
```

During one visual solve freeze:

```text
landmark set
active references
P_patch
mu_sync/delta_sync
n_sync
```

No lifecycle mutation inside iterations.

After solve, observation/lifecycle bookkeeping may run in documented order.

## 22. First ATE after corrected V-4

Run eee_01 and nya_01:

```text
B0 original LIO
C0 camera-epoch visual OFF
A0 corrected MODE-A visual ON
```

Follow evaluation_protocol.md with identical:
- GT
- time range
- matching
- alignment
- crop.

Report:
- APE RMSE
- mean
- median
- max
- matched samples
- duration
- divergence time if any.

Primary effect:

```text
A0 vs C0
```

Secondary:

```text
B0 vs C0
```

## 23. No architecture tuning from ATE

If A0 is worse:

inspect:
- frontend coverage
- 6DOF FD
- visual update norm
- conditioning
- photo residual
- reference switches.

Do NOT change:
- grid height
- observation cap
- patch size
- geometry resolution
- observation trigger defaults
- feature lifetime.

Only the previously approved one-parameter visual measurement weight/noise `0.5x/1x/2x` may be used if implementation is verified and the instability is specifically measurement-strength related.

Otherwise report and STOP.

## 24. Correct documentation

### v0_visual_map_memory.md

Correct FAST-LIVO2 wording:

```text
FAST-LIVO2 observation history is capped at 30 per VisualPoint.
Super-LIVO intentionally caps at 3.
```

Mark:
- cap4/cap8 as rejected local heuristics;
- active-ref-never-switch as incomplete historical implementation.

### v2_photometric_jacobian.md

Old:

```text
6DOF FD PASS
```

must be corrected to:

```text
historical partial FD: tx only
```

until full gate passes.

Old:

```text
fixed P_patch conflicts with sustained visibility
```

must become:

```text
historical frontend-density failure under stride/cap/time heuristics;
fixed physical identity remains invariant.
```

Mark N/60/cap8/30s:

```text
REJECTED LOCAL CORRECTIVE HEURISTICS
```

## 25. Explicit architecture boundaries

FORBIDDEN:

```text
re-anchor/move P_patch for visibility
change coordinate-origin semantics
change 3° support sync
change parent plane support scale
change q_flat/q_line
change 3-observation cap
change uint8[64]
change patch size
enable parent direct LiDAR plane
neighbor surfel support
MODE-B
FEJ/Common-FEJ
MODE-C
new freshness timer
new parent landmark cap
new scan stride
S-0 redesign without failing test
```

## 26. Commit discipline

Forward corrective commits only; do not rewrite all prior history.

Suggested:

```text
docs: register visual frontend corrective
fix: replace stride sampling with image-grid selection
fix: couple visual landmarks to parent lifecycle
fix: restore bounded observation/reference lifecycle
test: validate photometric Jacobian in all pose directions
docs: close visual frontend corrective
feat: enable MODE-A visual update
docs: record corrected first visual ATE
```

## 27. Final report

If corrective and V-4 pass:

```text
Round 11 corrective completed; MODE-A executed.

Initial HEAD:
5bba1cf

=== Audit ===
Preserved:
Reopened:
Rejected heuristics:

=== FAST-LIVO2 reference ===
grid selector:
effective grid:
obs trigger:
obs cap:
reference lifecycle:

=== Corrected frontend ===
commits:
selector:
grid provenance:

eee visible landmarks/frame P10/P50/P90:
eee accepted landmarks/frame P10/P50/P90:
eee occupied grid:
eee new/reused:

nya:
...

=== Parent lifecycle ===
eviction:
generation reuse:
plane invalidation:
30s removed:

=== Observation lifecycle ===
cap:
translation trigger:
rotation trigger:
pixel trigger:
obs adds:
reference switches:
drops:
slot occupancy:

=== 6DOF FD ===
eee:
rx:
ry:
rz:
tx:
ty:
tz:

nya:
rx:
ry:
rz:
tx:
ty:
tz:

gate:

=== V-3 ===
eee parity:
nya parity:

=== Local shift ===
R12:
boundary:
R20:
interpretation:

=== Pre-V4 Gates ===
A:
B:
C:
D:
E:
F:
G:
H:
I:
J:
K:
L:

=== V-4 ===
commit:
accepted observations:
iterations:
residual before/after:
update norm:
conditioning:

=== ATE ===
eee:
B0:
C0:
A0:
A0/C0:

nya:
B0:
C0:
A0:
A0/C0:

HEAD:
repo status:
refs:
frontier: V-5 OWNER REVIEW ONLY

Next:
STOP.
```

If any pre-V4 gate fails:

```text
Round 11 corrective BLOCKED BEFORE V-4.

Failed gate:
...

Do NOT run MODE-A.
Do NOT invent another heuristic.
Await Architecture Owner.
```
