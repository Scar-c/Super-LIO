# Prob-LIO Prompt 3 — P1 Frame-Semantics Corrective Gate → P2 Probabilistic Map Plumbing

## 0. Mission

Continue on `prob-lio`.

This round has a strict two-stage order:

1. **Reopen and correct P1 frame semantics** for current-point covariance.
2. **Only if the new frame-semantics gate is GREEN**, continue directly into **P2 Probabilistic Map Plumbing**.

Do not start P3 QR-plane covariance, P4 probabilistic weighting, or P5 probabilistic association.

Expected current branch: `prob-lio`.

Expected current frontier is the latest pushed HEAD after Prompt 2, expected to include:

```text
4040875  P0 evaluator/parity closure
764488b  P1 current lidar point covariance
e3a1646  docs/SPEC follow-up
```

Verify the actual full SHA and worktree before edits. If branch/HEAD/worktree materially differs, STOP FOR OWNER. Do not reset/rebase/merge/discard unrelated work.

---

# 1. State consensus and prompt registration

Before modification:

```bash
cd ~/super_livo/src/Super-LIO
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -8
```

Requirements:

- branch = `prob-lio`
- worktree clean
- local/remote frontier understood
- no history rewrite

Register this exact prompt under:

```text
prompts/prob_lio/prompt3_P1_frame_corrective_P2_map_plumbing.md
```

and update the prompt index.

---

# 2. Architecture freeze

Authoritative roadmap:

```text
P0  CLOSED / OWNER VERIFIED
P1  REOPENED — frame-semantics corrective
P2  NOT STARTED until P1 corrective GREEN
P3  NOT STARTED
P4  NOT STARTED
P5  NOT STARTED
```

Authoritative seam IDs remain:

| Seam | Meaning |
|---|---|
| S0 | Downsample |
| S1 | LiDAR point covariance |
| S2 | current-point association covariance |
| S3 | map point covariance |
| S4 | initial map covariance |
| S5 | map covariance storage |
| S6 | compact-map aggregation |
| S7 | HKNN |
| S8 | Super QR plane estimator |
| S9 | QR plane covariance |
| S10 | correspondence gate |
| S11 | P2P measurement covariance / weighting |
| S12 | current pose covariance in final measurement `R_i` |
| S13 | IESKF information update |

This round may close **S1 corrective** and then **S3–S7 only**.

Not authorized:

- QR covariance / QR sensitivity (S9);
- changing QR plane solve (S8);
- changing `compute_error()` (S10);
- changing fixed `1000` (S11);
- adding current pose covariance to final measurement `R_i` (S12);
- changing ESKF math (S13);
- probabilistic association (S2/S10).

---

# PART A — P1 FRAME-SEMANTICS CORRECTIVE

# 3. Re-audit the production frames independently

Do not trust variable names like `body` without tracing transforms.

## 3.1 FAST-LIVO2 reference

Audit local `~/super_livo/ref/FAST-LIVO2` at its current reference HEAD.

Verify the active chain around:

```text
downsampled point
→ calcBodyCov(...)
→ extR/extT
→ later covariance rotations
```

Establish, from active code:

- the exact frame of the point passed into `calcBodyCov()`;
- whether `calcBodyCov()` is called before or after LiDAR→IMU extrinsic;
- how `body_cov_list_` is later rotated;
- the units of `dept_err` and `beam_err`.

Expected semantic to verify, not blindly assume:

```text
calcBodyCov input = LiDAR-frame point
beam_err = degrees in config, converted by DEG2RAD
```

## 3.2 Super-LIO production path

Trace actual production code:

```text
raw LiDAR point
→ LiDAR→IMU extrinsic
→ undistortion to scan-end body/IMU frame
→ VoxelGridClosest
→ points_body_v3_
```

Confirm the exact frame of `points_body_v3_`.

Record file/function/line evidence in `spec/prob_lio/SPEC.md`.

---

# 4. Required P1 correction

The P1 production path must model sensor covariance in the LiDAR measurement frame, then rotate it into the frame owned by `body_cov_list_`.

For a current downsampled Super-LIO point:

\[
p_I = \texttt{points\_body\_v3_[i]}
\]

with LiDAR→IMU extrinsic:

