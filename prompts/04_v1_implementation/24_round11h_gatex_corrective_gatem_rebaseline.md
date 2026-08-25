# Super-LIVO Round 11H — Gate X Extrinsic Convention Corrective + Gate M Re-baseline
## Architecture-Owner Execution Contract for DS/OpenCode
### Includes mandatory spinner-safe execution discipline

> **Owner decision:** Round 11G has produced strong pre-fix evidence that the visual residual path is composing the camera pose with the wrong direction of the camera/body extrinsic.  
> **Current expected committed HEAD:** `bd323d8`, but **do not assume** after the manual OpenCode interruption; record the actual HEAD on resume.  
> **Working tree:** may contain uncommitted Round 11G parity-test/CMake/debug changes. **Preserve them.**
>
> This prompt **authorizes one specific production correction** *only if* the provenance and three-path parity proof below confirm:
>
> \[
> T_{\text{cam\_body}} = T_{CB}: B\rightarrow C
> \]
>
> while current residual code consumes it as if it were:
>
> \[
> T_{BC}: C\rightarrow B.
> \]
>
> If that premise is not proven, **STOP FOR OWNER**.
>
> **Still forbidden:** PERF-0/TBB, V-4, depth gates, residual redesign, threshold relaxation, adaptive epsilon, warp redesign.

---

# 0. IMMEDIATE RESUME INSTRUCTION AFTER USER MANUALLY INTERRUPTS OPENCODE

The user is manually interrupting an **OpenCode completion spinner**, not intentionally cancelling the C++ test logic.

The previous visible test already reached:

```text
Assertion `(Xc_A - Xc_B).norm() < 1e-9' failed.
```

and therefore likely terminated via `SIGABRT` (`rc≈134`).

On resume:

1. **Do not rerun the test first.**
2. **Do not reset/stash/discard the working tree.**
3. First capture process state, HEAD, working tree, and the existing failure evidence.
4. Treat an already-exited assertion failure as **completed FAIL evidence**, not as a hanging test.

First action:

```bash
cd /home/lc/super_livo/src/Super-LIO

printf '\n=== ROUND11H RESUME ===\n'
git rev-parse HEAD
git status --short

pgrep -af 'extrinsic_parity_test|catkin|make|cc1plus|super_lio_offline_node' || true

ps -eo pid,ppid,stat,wchan:28,etime,%cpu,cmd \
  | grep -E 'extrinsic_parity_test|catkin|make|cc1plus|super_lio_offline_node' \
  | grep -v grep || true
```

If no actual test/build process exists:

```text
PREVIOUS TEST STATUS = COMPLETED FAIL
OPENCODE STATUS = COMPLETION SPINNER ONLY
```

Do **not** rerun just to make the UI stop spinning.

Preserve the existing pre-fix output:

```text
Xc_A=(1.321953652,1.548332306,2.140713292)
Xc_P(current-as-coded)=(1.626762772,1.997254548,1.186853274)
||Xc_A-Xc_P|| ≈ 1.1 m

||Jp_A-Jp_P|| ≈ 2.48

Jtheta_A == Jtheta_correct
Jtheta_P(current-as-coded) differs strongly
```

Save the original failure output/diff under Round11H raw evidence before modifying the test.

---

# 1. PERMANENT OPENCODE / DS SPINNER-SAFE EXECUTION CONTRACT

This section is **mandatory for this round and all future Super-LIVO execution work**.

The purpose is to prevent a finished process from being mistaken for a still-running experiment.

## 1.1 One bounded operation per shell invocation

Do not write long command chains like:

```bash
build && test && run && grep && rerun && analyze
```

Each of these must be a separate execution step:

```text
build
test
dataset run
post-process
```

A nonzero return code ends that step.

---

## 1.2 Every important command must expose its true return code

For tests/experiments use a completion sentinel.

Example:

```bash
set -o pipefail
set +e

