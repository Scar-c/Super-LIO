# Prob-LIO Prompt 10 — Documentation Consolidation / Canonicalization Before Generalization

## 0. Mission

This round is **documentation / project-state consolidation only**.

Do not modify algorithm behavior.

The goal is to leave one clean, internally consistent, auditable project baseline before multi-bag/generalization validation. After Prompt 10 the repository must make it obvious:

1. what the canonical Prob-LIO algorithm is;
2. which stages P0–P4 are Owner-verified;
3. what P5 is and why it is non-canonical;
4. which configuration future generalization runs must use;
5. what the clean canonical `eee_01` results are;
6. which FAST-LIVO2 semantics are parity-compatible, bug-compatible, corrected, or experimental;
7. which known limitations remain;
8. where future generalized evidence belongs;
9. which historical P5 diagnostic claims were invalidated and must not be cited as current truth;
10. what exact clean repository frontier is handed to generalization.

This is a docs/spec/evidence-index cleanup round, **not another P5 round**.

---

# 1. Hard scope freeze

Allowed:
- `spec/`
- `prompts/`
- documentation/index files under `docs/`, `eval/`, `tools/`, `results/`
- config comments only if loaded values do not change.

Forbidden:
- production algorithm changes;
- P5 lifecycle changes;
- P5 test redesign;
- any noise/association tuning;
- S6/QR/HKNN/IESKF changes;
- new dataset runs;
- `eee_01` reruns;
- sweeps.

If documentation exposes a real production bug, record it as deferred and report it. Do not fix it here.

---

# 2. Repository state consensus

Expected repo:

```text
~/super_livo/src/Super-LIO
```

Expected branch:

```text
prob-lio
```

Expected clean frontier is approximately:

```text
8b5a1dc
```

Verify:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -20
```

Require:
- `prob-lio`;
- clean worktree;
- `HEAD == origin/prob-lio`.

No reset/rebase/history rewrite/force push.

Register this exact prompt as:

```text
prompts/prob_lio/prompt10_documentation_consolidation.md
```

and update prompt index.

---

# 3. Authoritative final stage status

Normalize current authority to:

```text
P0 = CLOSED / OWNER VERIFIED
P1 = CLOSED / OWNER VERIFIED
P2 = CLOSED / OWNER VERIFIED
P3 = CLOSED / OWNER VERIFIED
P4 = CLOSED / OWNER VERIFIED
P5 = EXPERIMENTAL / NON-CANONICAL
Generalization = READY / NOT STARTED
```

P5 must not be described in current-authority docs as:
- `OWNER VERIFIED`;
- `CLOSED/PASS`;
- canonical;
- recommended default.

Required P5 wording, semantically:

> P5 probability-association mathematics and the S2/S12 separation were audited, and the clean applied P5 regression is reproducible. However, its final lifecycle/root-cause diagnosis remained incomplete. On `eee_01`, applied P5 is substantially worse than canonical P4, so P5 is retained only as an experimental ablation and excluded from the canonical Prob-LIO path.

Do not claim a single lifecycle mechanism has been conclusively proven as the sole cause of the regression.

---

# 4. Canonical Prob-LIO architecture

Create/normalize one concise authoritative architecture section:

```text
Super-LIO native frontend
  ├─ native downsampling
  ├─ native compact OctVox map
  ├─ native HKNN
  ├─ native QR plane estimator
  ├─ P1 LiDAR point covariance
  ├─ P2 probabilistic compact-map covariance
  ├─ P3 QR-plane uncertainty propagation
  └─ P4 probabilistic P2P soft weighting

Canonical association:
  Super legacy association gate

P5 probabilistic association:
  experimental only
