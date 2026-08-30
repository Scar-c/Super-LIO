# Prob-LIO Prompt 1 — P0 Baseline Freeze / Project Bootstrap

## 0. Role and goal

You are implementing **P0 only** for the new `prob-lio` branch of Super-LIO.

The project goal is to add FAST-LIVO2-style probabilistic LiDAR / plane uncertainty semantics to Super-LIO **without changing Super-LIO's native QR plane estimator**. The planned stages are:

- **P0** Baseline Freeze
- **P1** Current Point Probability
- **P2** Probabilistic Map Plumbing
- **P3** QR Plane Uncertainty
- **P4** Probabilistic P2P Weighting
- **P5** Probabilistic Association (optional / second stage)

This round must **only close P0**. Do not start P1.

Expected repository state at start:

- workspace: `~/super_livo`
- production repo: `~/super_livo/src/Super-LIO`
- FAST-LIVO2 reference: `~/super_livo/ref/FAST-LIVO2`
- validation dataset root: `~/super_livo/bag/NTU/eee_01`
- branch: `prob-lio`
- expected starting HEAD: `60b57aac8dc397f80c56364e7ccb008c300cc29`
- `ros1`, `prob-lio`, and `origin/prob-lio` were created from the same HEAD.

Paths above are expected locations, not immutable contracts. Inspect the actual workspace first. If a filename, launch path, package name, or bag filename differs, adapt to the discovered repository/dataset structure and report the resolved path. A normal path-resolution adjustment is **not** a P0 failure. Stop only for a real semantic/architecture conflict or evidence that cannot be established.

---

## 1. State consensus — mandatory before work

Before modifying files:

1. Inspect branch, HEAD, upstream, remotes, and worktree.
2. Confirm current branch is `prob-lio`.
3. Confirm current HEAD is exactly:

   `60b57aac8dc397f80c56364e7ccb008c300cc29`

4. Confirm the worktree is clean.
5. Record:
   - `git status --short`
   - `git branch -vv`
   - `git rev-parse HEAD`
   - `git rev-parse ros1`
   - `git rev-parse origin/prob-lio`

If the current branch/HEAD/worktree differs materially from the expected state, **STOP FOR OWNER** instead of silently rebasing, merging, resetting, or discarding work.

No merge, rebase, reset, force-push, or unrelated cleanup is authorized.

---

## 2. P0 architecture freeze

P0 exists to establish a trustworthy baseline before covariance code is introduced.

The following Super-LIO production semantics must remain unchanged in P0:

| Seam | P0 frozen semantic |
|---|---|
| S0 Downsample | Existing PCL VoxelGrid path remains unchanged |
| S7 HKNN | Existing Super-LIO HKNN / `getTopK()` behavior remains unchanged |
| S8 Plane estimator | Existing `ColPivHouseholderQR` plane solve remains unchanged |
| S10 Correspondence gate | Existing `compute_error()` logic remains unchanged |
| S11 P2P weight | Existing fixed `1000` weighting remains unchanged |
| S13 IESKF | Existing information-form update remains unchanged |

P0 must not introduce covariance arrays, probabilistic map storage, QR covariance propagation, PROB weighting, or probabilistic gating.

---

## 3. Independent source audit — do not trust this prompt blindly

Audit the **actual production code** in `src/Super-LIO` and the **actual reference code** in `ref/FAST-LIVO2`.

### 3.1 Super-LIO baseline invariants to verify

Locate the authoritative production path and record file + function + line evidence for:

1. `DownSample()` uses the current PCL VoxelGrid behavior.
2. `OctVoxMap/getTopK()` returns the current small set of representative map points (expected max 5; verify actual code).
3. Plane fitting uses `ColPivHouseholderQR` and normalization into Super-LIO's current plane representation.
4. `compute_error()` uses the current Super-LIO geometric correspondence gate.
5. `Observe()` uses a fixed `1000` factor in the P2P information accumulation.
6. `ESKF::UpdateObserve()` combines prior information and measurement information in the current information-form update.

If any item differs from the expected description, do **not** force the code to match the prompt. Record the actual implementation and stop for Owner only if the difference changes the planned P1–P4 architecture.

### 3.2 FAST-LIVO2 active-semantics ledger for later stages

Independently verify the current local FAST-LIVO2 reference and record the active production semantics needed by future stages:

- point covariance is constructed with `calcBodyCov()` from range/beam uncertainty after the relevant downsampled-point stage;
- current-query association covariance includes the active state/pose contribution where applicable;
- map insertion stores point covariance including the active insertion-pose contribution;
- final P2P measurement weighting uses the active form equivalent to  
  `0.001 + plane variance + sensor point variance`;
- current pose covariance is **not** added again to the final measurement covariance if the corresponding code is inactive/commented;
- FAST-LIVO2 uses its own PCA/eigen plane representation/covariance, which is reference semantics only and will **not** replace Super-LIO QR;
- note the current plane-covariance lifecycle actually implemented in code, not only what the paper says.