timeout --signal=TERM --kill-after=5s 60s \
  ./devel/.private/super_lio/lib/super_lio/extrinsic_parity_test \
  2>&1 | tee /tmp/extrinsic_parity_test.log

rc=${PIPESTATUS[0]}
echo "__CMD_DONE_RC=${rc}__"
exit "${rc}"
```

Interpretation:

```text
rc=0   -> PASS/completed
rc=134 -> assert/SIGABRT/completed FAIL
rc=124 -> timeout/hang candidate
other  -> completed failure unless process evidence says otherwise
```

An assertion failure is not a spinner/hang.

---

## 1.3 Never hide the real rc behind `grep`

Forbidden pattern:

```bash
catkin build super_lio 2>&1 | grep ...
```

followed by another build just to inspect the summary.

Instead:

```bash
set -o pipefail
set +e

catkin build super_lio 2>&1 | tee /tmp/round11h_build.log
rc=${PIPESTATUS[0]}
echo "__BUILD_DONE_RC=${rc}__"
exit "${rc}"
```

Then, in a **separate** shell invocation:

```bash
grep -E 'Summary|Failed|error:' /tmp/round11h_build.log | tail -30
```

Exactly one build per requested build step.

---

## 1.4 Before rerunning a command because OpenCode appears stuck

Always first check:

```bash
pgrep -af '<actual-process-name>' || true
```

and, if needed:

```bash
ps -eo pid,ppid,stat,wchan:28,etime,%cpu,cmd \
  | grep '<actual-process-name>' \
  | grep -v grep || true
```

If the process is gone and a result/rc/assert is already visible:

```text
DO NOT RERUN
```

The user may manually interrupt the OpenCode spinner and resume from evidence.

---

## 1.5 No accidental process cleanup

Never use broad cleanup such as:

```bash
pkill roscore
pkill -f ros
killall ...
```

A runner may kill only processes it started.

For ROS/offline scripts:

```text
trap cleanup EXIT INT TERM
cleanup only self-started roscore/node
preserve original node/test rc
```

Do not kill an existing user roscore.

---

## 1.6 Avoid interactive/pager stalls

For scripted work use:

```bash
export GIT_PAGER=cat
export PAGER=cat
```

No interactive editors, pagers, prompts, or commands waiting for stdin in automated execution.

---

## 1.7 Do not background experimental processes unless explicitly required

Avoid:

```bash
command &
```

unless the process lifecycle is owned by a wrapper that records PID, traps cleanup, and preserves rc.

---

## 1.8 Add persistent repository documentation

Record these execution rules in the repository.

Prefer an existing execution/recovery document if one already exists. Otherwise create:

```text
docs/super_livo/execution_hygiene.md
```

It must include:

```text
one bounded operation per invocation
explicit rc sentinel
pipefail/PIPESTATUS
assert/SIGABRT = completed FAIL
process check before rerun
no duplicate builds for grep
scoped cleanup
no interactive pager
user may interrupt completion spinner without invalidating finished evidence
```

Reference this document from future Super-LIVO Owner prompts / tracker.

---

# 2. REQUIRED SKILLS

Mandatory:

```text
/diagnosing-bugs
/tdd
```

Use `/grill-with-docs` only if source/config provenance still leaves transform direction ambiguous.

Final report must explicitly state how each skill was used.

---

# 3. PROMPT REGISTRATION — P0

Before production modification, register this **exact Round 11H prompt**.

Required:

```text
Super-LIO/prompts/
Super-LIO/prompts/README.md
.scratch/super-livo-v1/issues/
```

Requirements:

1. Inspect existing numbering first.
2. Preserve existing Round11D/E/F/G history.
3. Save this exact prompt under the canonical implementation history.
4. README entry must include:
   - Round11H
   - purpose
   - actual initial committed HEAD captured after resume
   - ACTIVE
   - predecessor Round11G
5. Active tracker must point to Round11H.

No prompt registration = no Round11H PASS.

---

# 4. FIRST PRESERVE THE PRE-FIX GATE-X EVIDENCE

Before editing the parity test or production residual, save:

```text
git HEAD
git status
git diff
original extrinsic_parity_test source
original test stdout/stderr
original rc if recoverable
```

under:

```text
docs/super_livo/evidence/raw/round11h/prefix/
```

Do not lose the evidence that current-as-coded produced approximately:

```text
Xc diff ≈ 1.1 m
Jp diff ≈ 2.48
```

The current assertion failure is evidence, not something to erase.

---

# 5. LOCK THE FRAME NOTATION

Use only this notation in code comments/evidence:

## Body -> Camera

\[
T_{CB}: X_C=R_{CB}X_B+t_{CB}
\]

## Camera -> Body

\[
T_{BC}=T_{CB}^{-1}
\]

\[
X_B=R_{BC}X_C+t_{BC}
\]

with ideal rigid-transform relation:

\[
R_{BC}=R_{CB}^{T}
\]

\[
t_{BC}=-R_{CB}^{T}t_{CB}
\]

Do not use ambiguous variable names like `R_bc` for a matrix actually representing `R_CB`.

After the correction:

```text
R_CB/t_CB always mean Body -> Camera
R_BC/t_BC always mean Camera -> Body
```

---

# 6. PROVENANCE GATE — PROVE `T_cam_body()` FIRST

From source/config, prove all of the following with exact file:function:line:

```text
YAML field name:
T_Body_Cam