\[
p_I = R_{LI}p_L + t_{LI}
\]

recover:

\[
\boxed{
p_L = R_{LI}^{T}(p_I-t_{LI})
}
\]

Then:

\[
\boxed{
\Sigma_L =
\operatorname{CalcLidarPointCov}(p_L,\sigma_r,\sigma_\theta)
}
\]

and if `body_cov_list_` is defined in scan-end IMU/body frame:

\[
\boxed{
\Sigma_I = R_{LI}\Sigma_LR_{LI}^{T}
}
\]

Store:

```text
body_cov_list_[i] ↔ points_body_v3_[i]
```

in the **same frame**.

Do not add current pose covariance here.

Do not change downsampling.

Do not preserve a mathematically wrong `calcBodyCov(points_body_v3_[i])` shortcut merely because it passed prior trajectory tests.

---

# 5. NEW HARD GATE — FRAME-ORIGIN EQUIVALENCE

This gate exists specifically to prevent the exact false-positive that escaped Prompt 2.

Name it:

```text
G-P1.F — Sensor-Frame Origin / Extrinsic Equivalence Gate
```

P1 may not be re-closed unless every subgate below passes.

## G-P1.F1 — Production-path semantic identity

**Invariant**

For every test point generated in LiDAR frame:

\[
p_I = R_{LI}p_L+t_{LI}
\]

the **actual production helper/path** used by Super-LIO must return:

\[
\Sigma_I^{prod}
=
R_{LI}
\operatorname{CalcLidarPointCov}(p_L)
R_{LI}^{T}
\]

within tight numerical tolerance.

This test must call the same helper/seam used by production code. A toy duplicate implementation is forbidden.

### Required cases

At least:

1. NTU-like:
   - \(R_{LI}=I\)
   - \(t_{LI}\neq0\)
2. synthetic rotated extrinsic:
   - \(R_{LI}\neq I\)
   - \(t_{LI}\neq0\)
3. multiple LiDAR points:
   - near
   - medium
   - far
   - oblique

### Required negative mutation

Construct or temporarily inject the old incorrect behavior:

\[
\Sigma_{\rm wrong}
=
\operatorname{CalcLidarPointCov}(p_I)
\]

and prove the gate fails for a nonzero translation case.

The negative mutation must produce a measurable failure. If the test still passes, the gate is invalid.

**Forbidden substitute**

- checking only \(R\Sigma R^\top\);
- checking only matrix symmetry/PSD;
- comparing two duplicated formulas that both use the same wrong frame;
- trajectory parity.

---

## G-P1.F2 — Translation sensitivity

**Invariant**

With \(R_{LI}=I\), two different nonzero extrinsic translations must **not** be allowed to silently collapse into the same sensor-frame model when the IMU-frame point is held fixed.

The test must demonstrate that:

```text
wrong: calcBodyCov(p_I)
```

ignores the LiDAR-origin shift, while:

```text
correct: p_L = p_I - t_LI → calcBodyCov(p_L)
```

changes the range/beam geometry appropriately.

Required negative mutation:
- force `t_LI = 0` in the production-frame conversion and prove failure.

This catches the exact NTU case where rotation is identity but translation is nonzero.

---

## G-P1.F3 — Rotation covariance consistency

**Invariant**

For a non-identity extrinsic rotation:

\[
\Sigma_I =
R_{LI}\Sigma_LR_{LI}^{T}
\]

must match an independent reference calculation.

Required negative mutations:

- use \(R_{LI}^{T}\Sigma_LR_{LI}\);
- skip the covariance rotation entirely.

Both must fail.

---

## G-P1.F4 — Production ownership / index identity

**Invariant**

After every scan:

```text
body_cov_list_.size() == points_body_v3_.size()
```

and covariance `i` belongs to point `i`, after the corrected frame conversion.

Required cases:

- empty scan;
- multiple successive scans with changing sizes;
- no stale tail;
- no reorder.

This reuses prior identity tests but must now exercise the corrected production helper.

---

## G-P1.F5 — Real NTU extrinsic seam

Use the actual loaded NTU extrinsic values from the production config, not only a hard-coded fixture.

At runtime or in a config-aware test, verify:

- resolved `R_LI`;
- resolved `t_LI`;
- `t_LI` is nonzero for the canonical NTU config;
- at least one real/representative point shows that `CalcLidarPointCov(p_I)` and the corrected LiDAR-frame path are not mathematically identical.

Persist a compact evidence artifact.

This is not a per-point dump.

---

# 6. P1 regression gates after correction

Re-run the existing P1 tests plus `G-P1.F`.

Requirements:

- FAST-LIVO2 formula parity still passes in LiDAR frame;
- symmetry/finite/PSD tests pass;
- frame-origin gate passes;
- negative mutations fail;
- point/cov identity passes;
- no estimator consumer introduced.

Then run full `eee_01` with P1 ON.

Because covariance remains unused by estimator:

```text
BYTE_PARITY(pre-P1 baseline, corrected P1) = PASS
rows = 3981
matched = 3329
ATE = 0.118875639 m within prior tolerance
```

If byte parity fails, investigate before proceeding to P2.

Update SPEC:

```text
P1 = CLOSED / OWNER-CORRECTIVE-GREEN
```

only after the new frame gate is genuinely GREEN.

Commit P1 corrective separately, e.g.:

```text
fix(prob-lio): correct lidar covariance frame semantics
```

Worktree must be clean after the commit.

---

# 7. HARD GATE B — permission to enter P2

P2 may begin in this same session only if:

- `G-P1.F1...F5` PASS;
- negative mutations all fail as intended;
- full `eee_01` trajectory remains byte-identical to frozen baseline;
- official ATE/matched/rows remain baseline-equivalent;
- production diff confirms no S3–S13 changes accidentally entered P1;
- P1 corrective commit exists and worktree is clean.

If any condition fails, STOP FOR OWNER.

---

# PART B — P2 PROBABILISTIC MAP PLUMBING

# 8. P2 scope

P2 closes:

```text
S3 map point covariance
S4 initial map covariance
S5 map covariance storage
S6 compact-map covariance aggregation
S7 HKNN returns point + covariance
```

P2 must **not affect estimator output yet**.

The QR plane still consumes only point coordinates.

The fixed `1000` remains.

`compute_error()` remains.

ESKF remains.

Therefore P2 must also preserve byte-identical trajectory.

---

# 9. Re-audit FAST-LIVO2 active map covariance semantics

Before coding P2, inspect local FAST-LIVO2 active production code and document:

1. current-point sensor covariance source;
2. transformation into world/map frame;
3. active insertion-pose rotation covariance contribution;
4. active insertion-pose translation covariance contribution;
5. whether cross rotation-position covariance is used;
6. where `pointWithVar.var` is stored;
7. map initialization versus later map update behavior.

For P2 parity, follow the **active local code**, not paper-only semantics.

Expected first-version semantic, verify before use:

\[
\boxed{
\Sigma_{map}
=
R_{WI}\Sigma_I R_{WI}^{T}
+
J_R P_{RR}J_R^{T}
+
P_{pp}
}
\]

with no full 6×6 cross term unless active FAST-LIVO2 code actually uses it.

Do not add a “better” model in P2.

---

# 10. P2 map insertion seam

For every point inserted after the LIO update:

\[
p_W = R_{WI}p_I+t_{WI}
\]

compute a map covariance consistent with the active FAST-LIVO2 insertion semantics.

Use the **posterior/final state appropriate to the existing `UpdateMap()` production path**.

Do not use a stale pre-update state accidentally.

Do not reuse the current-query final measurement covariance semantics: map insertion and current measurement are different roles.

Add the minimal reusable representation needed to carry:

```text
world point + map covariance
```

into OctVox.

---

# 11. P2 initial-map seam

`map_init()` must receive the same probability semantics as later insertion.

No initial map point may silently have “no covariance” while later map points do.

Required invariant:

```text
initial-map representative points
and
post-init representative points
share the same covariance ownership contract
```

If initialization uses a different state/covariance lifecycle, document and test the exact authoritative production state used.

---

# 12. P2 OctVox storage seam

Current Super-LIO compact map semantics remain:

- same voxel layout;
- same subvoxel selection;
- same representative point rule;
- same accepted point count behavior;
- same search topology.

Add only the covariance state required by future QR-plane propagation.