This is a **reference ledger only** in P0. Do not port any of it yet.

---

## 4. Project structure and persistent SPEC

Establish a maintainable project structure inside the Super-LIO repository.

Preferred logical layout:

```text
spec/prob_lio/
tools/prob_lio/
tests/prob_lio/
eval/prob_lio/
results/prob_lio/
prompts/prob_lio/
```

Do not create useless framework code merely to populate folders. Every committed artifact should have a concrete purpose. Small README/index files are acceptable when they define ownership, inputs/outputs, and future conventions.

### 4.1 Authoritative project SPEC

Create:

```text
spec/prob_lio/SPEC.md
```

This file is the authoritative project-state document. It must be readable by itself and contain at least:

1. project objective;
2. immutable architectural decisions:
   - FAST-LIVO2 probabilistic semantics are the reference;
   - Super-LIO compact map/HKNN remains;
   - Super-LIO QR plane estimator remains;
   - QR uncertainty must later propagate through the actual QR estimator;
3. stage roadmap P0–P5 with current status;
4. seam table S0–S13;
5. baseline SHA and branch;
6. P0 source-audit findings for both repositories;
7. P0 validation dataset (`NTU eee_01`);
8. round/history section containing:
   - round name;
   - starting HEAD;
   - ending HEAD/commit;
   - what changed;
   - evidence/result paths;
   - gates passed/failed;
   - next authorized stage.

At the end of this round, only P0 may become `CLOSED/PASS`. P1 must remain `NOT STARTED`.

Future rounds must update this same SPEC instead of creating disconnected status documents.

### 4.2 Prompt registration

Register the exact Owner prompt used for this round under:

```text
prompts/prob_lio/
```

Use a clear filename such as:

```text
prompt1_P0_baseline_freeze.md
```

Maintain a small prompt index (`prompts/README.md` or an equivalent project-local index) so later rounds can trace which Owner instruction produced each commit.

---

## 5. Reusable tooling policy

P0 may add reusable baseline/run/evaluation helpers when they are actually needed to reproduce the baseline.

Rules:

- important reusable tools must live in the repository, not only in `/tmp`;
- temporary one-off debugging files may live in `/tmp`, but they are not accepted as persistent evidence;
- do not hard-code absolute machine paths when a CLI argument, environment variable, config file, or discovered workspace path is appropriate;
- dataset path, output path, launch/config selection, and runtime options should be parameterizable;
- default values may be provided for this workspace, but callers must be able to override them;
- do not duplicate an existing production-quality runner/evaluator if one can be reused or minimally wrapped;
- do not create brittle grep-only “tests” merely to satisfy the `tests/` folder. Add tests only when they assert a real semantic contract robustly;
- large bags, generated point clouds, or large raw logs must not be committed.

If a reusable runner is created, it should write its results to a caller-selectable directory under the `results/prob_lio/` convention and include enough metadata to reproduce the invocation.

---

## 6. P0 validation on NTU `eee_01`

Use `~/super_livo/bag/NTU/eee_01` as the temporary validation dataset for this project.

First inspect the directory and resolve the actual bag/config/launch requirements. Do not assume an exact bag filename from this prompt.

### Required baseline validation

At minimum:

1. build the ROS1 Super-LIO package/workspace using the repository's correct build procedure;
2. run one bounded baseline experiment on NTU `eee_01` using the existing Super-LIO production path and the correct available config/launch;
3. capture:
   - exact git HEAD;
   - resolved bag path;
   - launch/config command;
   - real process return code;
   - completion sentinel;
   - relevant trajectory/output path;
   - any evaluation metric that is already supported by the repository/workspace;
   - effective runtime parameters after node startup when practical;
   - absence/presence of fatal error / NaN / premature termination.

Do not invent an ATE acceptance threshold in P0 if the baseline has not yet been independently established for this exact branch/config. P0's purpose is to **freeze and record** the baseline, not tune it.

If an official NTU evaluator or an already-established evaluator exists in the workspace, use it. Do not replace an existing authoritative evaluator with an ad-hoc metric.

If a runtime issue is only a path/launch/config resolution issue, fix the invocation or reusable runner and continue. If the production algorithm itself cannot complete the baseline run, classify and report the failure instead of hiding it.

---

## 7. P0 hard gates

P0 may be declared CLOSED only when every gate below has evidence.

### G0 — State consensus

**Invariant:** P0 started from `prob-lio@60b57aa...` with a clean worktree.

**Evidence:** git commands and exact SHA.

**Forbidden substitute:** a prose statement without git evidence.

### G1 — Super-LIO production semantic freeze

**Invariant:** the six baseline semantics in §3.1 are verified in the actual production source.

**Evidence:** file/function/line references and/or a committed audit artifact that points to the authoritative code.

**Forbidden substitute:** copying the seam table from this prompt without checking source.

### G2 — FAST-LIVO2 active-semantics ledger

**Invariant:** future probability seams are grounded in the actual local FAST-LIVO2 active code.

**Evidence:** file/function/line references recorded in `SPEC.md`.