its documented/tf meaning:
Camera -> Body = T_BC   [verify, do not merely repeat]

loader operation:
does it invert?

CameraCalibration stored matrix:
what equation does it implement?

T_cam_body():
what exact transform does it return?
```

The expected current finding to verify is:

```text
YAML T_Body_Cam = T_BC
T_cam_body() = inverse(T_Body_Cam) = T_CB
```

Then verify frontend usage:

\[
X_B=R_{WB}^{T}(X_W-p_{WB})
\]

\[
X_C=T_{\text{cam\_body}}X_B
\]

which requires `T_cam_body() = T_CB`.

### If provenance does NOT uniquely establish this

```text
STOP FOR OWNER
```

Do not apply the correction below.

---

# 7. REWRITE THE PARITY TEST AS THREE EXPLICIT PATHS

The existing test currently conflates “correct camera-pose path” and “production-as-coded path”.

Fix the experimental design first.

Use:

```text
A = FAST/direct correct path
B = mathematically correct camera-pose path
P = current production-as-coded path
```

## 7.1 Path A — FAST/direct

Given:

```text
R_WB, p_WB, T_CB, X_W
```

compute:

\[
X_B=R_{WB}^{T}(X_W-p_{WB})
\]

\[
X_C^A=R_{CB}X_B+t_{CB}
\]

For right perturbation:

\[
J_{\theta}^{A}=[X_C^A-t_{CB}]_\times R_{CB}
\]

\[
J_p^{A}=-R_{CB}R_{WB}^{T}
\]

---

## 7.2 Path B — mathematically correct camera-world pose

First invert the extrinsic:

\[
T_{BC}=T_{CB}^{-1}
\]

Then:

\[
T_{WC}=T_{WB}T_{BC}
\]

or explicitly:

\[
R_{WC}=R_{WB}R_{BC}
\]

\[
p_{WC}=p_{WB}+R_{WB}t_{BC}
\]

and:

\[
X_C^B=R_{WC}^{T}(X_W-p_{WC})
\]

Derive `Jtheta_B`, `Jp_B` consistently.

Mandatory hard assertions:

```text
A.Xc == B.Xc
A.Jrot == B.Jrot
A.Jtrans == B.Jtrans
```

---

## 7.3 Path P — current production-as-coded, pre-fix only

Reproduce exactly the current suspicious composition, e.g. if current code does:

```text
R_WC^P = R_WB * R_CB
p_WC^P = p_WB + R_WB * t_CB
```

record:

```text
Xc_P
Jrot_P
Jtrans_P
```

Do **not** call this Path B.

This is the pre-fix production path `P`.

Expected pre-fix evidence, if the Owner diagnosis is correct:

```text
A == B
A != P
```

Do not add a permanent unit test that intentionally fails after production is corrected.

Instead preserve the pre-fix mismatch in evidence, then convert the persistent test to:

```text
A == B == corrected production helper
```

---

# 8. SYNTHETIC TEST AND NTU REAL-CALIBRATION TEST MUST BE SEPARATE

## 8.1 Pure synthetic TDD

Build `R_CB` and `R_WB` from exact `Eigen::AngleAxisd` SO(3) matrices.

Use nonzero lever arm.

Use strict double tolerance determined from the numerical floor, approximately `1e-10` or tighter if observed.

Must test:

```text
Xc
Jrot
Jtrans
```

---

## 8.2 NTU real calibration audit

Load actual NTU eee calibration through the production loader.

Do not manually copy the YAML matrix as the sole production test.

Report:

```text
det(R_CB)-1
||R_CB^T R_CB - I||
T_CB
T_BC
```

Because text calibration may not be perfectly orthonormal, do not force an arbitrary `1e-9` tolerance without measuring the floor.

Do not orthonormalize production calibration merely to make the test pass.

Use the exact transform semantics actually supplied by the loader.

---

# 9. OWNER-AUTHORIZED PRODUCTION CORRECTION — ONLY AFTER A==B AND PROVENANCE PASS

If and only if:

```text
T_cam_body() = T_CB is proven
A == B PASS
P != A is reproduced
```

then the Owner **authorizes this exact correction without another intermediate STOP**.

Canonical visual calibration representation:

\[
\boxed{T_{CB}: Body\rightarrow Camera}
\]

The frontend convention remains canonical.

---

# 10. CORRECT CURRENT WORLD-POINT -> CAMERA TRANSFORM

For current body pose:

\[
T_{WB}=(R_{WB},p_{WB})
\]

compute the current camera point directly:

\[
X_B=R_{WB}^{T}(X_W-p_{WB})
\]

\[
\boxed{
X_C=R_{CB}X_B+t_{CB}
}
\]

Prefer this direct form in the visual residual/Jacobian/FD hot path.

Do not construct `T_WC` from `T_CB` as if `T_CB` were `T_BC`.

---

# 11. CORRECT CAMERA WORLD POSE WHEN IT IS ACTUALLY NEEDED

For reference rays / camera-origin geometry where `T_WC` is genuinely required:

\[
T_{BC}=T_{CB}^{-1}
\]

then:

\[
\boxed{
T_{WC}=T_{WB}T_{BC}
}
\]

so:

\[
R_{WC}=R_{WB}R_{BC}
\]

\[
p_{WC}=p_{WB}+R_{WB}t_{BC}
\]

Use this for reference camera center/rays.

Do not mix it with the direct current-point transform.

---

# 12. CORRECT RIGHT-PERTURBATION JACOBIAN

The body state remains:

\[
R_{WB}^{+}=R_{WB}\operatorname{Exp}(\delta\theta)
\]

No change to the ESKF rotation convention.

For:

\[
X_C=R_{CB}R_{WB}^{T}(X_W-p_{WB})+t_{CB}
\]

the Owner-approved geometric Jacobians are:

\[
\boxed{
\frac{\partial X_C}{\partial\delta\theta}
=
[X_C-t_{CB}]_\times R_{CB}
}
\]

and:

\[
\boxed{
\frac{\partial X_C}{\partial\delta p}
=
-R_{CB}R_{WB}^{T}
}
\]

These must match the FAST-LIVO2 forward semantics under the frame mapping established in Round11G.

Do not change sign/transposes experimentally until a TDD failure proves this contract wrong.

---

# 13. CORRECT THE DOUBLE FD ORACLE TO DIFFERENTIATE THE SAME PHYSICAL RESIDUAL

For Gate M:

### Rotation

\[
R_{WB}^{\pm}
=
R_{WB}\operatorname{Exp}(\pm\epsilon e_i)
\]

with `p_WB` frozen.

### Translation

\[
p_{WB}^{\pm}
=
p_{WB}\pm\epsilon e_i
\]

with `R_WB` frozen.

At each perturbation recompute current `X_C` using:

\[
X_C^\pm
=
R_{CB}(R_{WB}^{\pm})^{T}(X_W-p_{WB}^{\pm})+t_{CB}
\]

Do not perturb/recompute reference-side geometry.

Keep frozen:

```text
P_patch
n_sync
reference observation
active_ref_slot
reference body pose
reference camera pose
reference rays
world plane-intersection sample X_W
ref_idx
reference intensity
```

Only the **current body pose** is perturbed.

---

# 14. AUDIT EVERY VISUAL EXTRINSIC CALL-SITE AFTER THE FIX

Search all uses of:

```text
T_cam_body()
R_cb / R_bc
t_cb / t_bc
camera pose composition
```

Classify and correct them consistently.

At minimum:

```text
frontend projection
landmark creation
observation sampling
reference ray construction
runVisualResidual
double analytic B
double FD C
synthetic tests
NTU parity test
any shadow diagnostic
```

Do not leave compensating inversions in different call-sites.

---

# 15. REQUIRED TDD AFTER PRODUCTION CORRECTION

Persistent tests must include:

### Geometry parity
```text
synthetic A == B
corrected production Xc == A
corrected production Jrot == A
corrected production Jtrans == A
```

### Numerical geometry FD
For each of 6 pose DOFs:

```text
analytic dXc/dxi vs double FD Xc
```

using strict double precision.

### NTU loader parity
```text
frontend current Xc
corrected residual current Xc
FAST/direct Xc
```

must agree within measured calibration/numerical floor.

### Reference camera pose
Verify:

```text
T_WC == T_WB * inverse(T_CB)
```

not `T_WB*T_CB`.

---

# 16. HISTORICAL EVIDENCE POLICY

Once Gate X is fixed, mark all real-image Gate-M numbers obtained under the old physical residual as:

```text
HISTORICAL RESULT — PRE-GATE-X EXTRINSIC CONVENTION BUG
NOT VALID FOR POST-FIX GATE-M ACCEPTANCE
```

Do not delete them.

Do not use the old:

```text
eee ry/rz
nya rx/ry/rz
epsilon sweeps
```

to judge the corrected residual.

The correction can change:

```text
Xc
projection
valid support
bilinear cells
visible landmarks
raw/DC Jacobians
```

so Gate M must be re-baselined.

---

# 17. GATE M RE-BASELINE AFTER GATE X PASS

Only after:

```text
Gate X corrected
all parity TDD PASS
state apply remains OFF
```

rerun **eee_01 30s first**.

Formal Gate M remains unchanged:

```text
eps_double = 1e-6
DOUBLE_STRONG: |fd_double| >= 1e-3
BUNDLE_SMOOTH
same support/cells
strong max_rel < 1e-2
```

No:

```text
median escape
max-abs escape
depth filter
near-zero exception
adaptive epsilon
rotation-specific tolerance
```

Required eee report per direction:

```text
strong_n
strong_max_rel
strong_med_rel
max_abs
bundle_smooth count
```

If any direction fails:

1. capture the **new post-Gate-X** worst sample;
2. run current-sample L1→L5 A/B/C decomposition;
3. STOP FOR OWNER.

Do not proceed to nya after an eee Gate-M failure.

---

# 18. NYA AFTER EEE PASS

Only if eee Gate M fully passes:

run `nya_01 30s`.

Same Gate M semantics.

If Gate M passes but `distinct_epochs < 5`, then extend the run only enough to satisfy coverage.

Do not run full nya before correctness is established.

If nya fails:

```text
capture new post-fix worst
STOP FOR OWNER
```

---

# 19. STATE-OFF PARITY

Visual state application remains OFF.

The extrinsic correction changes shadow visual measurements, not LIO state.

Therefore verify existing LIO state/trajectory parity against the correct C0 baseline for eee/nya.

Any state-off trajectory change:

```text
REGRESSION
STOP
```

---

# 20. H/b AUDIT STATUS

Previous:

```text
worst_h_rel=0
worst_b_rel=0
```

is **not accepted as valid production numeric evidence** until the audit compares:

```text
actual production float HTVH/HTVr
vs
independently accumulated double H/b
```

Do not claim H/b exact parity from two double accumulators derived from identical A/B vectors.

This is not the first blocker in Round11H.

If Gate M passes and you touch H/b audit, it must:

1. accumulate the reference entirely in double;
2. obtain production values from the actual float accumulator;
3. compare before any shared cast;
4. print scientific notation;
5. accumulate each bundle once, not once per FD direction.

Otherwise label:

```text
H/b audit: NOT VALIDATED
```

Do not let this distract from Gate X/Gate M.

---

# 21. RAW EVIDENCE

Create:

```text
docs/super_livo/evidence/raw/round11h/
```

Suggested layout:

```text
prefix/
  resume_process_state.txt
  initial_head.txt
  initial_status.txt
  initial_diff.patch
  pre_fix_parity_source.cpp
  pre_fix_parity_output.txt