```

Canonical final measurement:

\[
R_i = 0.001 + \sigma^2_{QR-plane,i} + \sigma^2_{sensor-point,i}
\]

\[
w_i = 1/R_i
\]

Explicitly state:

```text
current pose covariance P is NOT in final P4 R_i
```

Current pose covariance may appear in map covariance and experimental P5 association covariance.

---

# 5. Canonical P0–P4 seam ledger

Create one authoritative seam table in `spec/prob_lio/SPEC.md`.

| Seam | Canonical state |
|---|---|
| S0 | Super native downsample unchanged |
| S1 | LiDAR sensor covariance with correct LiDAR→IMU frame chain |
| S2 | canonical association = Super legacy; P5 association experimental |
| S3 | map-point covariance |
| S4 | initial-map covariance |
| S5 | covariance storage, canonical precision double |
| S6 | compact representative covariance approximation |
| S7 | Super HKNN unchanged |
| S8 | Super QR plane estimator unchanged |
| S9 | QR plane covariance active for P4 |
| S10 | canonical correspondence gate = Super legacy |
| S11 | P4 probabilistic P2P soft weight |
| S12 | current pose P excluded from final R_i |
| S13 | Super IESKF update unchanged |

Each row should identify:
- owner stage;
- production file/function;
- canonical vs experimental;
- known caveat.

Remove duplicate/conflicting current seam tables.

---

# 6. Canonical `eee_01` result ledger

Create one concise authoritative table based only on clean committed evidence:

| Configuration | ATE | Status |
|---|---:|---|
| fixed `1000` baseline | 0.118875639 m | frozen baseline |
| P4 `prob_livo2 + livo2_compat` | 0.088831554 m | canonical Prob-LIO |
| P4 `prob_livo2 + super_right_consistent` | 0.089745655 m | clean A/B observation |
| P5 applied probabilistic association | 1.190814611 m | experimental regression |

Where applicable preserve:
- rows = 3981;
- matched = 3329;
- trajectory hash;
- exact clean run directory;
- algorithm/source identity.

Never promote dirty-worktree provisional runs into the canonical ledger.

---

# 7. HARD cleanup: invalid P5 lifecycle claims

Search current-authority docs/index files for stale claims equivalent to:

```text
3980/3981 frames execute one IEKF iteration
need_converge rarely reached
convergence phase performs no probability reevaluation
no sticky mask semantics
sticky_skip=0 proves no sticky lifecycle
counterfactual_reaccept=0 proves no reentry issue
P5 architecture/model mismatch proven as root cause
P5 lifecycle mismatch proven as the cause of 1.19 m
```

They must not remain as current truth.

Replace with this bounded statement:

> Corrected iteration accounting showed that most `eee_01` frames execute multiple IEKF Observe callbacks; the earlier one-iteration conclusion came from corrupted diagnostic frame identity. Prompt-9-REDO fixed frame identity and cleanly reproduced the P5 accuracy regression, but its final lifecycle diagnostic still did not fully cover the applied convergence-phase P5 control flow. Therefore the exact root cause of the P5 regression remains unresolved. P5 is non-canonical because its clean applied result is substantially worse than P4, not because one root-cause hypothesis was conclusively proven.

Historical prompt/report text may remain only if clearly marked `SUPERSEDED/INVALIDATED` or moved into HISTORY.

---

# 8. Correct iteration facts

Where iteration facts are summarized, use the accepted corrected histogram:

```text
final obs_iter=2: 395 frames
final obs_iter=3: 195 frames
final obs_iter=4: 3391 frames
```

Document 1-based mapping:

```text
obs_iter=1 ↔ ESKF iter=0
obs_iter=2 ↔ ESKF iter=1
obs_iter=3 ↔ ESKF iter=2
obs_iter=4 ↔ ESKF iter=3
```

and:

```text
need_converge=true when ESKF iter > 2
```

Therefore the convergence callback corresponds to `obs_iter=4`, i.e. 3391 frames (~85.2%) in this run.

Do not retain `3586 frames reach need_converge` as current truth unless independently disproven/redefined with exact source semantics.

---

# 9. P5 final evidence interpretation

Create one authoritative P5 subsection.

### Verified
- probability-association formula;
- S2/S12 separation;
- single candidate/candidate-builder math;
- association pose-model separation;
- clean applied regression;
- P4 shadow non-interference.

### Not fully verified
- complete convergence-phase shadow parity with applied P5;
- final sticky/reentry attribution;
- sole/root cause of the 1.190814611 m regression.

### Empirical conclusion

```text
P4 canonical = 0.088831554 m
P5 applied   = 1.190814611 m
```

Therefore:

```text
P5 is rejected as canonical for the current Prob-LIO version.
```

### Future status

```text
P5 remains selectable as an experimental ablation only.
Any future P5 lifecycle redesign requires explicit Owner authorization and is outside the current canonical roadmap.
```

---

# 10. S6 final documentation

Use conservative wording:

```text
S6_PRIMARY_CAUSE_NOT_SUPPORTED on current eee_01 evidence
```

Evidence includes:
- LA_PR rate decreases with representative maturity/count;
- unshrink sensitivity rescued only a minority;
- no S6 estimator modification was made.

Preserve the modeling limitation:

\[
\Sigma_{\mu_N} = rac{1}{N^2}\sum_i \Sigma_i
\]

is an approximation treating contributions as independent and does not explicitly model same-scan shared pose-error correlation.

Required wording:

> S6 is not identified as the primary cause of the current P5 regression, but compact-map covariance aggregation remains an acknowledged modeling approximation.

---

# 11. FAST-LIVO2 compatibility ledger

Create a compact semantic table using these labels:

```text
PARITY
BUG-COMPATIBLE
CORRECTED
EXPERIMENTAL
```

At minimum cover:

### P1 sensor covariance
Prob-LIO uses the correct LiDAR→IMU covariance frame chain. If active FAST-LIVO2 omits nonidentity extrinsic rotation in an association path, Prob-LIO is **extrinsic-consistent**, not exact bug-compatible parity. For NTU, `R_LI=I`, so this distinction does not affect the `eee_01` result.

### P2 map pose covariance
- `livo2_compat`: active-reference-style / bug-compatible mode.
- `super_right_consistent`: corrected Super right-perturbation mode.

Canonical first generalization mode:

```text
livo2_compat
```

unless Owner later changes it.

### P4 final weight
Active FAST-LIVO2 final-weight parity:
- floor `0.001`;
- plane variance;
- sensor-point variance;
- no current pose P in final weight.

### P5 association
Experimental adaptation. Do not call its whole lifecycle exact FAST-LIVO2 parity.

---

# 12. Canonical configuration for generalization

Create one authoritative future-run profile:

```text
cov_enable = ON
cov_validation_mode = light
map_pose_cov_model = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable = ON
p2p_weight_mode = prob_livo2
association_mode = super_legacy
prob_assoc_shadow_enable = OFF
```

Heavy/debug diagnostics OFF by default:
- full eigensolver covariance validation OFF;
- P5 shadow OFF;
- per-point dumps OFF;
- FD/profiling/heavy intrinsic instrumentation OFF.

Do not change actual values in Prompt 10. Document the existing canonical values.

---

# 13. Config provenance for future datasets

Document priority:

```text
1. dataset/algorithm official config if available;
2. frozen Super-LIO dataset-specific config;
3. FAST-LIVO2 official config where relevant;
4. default only if no dataset-specific authoritative value exists.
```

No sweeps by default.

Preserve dataset-specific authoritative:
- blind/max range;
- beam/depth noise;
- IESKF iterations;
- plane/HKNN settings;
- extrinsics;
- IMU noise.

Prob-LIO mode switches remain fixed unless an explicit semantic adapter is required.

---

# 14. Generalization preparation — no runs yet

Create a reusable future result-ledger template with fields such as:

```text
dataset
sequence
bag/hash
config source
algorithm HEAD
production_code_tree_oid
git_dirty
association mode
map pose covariance mode
evaluator
ATE
completion
runtime
classification
notes
```

Store under `spec/prob_lio/` or `results/prob_lio/README.md`.

Do not fabricate results.

Planning order should document the currently active Owner plan:

```text
MCD second target / NTU night_08
→ NTU VIRAL
→ Oxford
→ M3DGR
```

If repository evidence contains a newer explicit Owner order, use that newer one and identify provenance.

Do not execute any dataset in Prompt 10.

---

# 15. Evaluator ledger

Preserve known evaluator authority.

For `eee_01`, document the canonical official-compatible evaluator contract:
- prism lever arm;
- strict timestamp interpolation;
- SE(3) Umeyama alignment, no scale;
- 3981 estimated rows / 3329 matched in canonical run.

For future dataset families, document only choices already audited. Do not invent evaluator semantics.

---

# 16. Canonical run/evidence hygiene

Document the permanent rule:

```text
modify
→ test
→ commit
→ clean
→ canonical run
→ evaluate
```

Canonical metadata must include:

```text
algorithm_commit
run_git_head
run_git_dirty=false
run_git_status_short=""
production_code_tree_oid
bag hash
effective config snapshot
```

Generated runtime artifacts belong under:

```text
results/prob_lio/run_xxx/
```

never under `src/super_lio/`.

---

# 17. Authoritative document hierarchy

Normalize to a simple hierarchy.

### Level 1 — current truth

```text
spec/prob_lio/SPEC.md
```

Single current architecture/status authority.

### Level 2 — evidence index

Create/normalize:

```text
spec/prob_lio/EVIDENCE_INDEX.md
```

Include:
- canonical runs;
- hashes;
- evaluator outputs;
- stage closure evidence;
- clean commit/source identities.

### Level 3 — history

Create/normalize:

```text
spec/prob_lio/HISTORY.md
```

Include:
- P0→P5 chronology;
- corrective rounds;
- superseded diagnostic conclusions;
- rejected hypotheses.

Historical mistakes are allowed here if explicitly labeled superseded. Do not duplicate the whole SPEC.

---

# 18. HARD GATE G-P10.1 — no contradictory project state

Search repository current-authority docs for status strings.

There must not be simultaneous current claims such as:

```text
P5 CLOSED/PASS
P5 NOT STARTED
P5 OWNER VERIFIED
P5 canonical
```

while authoritative state is experimental/non-canonical.

Likewise P0–P4 must not be pending in current authority.

Provide search/grep evidence.

---

# 19. HARD GATE G-P10.2 — no stale lifecycle conclusion

Search current-authority docs for stale claims from §7.

Required:

```text
0 unqualified stale lifecycle/root-cause claims in current SPEC/EVIDENCE_INDEX
```

Historical occurrences must be marked superseded/invalidated.

---

# 20. HARD GATE G-P10.3 — clean P4 reproducibility identity

Without rerunning any bag, verify the evidence index points to one clean canonical P4 run containing:

```text
ATE 0.088831554...
trajectory hash
3981 rows
3329 matched
clean git metadata
```

Record exact run directory and source identity.

Do not use dirty provisional evidence.

---

# 21. HARD GATE G-P10.4 — clean P5 non-canonical evidence

Verify the index points to one clean applied P5 result:

```text
ATE 1.190814611...
clean committed source
```

and labels it only experimental/non-canonical.

The rejection reason is empirical performance, not a falsely proven root cause.

---

# 22. HARD GATE G-P10.5 — canonical config explicit

One authoritative section/file must allow a future Agent to start generalization without reconstructing P0–P9 history.

Required switches:

```text
cov ON
validation light
map pose livo2_compat
storage double
QR plane covariance ON
P4 probabilistic weight
Super legacy association
P5 shadow OFF
heavy diagnostics OFF
```

No conflicting current recommendation elsewhere.

---

# 23. HARD GATE G-P10.6 — no behavior change

Compare starting HEAD to final docs commit.

Expected:
- no production C++ behavior change;
- no parameter value change;
- no estimator behavior change.

If any behavior-changing diff appears, revert it and mark the gate failed until clean.

---

# 24. HARD GATE G-P10.7 — evidence links/paths valid

Every canonical evidence path referenced by SPEC/EVIDENCE_INDEX must exist.

Check at least:
- frozen baseline;
- clean P3/P4 closure;
- canonical P4;
- clean P5 applied;
- evaluator output;
- prompt files.

No broken/stale canonical links.

---

# 25. Prompt/history index

Normalize `prompts/prob_lio/README.md` or equivalent.

For P0–P10 list:
- prompt number;
- purpose;
- starting/ending stage;
- status;
- corrective/superseded relationship.

Prompt 8 and earlier Prompt-9 lifecycle conclusions must be marked superseded where appropriate.

Do not delete historical prompts.

---

# 26. Recommended final docs

At minimum:

```text
spec/prob_lio/SPEC.md
spec/prob_lio/EVIDENCE_INDEX.md
spec/prob_lio/HISTORY.md
prompts/prob_lio/README.md
```

Reuse equivalent existing files rather than creating duplicate authorities.

---

# 27. Commit policy

One focused docs commit or a tiny docs-only sequence.

Suggested:

```text
docs(prob-lio): canonicalize p0-p5 status before generalization
```

After commit:

```bash
git status --short
```

must be empty.

Push fast-forward to `origin/prob-lio`.

No force push.

---

# 28. Final report format

## Agent State Consensus
- starting HEAD
- branch/upstream
- clean state
- prompt path

## Documentation Audit
- contradictory statuses found
- invalid P5 lifecycle claims found
- broken evidence references found
- duplicate/obsolete authority docs found

## Canonical Architecture
- one-paragraph summary
- P0–P4 state
- P5 state

## Canonical `eee_01` Ledger
- fixed baseline
- P4 canonical
- P4 right-consistent A/B
- P5 experimental
- evidence paths/hashes

## P5 Documentation Correction
Explicitly state:
- what old claims were invalidated;
- what remains verified;
- root cause unresolved;
- why P5 is non-canonical.

## FAST-LIVO2 Compatibility Ledger
- parity
- bug-compatible
- corrected
- experimental

## Generalization Canonical Config
List exact switches.

## Generalization Planning
- ledger/template
- dataset order
- evaluator provenance
- confirm no run executed

## Gates
Report PASS/FAIL with evidence:

```text
G-P10.1 project-state consistency
G-P10.2 lifecycle-claim cleanup
G-P10.3 clean P4 evidence identity
G-P10.4 clean P5 evidence identity
G-P10.5 canonical config explicit
G-P10.6 no behavior change
G-P10.7 valid evidence links
```

## Files
List modified/created docs.

## Commit
- docs commit SHA
- final HEAD
- clean worktree
- push status

## Final Project State

End exactly with the semantic state:

```text
P0–P4 = CLOSED / OWNER VERIFIED
P5 = EXPERIMENTAL / NON-CANONICAL
Canonical Prob-LIO = P0–P4 + Super legacy association
Generalization = READY / NOT STARTED
```

Do not start generalization.

---

# 29. Review contract

The report is not acceptance authority.

Owner will independently inspect:
- SPEC;
- evidence index;
- history;
- P5 wording;
- canonical config;
- absence of production changes;
- evidence paths.

Prompt 10 succeeds only if a future Agent can enter generalization from the repository docs without reconstructing the P0–P9 conversation history.