Prefer compact storage appropriate to a symmetric \(3\times3\) covariance rather than bloating every subvoxel with an unnecessary general matrix, unless repository alignment/performance considerations justify otherwise.

The stored covariance must describe the **same representative point** returned by HKNN.

---

# 13. P2 compact aggregation semantic

Super-LIO's subvoxel representative is updated by a running mean of accepted map points.

Define and document the covariance aggregation approximation used in P2.

For the first Prob-LIO version, use the smallest mathematically coherent model needed to produce a covariance for the stored representative, and keep it consistent with the actual representative update.

Do not silently pretend a raw input covariance belongs to the resulting mean point.

If using the independent-point approximation:

\[
\mu_N = \frac1N\sum_i p_i
\]

\[
\boxed{
\Sigma_{\mu_N}
=
\frac1{N^2}\sum_i \Sigma_i
}
\]

implement it recursively or with an equivalent accumulator.

Important caveat:

- historical insertion-pose errors can be correlated across points from the same scan;
- P2 first version may follow the declared independent approximation if that is the chosen seam;
- record this approximation explicitly in SPEC as a limitation;
- do not secretly add a more complex correlated-frame model in P2.

If source inspection suggests a better minimal seam is necessary, report before broadening scope.

---

# 14. P2 HKNN seam

Current `getTopK()` search semantics remain unchanged.

Extend the output/neighbor representation so future P3 can obtain:

```text
neighbor point_i
neighbor covariance_i
```

for each selected representative.

Hard invariant:

```text
the covariance returned for neighbor i belongs to exactly the representative point returned as neighbor i
```

Do not:

- change K;
- change heap ordering;
- change distance metric;
- change acceptance radius;
- change search traversal.

P2 is plumbing only.

---

# 15. P2 hard gates

## G-P2.1 — map insertion covariance formula

**Invariant**

Production map insertion covariance matches the independently computed FAST-LIVO2-parity formula for representative state/point fixtures.

Required cases:

- nonzero rotation covariance;
- nonzero position covariance;
- nonidentity world rotation;
- multiple point directions.

Negative mutations:

- omit sensor covariance;
- omit rotation-state covariance;
- omit translation covariance.

Each must fail.

---

## G-P2.2 — initialization parity

**Invariant**

`map_init()` and normal `UpdateMap()` produce map-point covariance under the same declared ownership/semantic contract.

Negative mutation:

- initialization inserts zero/default covariance;
- test must fail.

---

## G-P2.3 — representative covariance aggregation

**Invariant**

For a known sequence of accepted points/covariances in one subvoxel, stored representative point and covariance equal the declared aggregation model.

Test:

- N=1;
- N=2;
- N=20;
- rejection/no-update case.

Negative mutations:

- divide covariance accumulator by `N` instead of `N^2`;
- update point but not covariance;
- update covariance but not point.

All must fail.

---

## G-P2.4 — HKNN point/cov identity

**Invariant**

The exact representative selected by `getTopK()` returns its own covariance.

Required adversarial test:

- neighboring subvoxels have deliberately distinct covariance fingerprints;
- force KNN ordering;
- verify each returned point is paired with the correct fingerprint.

Negative mutations:

- shifted index;
- covariance from parent/neighbor slot;
- sorted points without applying identical permutation to covariance.

All must fail.

---

## G-P2.5 — no estimator consumption

**Invariant**

P2 covariance reaches map storage and HKNN output, but:

- QR plane uses only point coordinates;
- no plane covariance is computed;
- `compute_error()` unchanged;
- fixed `1000` unchanged;
- ESKF unchanged.

Evidence:

- production diff audit;
- source trace;
- full trajectory parity.

---

## G-P2.6 — initial + mature map runtime coverage

On full `eee_01`, collect bounded counters proving:

- initial map covariance insertions occurred;
- normal update-map covariance insertions occurred;
- HKNN returned covariance-bearing neighbors during runtime;
- zero invalid/nonfinite covariance under normal run, or classify any observed anomaly.

No per-point dump.

Counters must be optional/bounded and not heavy by default.

---

## G-P2.7 — trajectory / accuracy parity

Because P2 still does not affect estimator math:

```text
BYTE_PARITY(P1-corrected, P2) = PASS
```

and canonical evaluator remains:

```text
rows    = 3981
matched = 3329
ATE     = frozen baseline within 1e-6 m
```

If trajectory bytes change, investigate before accepting P2.

---

# 16. Runtime/evidence workflow

For both P1 corrective and P2:

- build with bounded commands;
- run unit/seam tests first;
- then one full offline `eee_01`;
- preserve real RC and explicit sentinel;
- use the existing official NTU evaluator;
- preserve trajectory MD5/SHA256;
- compare byte parity first, numeric parity second.

Use the Owner-authorized aggressive cleanup policy before canonical runs.

Do not rerun just because the UI spinner remains; inspect actual process state first.

Heavy diagnostics remain OFF unless a concrete anomaly requires them.

---

# 17. Reusable artifact policy

Continue maintaining:

```text
spec/prob_lio/
prompts/prob_lio/
tools/prob_lio/
eval/prob_lio/
tests/prob_lio/
results/prob_lio/
```

Important rules:

- production tests/tools must not live only in `/tmp`;
- no dataset-specific absolute paths in production code;
- config parameters must be reusable;
- no giant raw debug dumps committed;
- small evidence summaries/hashes/counters are preferred;
- update the same authoritative `SPEC.md`, do not create a competing project-state file.

---

# 18. Commit structure

Use logical commits.

### Commit C — P1 frame corrective

Example:

```text
fix(prob-lio): correct lidar covariance frame semantics
```

Contains:

- production frame fix;
- `G-P1.F` tests;
- evidence;
- SPEC correction;
- no P2 code.

### Commit D — P2 map plumbing

Only after HARD GATE B.

Example:

```text
feat(prob-lio): add probabilistic map covariance plumbing
```

Contains:

- S3–S7 production plumbing;
- P2 tests;
- P2 evidence;
- SPEC update.

A small final docs-only SHA bookkeeping commit is acceptable.

Push normally. No force push.

---

# 19. Final report format

## Agent State Consensus
- starting HEAD
- branch/upstream
- clean state

## P1 Corrective Source Audit
- FAST-LIVO2 calcBodyCov input frame
- Super-LIO points_body_v3_ frame
- loaded NTU extrinsic
- exact corrected frame equation

## G-P1.F Evidence
For F1...F5:
- test path/name
- authoritative production seam exercised
- positive cases
- negative mutation
- PASS evidence

## P1 Corrective Runtime
- full eee_01 command
- RC/sentinel
- byte parity
- numeric parity
- rows/matched/ATE
- runtime
- corrective commit SHA

## HARD GATE B
- explicit PASS/FAIL
- P2 started only if PASS

## P2 Source Audit
- FAST-LIVO2 map insertion covariance active formula
- state covariance blocks actually used
- Super map_init / UpdateMap / OctVox / getTopK ownership paths

## P2 Implementation
- production files changed
- map covariance representation
- initialization path
- update path
- aggregation model
- HKNN point/cov pairing
- what remains deliberately untouched

## P2 Tests
For G-P2.1...G-P2.6:
- test path
- semantic invariant
- negative mutation
- evidence

## P2 Full eee_01
- run command
- RC/sentinel
- counters
- trajectory MD5/SHA256
- byte parity
- rows/matched/ATE
- runtime

## Diff Audit
Confirm:
- no P3 QR covariance;
- no P4 weight change;
- no gate change;
- no ESKF change.

## SPEC
- P0 status
- P1 status
- P2 status
- P3 remains NOT STARTED

## Commits
- P1 corrective SHA
- P2 SHA
- final HEAD
- clean worktree
- push status

## Gate Summary
- G-P1.F1...F5
- HARD GATE B
- G-P2.1...G-P2.7
- PASS/FAIL with evidence paths

---

# 20. Review contract

Your report is not acceptance authority.

The Owner/reviewer will independently inspect:

- actual production diff;
- frame transforms;
- test implementation;
- negative mutations;
- map covariance math;
- OctVox representative ownership;
- HKNN point/cov pairing;
- runtime evidence;
- trajectory hashes;
- evaluator results;
- SPEC.

A gate is GREEN only if the code and evidence prove it, not because the final report says PASS.