provenance/
  calibration_trace.txt
  transform_callsite_table.md
  fast_livo2_frame_mapping.md

parity/
  synthetic_parity.txt
  ntu_loader_parity.txt
  production_corrected_parity.txt

gate_m_eee/
  command.txt
  rc.txt
  stdout.log
  gate_m.csv

gate_m_nya/
  ... only if eee PASS
```

---

# 22. EVIDENCE DOCUMENT

Create:

```text
docs/super_livo/evidence/v2_gate_x_corrective_round11h.md
```

Required sections:

1. manual-spinner resume state
2. spinner-safe execution rules
3. prompt registration
4. pre-fix failure preserved
5. calibration provenance
6. exact `T_CB/T_BC` equations
7. three-path A/B/P experiment
8. Gate X adjudication
9. production correction
10. all visual call-sites audited
11. synthetic TDD
12. NTU loader parity
13. post-fix Gate M eee
14. post-fix Gate M nya if permitted
15. state-off parity
16. historical results superseded
17. H/b audit status
18. STOP FOR OWNER

---

# 23. GIT DISCIPLINE

Forward commits only.

Do not:

```bash
git reset --hard
git checkout -- .
git add .
git add -A
```

because the working tree may contain useful interrupted Round11G evidence.

Explicit staging only.

Suggested commits:

```text
docs(super-livo): register round11h gate-x corrective
docs(super-livo): record spinner-safe execution discipline
test(super-livo): prove camera extrinsic three-path parity
fix(super-livo): correct body-to-camera visual transform semantics
test(super-livo): validate corrected visual pose jacobian
docs(super-livo): record post-gate-x gate-m evidence
```

Do not commit unrelated local artifacts.

---

# 24. EXACT STOP / CONTINUE LOGIC

## Case X-A — provenance disproves Owner premise

If `T_cam_body()` is not actually `T_CB`:

```text
NO production correction
STOP FOR OWNER
```

with exact equations.

## Case X-B — A != B

The mathematical parity test itself is wrong/incomplete:

```text
NO production correction
diagnose test
STOP FOR OWNER if frame semantics remain ambiguous
```

## Case X-C — A == B and P != A

This is the expected current result:

```text
Gate X pre-fix = FAIL
Owner-authorized correction = APPLY
```

Apply exactly Sections 9–14.

Then rerun parity.

## Case X-D — corrected production == A == B

```text
Gate X post-fix = PASS
```

Proceed to eee Gate M.

## Case M-E — eee Gate M FAIL

```text
STOP FOR OWNER
```

Do not run nya/TBB/V-4.

## Case M-P — eee PASS, nya PASS

```text
Round11H correctness evidence complete
STOP FOR OWNER
```

Still do not start PERF-0/TBB/V-4 automatically.

---

# 25. FINAL REPORT FORMAT

```text
Round 11H Gate X Corrective + Gate M Re-baseline