**Forbidden substitute:** paper-only claims or commented/inactive code presented as active.

### G3 — Build

**Invariant:** the baseline branch builds with the correct ROS1 build path.

**Evidence:** bounded build command, real return code, completion sentinel, persistent or reproducible log path.

### G4 — `eee_01` baseline run

**Invariant:** one bounded production-path run is executed against the resolved NTU `eee_01` dataset.

**Evidence:** command/config, real return code, output/result path, runtime evidence, and evaluation result if an authoritative evaluator is available.

**Forbidden substitute:** unit tests or synthetic input instead of the requested bag run.

### G5 — No algorithm change

**Invariant:** P0 does not change Super-LIO estimator/map/association production semantics.

**Evidence:** inspect the final diff against starting HEAD and classify every changed file.

Any unintended production algorithm change means P0 is not closed.

### G6 — SPEC/project layout

**Invariant:** `spec/prob_lio/SPEC.md` exists and accurately records roadmap, seams, P0 audit, evidence, and history; reusable artifacts are placed in appropriate persistent directories.

**Evidence:** committed files and final repository tree.

### G7 — Commit integrity

**Invariant:** all authorized P0 changes are committed on `prob-lio`; worktree is clean after commit.

**Evidence:** final commit SHA, `git status --short`, `git log -1 --oneline`.

Do not start P1 in the same commit.

---

## 8. Execution hygiene

Use bounded, observable commands.

For each build/test/run shell invocation:

- use one bounded operation per invocation;
- preserve the real return code;
- use `set -o pipefail` when piping through `tee`;
- emit an explicit completion sentinel such as `__P0_BUILD_DONE_RC=<rc>`;
- do not rerun an aborted/failed command merely because the UI still appears busy;
- first inspect whether the real process is still alive (`pgrep`, `ps`, ROS node/process inspection as appropriate);
- if a run must be cancelled, terminate it cleanly, verify no duplicate rosbag/node remains, then proceed;
- never allow two copies of the same bag playback/estimator run to contaminate one result.

Heavy instrumentation is not needed for P0 unless a concrete anomaly requires it.

---

## 9. Allowed modifications in P0

Authorized:

- project SPEC / documentation;
- prompt history/index;
- reusable generic run/evaluation helper needed for P0;
- lightweight project-directory README/index files;
- non-invasive baseline validation support.

Not authorized:

- covariance implementation;
- `calcBodyCov()` port;
- `OctVox` probability fields;
- QR covariance/Jacobian code;
- changing `1000`;
- changing `compute_error()`;
- changing HKNN semantics;
- changing ESKF equations;
- parameter tuning for accuracy;
- any P1–P5 implementation.

If a production-code change seems necessary merely to make baseline tooling run, prefer a wrapper/tooling fix first. If a true production fix is required, **STOP FOR OWNER** with evidence and a minimal proposed change.

---

## 10. Commit

After all P0 gates pass:

1. update `SPEC.md` to mark P0 `CLOSED/PASS`;
2. commit all authorized changes with a focused message, e.g.:

```text
chore(prob-lio): freeze P0 baseline and project spec
```

3. ensure the worktree is clean;
4. push the `prob-lio` branch normally if upstream is configured.

No force push.

---

## 11. Final report format

Return a concise but evidence-rich report with these sections:

### Agent State Consensus
- starting branch / HEAD
- upstream
- worktree state

### P0 Source Audit
- Super-LIO six baseline invariants with authoritative file/function/line evidence
- FAST-LIVO2 active-semantics ledger with authoritative evidence
- any discrepancy from the Owner's expected seam table

### Project Structure
- final relevant tree
- `SPEC.md` path
- prompt registration path
- reusable tools/eval/tests/results artifacts added and why

### Build Gate
- exact bounded command
- return code + sentinel
- log/evidence path

### NTU eee_01 Gate
- resolved bag/config/launch
- exact bounded command
- return code + sentinel
- trajectory/result/evaluation evidence
- effective-parameter snapshot if captured

### No-Algorithm-Change Gate
- diff classification against starting HEAD
- explicit confirmation whether any production estimator/map/association file changed

### Commit
- final commit SHA
- `git log -1 --oneline`
- clean worktree evidence
- push status

### Gate Summary
- G0..G7 PASS/FAIL with evidence path/reference for each
- P0 status in SPEC
- P1 status must remain `NOT STARTED`

### STOP / Owner Decision
If anything remains unresolved, classify it precisely and ask the Owner only for the decision actually needed. Do not convert ordinary path discovery or a harmless runner adjustment into a false non-closure.

---

## 12. Important review contract

Your final prose report is not sufficient by itself.

The Owner/reviewer will independently inspect:

- committed production/source diff;
- `SPEC.md`;
- reusable runner/eval/test tooling;
- result/evidence files;
- actual commands/return codes;
- the source locations cited for the semantic gates.

Therefore, make the repository and evidence self-explanatory. A gate is closed only when the **code/evidence** supports it, not because the report says PASS.