Initial committed HEAD:
(actual after resume)

Current HEAD:

Initial working tree preserved:
YES/NO

Architecture deviations:
NONE

=== Skills Used ===
/diagnosing-bugs:
/tdd:
/grill-with-docs:

=== Spinner-Safe Execution ===
previous extrinsic process existed on resume:
YES/NO

previous test classification:
COMPLETED FAIL / ACTUALLY RUNNING / UNKNOWN

pre-fix evidence preserved:
YES/NO

execution hygiene doc:
path

duplicate build/rerun due spinner:
NONE

=== Prompt Registration ===
canonical:
prompts/README:
active tracker:

=== Calibration Provenance ===
YAML field:
YAML equation:
loader operation:
T_cam_body equation:
final accessor direction:
T_CB / T_BC

=== Three-Path Gate X ===
Path A FAST/direct Xc:
Path B correct camera-pose Xc:
Path P pre-fix production Xc:

||A-B||:
||A-P||:

Jrot ||A-B||:
Jrot ||A-P||:

Jtrans ||A-B||:
Jtrans ||A-P||:

pre-fix Gate X:
PASS/FAIL

=== Production Correction ===
authorized premise satisfied:
YES/NO

canonical stored visual extrinsic:
T_CB

current point transform:
equation

reference camera-pose transform:
equation

rotation Jacobian:
equation

translation Jacobian:
equation

affected call-sites:
...

=== Post-Fix TDD ===
synthetic Xc:
synthetic Jrot:
synthetic Jtrans:
6DOF Xc FD:
NTU loader Xc:
NTU Jrot:
NTU Jtrans:

Gate X post-fix:
PASS/FAIL

=== Historical Gate M ===
old pre-fix results marked superseded:
YES/NO

=== eee Post-Fix Gate M ===
command rc:
epochs:
landmarks:
all6_smooth:

rx:
ry:
rz:
tx:
ty:
tz:

Gate M eee:
PASS/FAIL

=== nya Post-Fix Gate M ===
NOT RUN because eee failed
or:
...

=== State-Off Parity ===
eee:
nya:

=== H/b Audit ===
VALID / NOT VALIDATED
Do not report old zero result as proof.

=== Raw Evidence ===
...

=== Gates ===
prompt registration:
spinner-safe execution:
provenance:
A==B:
P mismatch reproduced:
production fix:
post-fix parity:
eee Gate M:
nya Gate M:
state-off:

Round 11H:
PASS-TO-OWNER / BLOCKED

PERF-0/TBB:
NOT STARTED

V-4:
BLOCKED

Next:
STOP FOR OWNER
```

---

# 26. NON-NEGOTIABLE OWNER RULES

1. Do not rerun a test merely because OpenCode still shows a spinner.
2. An exited `assert` is a completed FAIL, not a hang.
3. Check the real OS process before rerunning.
4. Preserve rc with `pipefail`/`PIPESTATUS` and print a completion sentinel.
5. Do not run the same `catkin build` twice just to grep its output.
6. Do not hide test/build rc behind `grep`.
7. Do not delete/reset the manually interrupted working tree.
8. Do not “try an inverse” experimentally in production.
9. Prove `T_CB/T_BC`, prove A==B, reproduce P!=A, **then** apply the Owner-authorized correction.
10. Do not use old pre-fix Gate-M data to validate the corrected residual.
11. Do not change Gate-M epsilon/threshold/depth semantics.
12. Do not start TBB or V-4.
13. End with **STOP FOR OWNER**.
