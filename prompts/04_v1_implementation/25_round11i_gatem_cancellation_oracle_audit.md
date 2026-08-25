# Super-LIVO Round 11I — Gate M Cancellation-Conditioned Numerical Oracle Audit
## Architecture-Owner Execution Contract for DS/OpenCode

> **Owner status at entry**
>
> - Round 11H Current HEAD reported: `213cac0`
> - Gate X: **PASS / CLOSED**
> - Gate X production correction: **accepted**
> - Post-fix eee Gate M: **formal FAIL only on `ry`**
> - Formal Gate M criterion remains unchanged in this round.
> - This round is **diagnostic only**: determine whether the remaining `ry` failure is a true Jacobian/path mismatch or a cancellation-conditioned finite-difference numerical limit.
>
> **Do not run nya.**
>
> **Do not modify Gate M thresholds.**
>
> **Do not add an absolute-error escape.**
>
> **Do not start state-off parity, PERF-0/TBB, or V-4.**
>
> End this round with **STOP FOR OWNER**.

---

# 0. OWNER RULING FROM ROUND 11H

Round 11H established:

```text
Gate X:
PASS / CLOSED
```

The following convention is now canonical:

\[
T_{CB}: B\rightarrow C
\]

\[
X_C=R_{CB}R_{WB}^{T}(X_W-p_{WB})+t_{CB}
\]

with right perturbation:

\[
R_{WB}^{+}=R_{WB}\operatorname{Exp}(\delta\theta)
\]

and:

\[
\frac{\partial X_C}{\partial\delta\theta}
=
[X_C-t_{CB}]_\times R_{CB}
\]

\[
\frac{\partial X_C}{\partial\delta p}
=
-R_{CB}R_{WB}^{T}.
\]

Do not reopen or redesign Gate X in this round unless new evidence directly contradicts the corrected parity tests.

---

# 1. CURRENT POST-GATE-X EEE RESULT TO PRESERVE

Round 11H post-fix eee 30 s:

```text
epochs:        13
landmarks:     443
trials:        2611
all6_smooth:   2475

rx: 4.59e-4 PASS
ry: 1.83e-2 FAIL
rz: 2.27e-4 PASS
tx: 9.41e-5 PASS
ty: 4.24e-4 PASS
tz: 2.37e-4 PASS

all max_abs < 3e-4
all med_rel ~ 5e-9
```

Current reported `ry` worst forensic:

```text
L1 Xc:
analytic ≈ (-0.0415904, 19.0595, 0.104263)
FD       ≈ (-0.0415905, 19.0595, 0.104263)

L2 uv:
analytic ≈ (-0.0001159, 423.128)
FD       ≈ (-0.000117936, 423.128)

L3 raw:
analytic ≈ 636.98
FD       ≈ 636.98

L4 mean:
analytic ≈ 636.977
FD       ≈ 636.977

L5 DC:
analytic = 0.00332923
FD       = 0.00326953
abs diff ≈ 5.97e-5
rel diff ≈ 1.83e-2
```

Important Owner interpretation:

```text
This is NOT the old Gate-X-scale error.
```

The current residual derivative is strongly cancellation-conditioned:

\[
J_{DC}=J_{raw}-J_{mean}
\]

with two terms near `637` leaving a result near `0.0033`.

Do not call this sample simply "near zero".

Use:

```text
strong-but-cancellation-conditioned DC derivative
```

because:

\[
|J_{FD}|\approx 3.27\times10^{-3}
\]

is still above the formal strong threshold:

\[
10^{-3}.
\]

---

# 2. PURPOSE OF ROUND 11I

Answer one question only:

> For the exact current post-Gate-X eee `ry` worst sample, does the analytic-vs-FD discrepancy converge like a central-difference truncation/roundoff phenomenon under a fully frozen smooth bundle, or does a non-vanishing L1/L2/L3/L4/L5 mismatch remain?

This is not a new parameter sweep.

This is not a performance experiment.

This is not a new Gate definition.

---

# 3. REQUIRED SKILLS

Mandatory:

```text
/diagnosing-bugs
/tdd
```

Use `/grill-with-docs` only if an ambiguity in the already-frozen Round 11H equations prevents a deterministic implementation.

Final report must contain:

```text
=== Skills Used ===
/diagnosing-bugs:
...

/tdd:
...

/grill-with-docs:
NOT USED / ...
```

---

# 4. PROMPT REGISTRATION — P0 BEFORE IMPLEMENTATION

Register this exact Owner prompt.

Required:

```text
Super-LIO/prompts/
Super-LIO/prompts/README.md
.scratch/super-livo-v1/issues/
```

Expected next prompt number is likely `25`, but inspect existing repository numbering first.

Suggested canonical name if available:

```text
prompts/04_v1_implementation/25_round11i_gatem_cancellation_oracle_audit.md
```

README entry:

```text
Round 11I
Gate M cancellation-conditioned numerical oracle audit
initial HEAD = actual HEAD at start
status ACTIVE
predecessor = Round 11H
```

Active tracker must point to Round 11I.

No prompt registration = no Round 11I PASS.

---

# 5. SPINNER-SAFE EXECUTION — MANDATORY

Follow:

```text
docs/super_livo/execution_hygiene.md
```


---

# 5A. P0 DATASET / SENSOR / CONFIG PROVENANCE PREFLIGHT — UNCONDITIONAL

This preflight is mandatory **before any Round 11I numerical work**, even if no eee replay is required.

Reason: `/tmp/opencode/tb0` was cleaned and the runner was reconstructed. The workspace contains:

```text
results/super_livo/tb0/config/
  eee_01_tb0_offline.yaml
  eee_01_tb0.yaml
  livox_360_tb0.yaml
```

The presence and purpose of `livox_360_tb0.yaml` are currently **NOT Owner-verified**.

Do not assume it is harmless historical residue merely from its filename.

DS must determine exactly:

```text
what livox_360_tb0.yaml contains
why it exists
who/what references it
whether the reconstructed dbg_v0c.sh references it directly or indirectly
whether eee_01_tb0_offline.yaml includes/inherits/copies any of it
whether any ROS parameter loaded for the eee experiment is subsequently overridden by it
```

## 5A.1 Inspect all three config files

Run read-only inspection first:

```bash
cd /home/lc/super_livo

for f in \
  results/super_livo/tb0/config/eee_01_tb0_offline.yaml \
  results/super_livo/tb0/config/eee_01_tb0.yaml \
  results/super_livo/tb0/config/livox_360_tb0.yaml
do
  echo "========== $f =========="
  readlink -f "$f"
  stat "$f"
  sha256sum "$f"
  sed -n '1,260p' "$f"
done
```

Save the output.

Do not modify any config during this inspection.

## 5A.2 Find every reference to these configs

Search the runner and workspace scripts/configuration for the exact filenames:

```bash
grep -RsnE \
  'livox_360_tb0\.yaml|eee_01_tb0_offline\.yaml|eee_01_tb0\.yaml' \
  /home/lc/super_livo/src/Super-LIO \
  /home/lc/super_livo/results/super_livo \
  /tmp/opencode/tb0 \
  2>/dev/null || true
```

Also inspect the reconstructed runner verbatim:

```bash
sed -n '1,260p' /tmp/opencode/tb0/dbg_v0c.sh
```

Report:

```text
runner-selected config:
direct references:
indirect references:
parameter overrides after load:
```

## 5A.3 Establish the eee sensor contract from the actual bag/config

The eee_01 bag is an Ouster dataset, not Livox Mid360.

The bag evidence currently shows Ouster streams such as:

```text
/os1_cloud_node1/points
/os1_cloud_node1/imu
/os1_cloud_node2/points
/os1_cloud_node2/imu
```

For the actual Super-LIVO eee run, determine from the production config/loader:

```text
selected point topic
selected IMU topic
selected Ouster unit (node1/node2)
sensor/preprocess type
point timestamp field interpretation
scan-line/ring handling if applicable
blind/range filtering if applicable
LiDAR-IMU extrinsic file/source
camera calibration file/source
```

Do not assume parameter names. Trace the keys actually consumed by the Super-LIO loader.

The current expected point stream is:

```text
/os1_cloud_node1/points
```

If the active eee config selects Livox/Mid360 semantics, or a Livox custom-message preprocessing path:

```text
CONFIG PROVENANCE GATE = FAIL
STOP FOR OWNER
```

No Gate-M numerical result from that run is valid.

## 5A.4 Explain `livox_360_tb0.yaml`

Final evidence must classify it as exactly one of:

```text
A. unrelated historical/template config, not referenced by eee runner
B. referenced only by another dataset/tool, not eee
C. included/copied/used by eee path but with sensor-neutral fields only
D. actively contributing Livox/Mid360 parameters to eee
E. unresolved
```

For A/B/C, prove the classification from file contents + references + effective parameters.

For D/E:

```text
STOP FOR OWNER
```

Do not continue the Gate-M epsilon audit.

---

# 5B. P0 CAMERA EXTRINSIC REGRESSION SENTINEL — UNCONDITIONAL

Gate X is closed, but Round 11I must verify that the **actual current eee execution path still receives the corrected extrinsic semantics** after runner reconstruction.

This is a regression sentinel, not a redesign of Gate X.

Before Gate-M numerical work, record:

```text
actual camera calibration path used by eee
SHA256 of that calibration
YAML T_Body_Cam matrix
production loader stored matrix
T_cam_body() matrix
```

Required semantics:

\[
T_{Body\_Cam}=T_{BC}: C\rightarrow B
\]

\[
T_{\text{cam\_body}}=T_{CB}=T_{BC}^{-1}: B\rightarrow C
\]

Check:

\[
\|T_{CB}T_{BC}-I\|
\]

and report the numerical closure.

Run the existing corrected `extrinsic_parity_test` once, spinner-safe.

It must verify the post-fix relationship:

```text
FAST/direct A
==
mathematically correct camera-pose B
==
corrected production path
```

for:

```text
Xc
Jrot
Jtrans
```

The test must use the current corrected implementation, not merely the old hard-coded pre-fix reproduction.

If the production loader path used by eee does not yield `T_CB`, or the corrected parity test fails:

```text
GATE X REGRESSION = FAIL
STOP FOR OWNER
```

Do not perform the cancellation/epsilon audit.

## 5B.1 If an eee node is actually running, capture the live process provenance

While it is running:

```bash
PID=$(pgrep -n -f 'super_lio_offline_node')
echo "PID=$PID"

if [ -n "$PID" ]; then
  echo "=== exact cmdline ==="
  tr '\0' ' ' < "/proc/$PID/cmdline"
  echo

  echo "=== process tree ==="
  pstree -aps "$PID" || true

  echo "=== open config-like files ==="
  lsof -p "$PID" 2>/dev/null | grep -Ei '\.ya?ml|config' || true
fi
```

`lsof` may be empty after YAML load; this is not proof by itself.

If ROS parameters are used, capture the effective parameter server state:

```bash
rosparam dump /tmp/opencode/tb0/effective_rosparams_round11i.yaml
```

and inspect the actual sensor/topic/extrinsic-related keys consumed by Super-LIO.

Save the dump into Round11I raw evidence.

## 5B.2 Hard preflight decision

Before numerical Gate-M work, print exactly:

```text
=== ROUND11I CONFIG / EXTRINSIC PREFLIGHT ===
dataset:
bag:
runner:
runner config:
runner config sha256:

livox_360_tb0.yaml classification:
referenced by eee runner:
YES/NO
reason:

effective LiDAR:
effective point topic:
effective IMU topic:
effective preprocess branch:

camera calibration:
camera calibration sha256:
YAML T_Body_Cam direction:
T_cam_body direction:
T_CB*T_BC closure:

extrinsic_parity_test:
PASS/FAIL

CONFIG PROVENANCE GATE:
PASS/FAIL

GATE X REGRESSION SENTINEL:
PASS/FAIL
```

Only if both gates are PASS may Round 11I continue.

Additionally:

1. One bounded operation per shell invocation.
2. Preserve real return code.
3. Use `set -o pipefail` with `tee`.
4. Print explicit completion sentinel.
5. Before rerunning because OpenCode spins, check the actual OS process.
6. Never rerun an exited assert merely because the UI is still spinning.
7. Do not run `catkin build` twice just to grep its output.

For a test:

```bash
set -o pipefail
set +e

timeout --signal=TERM --kill-after=5s 90s \
  <COMMAND> 2>&1 | tee <LOG>

rc=${PIPESTATUS[0]}
echo "__CMD_DONE_RC=${rc}__"
exit "${rc}"
```

Interpret an assertion/SIGABRT as completed FAIL evidence.

---

# 6. DO NOT RERUN EEE 30 S UNLESS THE EXACT WORST IDENTITY IS MISSING

First inspect existing Round 11H evidence:

```text
docs/super_livo/evidence/raw/round11h/gate_m_eee/
```

and current logs.

Recover the exact current worst:

```text
dataset
epoch timestamp
landmark_id
sample/reference index
direction = ry / d=1
active_ref_slot
bundle sample identities
base u/v cells
```

If all identifiers are already available:

```text
DO NOT rerun the 30 s eee experiment.
```

If the old log omitted a required identity field and the exact worst cannot be reconstructed:

```text
one eee 30 s replay is authorized solely to capture deterministic worst identity
```

subject to spinner-safe execution.

Do not run nya.

---

# 7. FREEZE THE EXACT SAME WORST BUNDLE ACROSS ALL EPSILON VALUES

For the selected worst trial, freeze once at the base state:

```text
dataset
camera epoch
landmark_id
sample k
direction ry
active_ref_slot

P_patch
n_sync
reference body pose
reference camera pose
reference observation
reference rays

ref_idx list
world sample X_W for every sample
reference intensity for every sample
```

No epsilon case may rebuild or reselect:

```text
landmark
reference observation
active_ref_slot
plane
reference ray
world intersection X_W
ref_idx
```

Only the **current body rotation** changes.

---

# 8. DIAGNOSTIC EPSILON SET — EXACT

Use exactly:

```text
3e-6
1e-6
3e-7
1e-7
3e-8
1e-8
```

The formal Gate M epsilon remains:

```text
1e-6
```

This sweep does not authorize a formal epsilon change.

Do not add more epsilon values unless an implementation bug requires a repeat after correction.

---

# 9. BUNDLE-SMOOTHNESS MUST BE RECHECKED AT EVERY EPSILON

For every epsilon, for the **entire DC bundle**, verify:

```text
same sample count at base / plus / minus
same sample identities
all samples valid at base / plus / minus
same floor(u) at base / plus / minus
same floor(v) at base / plus / minus
same reference support
```

Per-epsilon classification:

```text
SMOOTH
NON_SMOOTH_SUPPORT
NON_SMOOTH_CELL
```

A nonsmooth epsilon point:

```text
must be reported
must NOT be included in convergence fitting
must NOT be silently replaced by another sample
```

Do not introduce a small-depth exclusion.

---

# 10. PRINT FULL DOUBLE PRECISION

Current output such as:

```text
636.98
636.977
```

is insufficient.

Use at least:

```cpp
%.17g
```

or scientific equivalent with 16–17 significant digits.

For every smooth epsilon print all required values below.

---

# 11. L1 — CAMERA POINT DERIVATIVE

For `ry`, print the full 3-vector:

```text
L1 analytic dXc/dry
L1 FD       dXc/dry
L1 error    analytic - FD
L1 max_abs_component
L1 l2_abs
L1 relative_l2
```

FD:

\[
C_{X_c}(\epsilon)
=
\frac{X_c(+\epsilon)-X_c(-\epsilon)}
{2\epsilon}.
\]

Do not report only a rounded tuple.

---

# 12. L2 — PROJECTION DERIVATIVE

Print separately:

```text
du_analytic
du_FD
du_abs
du_rel

dv_analytic
dv_FD
dv_abs
dv_rel
```

with full precision.

This level is currently important because Round 11H first visible discrepancy was reported in the small `du/dry` component:

```text
~ -1.16e-4 vs -1.18e-4
```

while `dv/dry ~ 423`.

Do not describe L2 as "all consistent" if `du` itself has a measurable discrepancy.

---

# 13. L3 — RAW PHOTOMETRIC DERIVATIVE

For the exact sample `k`:

\[
J_{raw}
=
[I_u\ I_v]
\begin{bmatrix}
du/dry\\
dv/dry
\end{bmatrix}.
\]

Print:

```text
Iu
Iv

Jraw_analytic
Jraw_FD
Jraw_abs
Jraw_rel
```

Also print the contribution decomposition:

```text
Iu * du/dry   analytic / FD
Iv * dv/dry   analytic / FD
```

This tells whether the small `du` discrepancy materially contributes to `Jraw`.

---

# 14. L4 — BUNDLE MEAN DERIVATIVE

The DC mean derivative must use exactly the same frozen smooth bundle.

Print:

```text
M = bundle size

Jmean_analytic
Jmean_FD
Jmean_abs
Jmean_rel
```

Additionally print the largest per-sample raw derivative error inside the bundle:

```text
bundle_raw_max_abs_error
bundle_raw_med_abs_error
bundle_raw_max_rel_error on DOUBLE_STRONG raw terms
```

Do not change the Gate M strong definition; this is diagnostic reporting only.

---

# 15. L5 — DC DERIVATIVE

For the selected sample:

\[
J_{DC}=J_{raw}-J_{mean}.
\]

Print:

```text
Jdc_analytic
Jdc_FD_direct
Jdc_abs
Jdc_rel
```

Also independently compute:

\[
J_{DC,FD}^{closure}
=
J_{raw,FD}-J_{mean,FD}.
\]

Print:

```text
Jdc_FD_direct
Jdc_FD_closure
dc_fd_closure_abs
```

If:

```text
dc_fd_closure_abs > 1e-7
```

then treat this as a numerical-path/harness anomaly requiring diagnosis before interpreting convergence.

Do not modify the formal Gate M based on this auxiliary threshold.

---

# 16. CANCELLATION CONDITION METRICS

For each smooth epsilon, compute:

\[
\kappa_{DC}
=
\frac{|J_{raw}|+|J_{mean}|}
{\max(|J_{DC}|,10^{-30})}.
\]

Report separately using analytic and FD values:

```text
kappa_dc_analytic
kappa_dc_fd
```

Also report:

```text
raw_to_dc_ratio  = |Jraw| / max(|Jdc|, 1e-30)
mean_to_dc_ratio = |Jmean| / max(|Jdc|, 1e-30)
```

These are diagnostics, not gates.

---

# 17. ERROR-PROPAGATION CHECK

For each epsilon define:

\[
e_{raw}=|J_{raw}^{A}-J_{raw}^{C}|
\]

\[
e_{mean}=|J_{mean}^{A}-J_{mean}^{C}|
\]

\[
e_{DC}=|J_{DC}^{A}-J_{DC}^{C}|.
\]

Print:

```text
e_raw
e_mean
e_dc
e_raw_plus_e_mean
e_dc / max(e_raw + e_mean, 1e-30)
```

Expected triangle bound:

\[
e_{DC}\le e_{raw}+e_{mean}
\]

up to numerical closure effects.

Diagnostic rule:

```text
if e_dc > e_raw + e_mean + 1e-7:
    classify NUMERICAL_PATH_INCONSISTENCY
    STOP FOR OWNER
```

This is an internal consistency check, not a new Gate-M acceptance rule.

---

# 18. CONVERGENCE TABLE

Create one CSV row per epsilon with at least:

```text
eps

smooth_class
support_same
cells_same

L1_abs
L1_rel

du_abs
du_rel
dv_abs
dv_rel

Jraw_an
Jraw_fd
Jraw_abs
Jraw_rel

Jmean_an
Jmean_fd
Jmean_abs
Jmean_rel

Jdc_an
Jdc_fd
Jdc_abs
Jdc_rel

Jdc_fd_closure
dc_fd_closure_abs

kappa_dc_an
kappa_dc_fd

e_raw
e_mean
e_dc
e_raw_plus_e_mean
```

Save raw CSV, do not rely on log parsing only.

---

# 19. QUANTITATIVE R1 / R2 / R3 CLASSIFICATION

Do not classify by intuition.

Use only smooth epsilon points.

## R1 — CENTRAL-FD TRUNCATION / ROUNDOFF SUPPORTED

Classify `R1` only if:

1. At least **three consecutive smooth epsilon points** exist before the minimum-error point.
2. For the relevant discrepancy (`du_abs`, `Jraw_abs`, `Jmean_abs`, or `Jdc_abs`), the log-log slope between adjacent pre-roundoff points is broadly consistent with second-order central FD:

\[
s=
\frac{\log(e_i/e_{i+1})}
{\log(\epsilon_i/\epsilon_{i+1})}.
\]

Require for at least two adjacent intervals:

```text
1.5 <= slope <= 2.5
```

3. At smaller epsilon, error may plateau or increase due to roundoff.
4. L1/L2/L3/L4/L5 must not show a non-vanishing structural offset inconsistent with that trend.
5. All used points must be bundle-smooth.

Then report:

```text
classification = R1
central-difference truncation/roundoff hypothesis SUPPORTED
```

Do **not** change formal epsilon or Gate M.

---

## R2 — PLATEAU / FORMULA OR PATH MISMATCH

Classify `R2` if, over at least three smooth decreasing epsilon values:

```text
error does not materially decrease
```

Quantitatively, if the absolute error changes by less than a factor of 2 while epsilon decreases by at least 10× overall, and there is no preceding clear second-order regime.

Then:

```text
classification = R2
analytic/residual-path mismatch remains plausible
```

STOP FOR OWNER.

---

## R3 — HIDDEN NONSMOOTH / DISCONTINUOUS BEHAVIOR

Classify `R3` if:

```text
support/cell changes
or
error changes discontinuously with epsilon
or
bundle identity changes
```

and those changes explain the instability.

Report exact sample/cell causing the event.

STOP FOR OWNER.

---

## R4 — ROUND-OFF DOMINATED FROM THE START

If all smaller epsilons are smooth but there are not enough pre-roundoff points to establish a second-order region, and error is already at a small absolute floor:

```text
classification = R4
insufficient evidence to distinguish truncation from roundoff
```

STOP FOR OWNER.

Do not promote R4 to PASS.

---

# 20. FORMAL GATE M IS FROZEN IN ROUND 11I

Keep exactly:

```text
formal eps = 1e-6

DOUBLE_STRONG:
|fd_double| >= 1e-3

BUNDLE_SMOOTH

formal strong gate:
max_rel < 1e-2
```

Forbidden:

```text
median escape
max_abs escape
new atol
mixed tolerance
condition-aware PASS
rotation-specific tolerance
depth gate
adaptive epsilon
weakening strong threshold
```

Round 11I only supplies evidence for the Owner to decide whether a future Gate-M semantic revision is justified.

---

# 21. DO NOT RUN NYA

Regardless of whether the diagnostic sweep looks excellent:

```text
nya = NOT RUN
```

Round 11I ends at Owner review.

---

# 22. DO NOT RUN STATE-OFF PARITY

Round 11H state-off parity remains pending.

Do not spend this round on it.

Reason:

```text
Gate M criterion has not yet been adjudicated.
```

---

# 23. DO NOT TOUCH H/b AUDIT

Current status remains:

```text
H/b audit = NOT VALIDATED
```

Do not resurrect the old same-source double-vs-double `0` result.

No H/b work in Round 11I.

---

# 24. DO NOT TOUCH PERF-0/TBB OR V-4

Explicit:

```text
PERF-0/TBB = NOT STARTED
V-4 = BLOCKED
```

No exceptions.

---

# 25. TDD REQUIRED

At minimum add/maintain tests for:

1. Epsilon-specific whole-bundle smoothness:
   - same support
   - same sample identity
   - same bilinear cell

2. DC derivative closure:
   - direct FD DC
   - FD raw minus FD mean
   - both agree within the diagnostic numerical budget.

3. Frozen reference geometry across all epsilons.

4. High-precision L1/L2 diagnostic helper:
   - no float cast.

5. Convergence-classification helper if classification is implemented in code:
   - synthetic \(O(\epsilon^2)\) sequence -> R1
   - plateau -> R2
   - nonsmooth point -> R3
   - roundoff-only/no second-order region -> R4

Do not overengineer a new generic framework if a small testable helper suffices.

---

# 26. RAW EVIDENCE

Create:

```text
docs/super_livo/evidence/raw/round11i/
```

Required:

```text
initial_head.txt
initial_status.txt
prompt_registration.txt

config_directory_audit.txt
config_reference_search.txt
runner_source.txt
runner_selected_config.txt
livox_360_classification.txt
effective_rosparams.yaml
camera_extrinsic_provenance.txt
extrinsic_parity_regression.txt

worst_identity.txt
frozen_bundle.txt

epsilon_sweep.csv
epsilon_sweep.log

l1_high_precision.txt
l2_high_precision.txt
l3_high_precision.txt
l4_high_precision.txt
l5_high_precision.txt

dc_closure.txt
error_propagation.txt
classification.txt

commands.txt
return_codes.txt
```

If an eee replay was required solely to recover identity:

```text
identity_capture_eee/
```

must include config path/hash/effective ROS params and the real command rc.

---

# 27. EXPERIMENT CONFIG PROVENANCE — REPLAY-SPECIFIC ADDITION

The unconditional config/sensor/extrinsic provenance is already required by §5A–§5B.

If any eee replay is additionally performed, record before node execution:

```text
absolute config path
SHA256 of config
full config content
effective ROS params after load
```

Expected dataset identity:

```text
NTU VIRAL eee_01
Ouster OS1-16
/os1_cloud_node1/points
```

Do not use `livox_360_tb0.yaml`.

At runner startup print:

```bash
echo "========== ACTIVE CONFIG =========="
echo "__CONFIG_PATH=$(readlink -f "$CFG")__"
sha256sum "$CFG"
sed -n '1,240p' "$CFG"
echo "========== END CONFIG =========="
```

After ROS params are loaded:

```bash
rosparam dump <RAW_EVIDENCE>/effective_rosparams.yaml
```

This section's replay-capture additions apply only if a replay is needed; the §5A–§5B provenance gates are unconditional.

---

# 28. EVIDENCE DOCUMENT

Create:

```text
docs/super_livo/evidence/v2_gatem_cancellation_oracle_round11i.md
```

Required sections:

```text
Prompt registration
Round11H entry state
Gate X closed status
Exact worst identity
Frozen bundle proof
Per-epsilon smoothness
High-precision L1
High-precision L2
High-precision L3
High-precision L4
High-precision L5
DC closure
Cancellation condition metrics
Error propagation
Convergence slopes
R1/R2/R3/R4 classification
Formal Gate M unchanged
Owner stop
```

---

# 29. GIT DISCIPLINE

Forward commits only.

Forbidden:

```bash
git add .
git add -A
git reset --hard
git checkout -- .
```

Explicit staging only.

Suggested commits:

```text
docs(super-livo): register round11i gate-m cancellation audit
test(super-livo): cover bundle-smooth epsilon diagnostics
feat(debug): add high-precision gate-m cancellation oracle
docs(super-livo): record round11i numerical convergence evidence
```

Do not modify production residual semantics in this round.

---

# 30. ROUND 11I DECISION LOGIC

## Case I-A — clear R1

If:

```text
whole bundle smooth
high-precision L1/L2/L3/L4/L5 captured
central-FD error shows second-order convergence before roundoff
DC error is consistent with raw/mean error propagation
```

then report:

```text
Round11I diagnostic = R1
Gate M formal status = STILL FAIL under current max_rel criterion
Recommendation = Owner may now redesign Gate M numerical acceptance
```

Then STOP FOR OWNER.

Do not implement mixed tolerance.

---

## Case I-B — R2

If error plateaus:

```text
Round11I diagnostic = R2
Gate M = genuine unresolved discrepancy
```

Identify the first non-converging level.

STOP FOR OWNER.

---

## Case I-C — R3

If hidden nonsmoothness is found:

```text
Round11I diagnostic = R3
```

Identify exact sample/cell/support transition.

STOP FOR OWNER.

---

## Case I-D — R4

If the data are already roundoff-dominated and no second-order regime can be established:

```text
Round11I diagnostic = R4
```

Report numerical floor.

STOP FOR OWNER.

---

# 31. FINAL REPORT FORMAT

Use exactly:

```text
Round 11I Gate M Cancellation-Conditioned Numerical Oracle Audit

Initial HEAD:
...

Current HEAD:
...

Architecture deviations:
NONE

=== Skills Used ===
/diagnosing-bugs:
/tdd:
/grill-with-docs:

=== Prompt Registration ===
canonical:
prompts/README:
active tracker:

=== Entry State ===
Gate X:
PASS / CLOSED

Round11H eee Gate M:
ry FAIL = 1.83e-2
other 5 directions PASS

formal Gate M changed:
NO

nya run:
NO

=== Config / Sensor / Extrinsic Preflight ===
runner config:
runner config sha256:
livox_360 classification:
livox_360 referenced by eee:
effective LiDAR:
effective point topic:
effective IMU topic:
effective preprocess branch:
camera calibration:
camera calibration sha256:
T_Body_Cam direction:
T_cam_body direction:
T_CB*T_BC closure:
extrinsic_parity_test:
CONFIG PROVENANCE GATE:
GATE X REGRESSION SENTINEL:

=== Worst Identity ===
dataset:
epoch:
landmark_id:
sample k:
direction:
active_ref_slot:
bundle size:

identity recovered without replay:
YES/NO

if replayed:
config path:
config sha256:
effective lidar:
effective point topic:

=== Frozen Bundle ===
P_patch unchanged:
n_sync unchanged:
reference pose unchanged:
active_ref unchanged:
ref_idx unchanged:
world X unchanged:
PASS/FAIL

=== Per-Epsilon Smoothness ===
3e-6:
1e-6:
3e-7:
1e-7:
3e-8:
1e-8:

=== High-Precision L1 ===
eps table:
...

=== High-Precision L2 ===
du table:
...
dv table:
...

=== High-Precision L3 ===
Iu:
Iv:
Iu*du:
Iv*dv:
Jraw table:
...

=== High-Precision L4 ===
Jmean table:
...
bundle raw max abs:
bundle raw median abs:

=== High-Precision L5 ===
Jdc table:
...

=== DC Closure ===
Jdc_fd_direct:
Jraw_fd - Jmean_fd:
closure abs:
PASS/ANOMALY

=== Cancellation Conditioning ===
kappa_dc analytic:
kappa_dc fd:
raw/DC ratio:
mean/DC ratio:

=== Error Propagation ===
e_raw:
e_mean:
e_dc:
e_raw+e_mean:
ratio:
PASS/ANOMALY

=== Convergence Slopes ===
metric:
eps_i -> eps_j:
slope:
...

=== Classification ===
R1 / R2 / R3 / R4

Reason:
...

=== Formal Gate M ===
formal eps:
1e-6

formal strong threshold:
1e-3

formal max_rel:
1e-2

changed:
NO

eee formal status:
FAIL pending Owner criterion decision

=== Deferred ===
nya:
NOT RUN

state-off parity:
NOT RUN

H/b:
NOT VALIDATED

PERF-0/TBB:
NOT STARTED

V-4:
BLOCKED

=== Raw Evidence ===
...

Round 11I:
PASS-TO-OWNER / BLOCKED

Next:
STOP FOR OWNER
```

---

# 32. NON-NEGOTIABLE

The purpose of this round is to determine whether the final `1.83e-2` is:

```text
a real derivative/path bug
```

or:

```text
the expected numerical amplification from differentiating
Jdc = Jraw - Jmean
when Jraw and Jmean nearly cancel.
```

Do not decide that question by looking only at:

```text
median
max_abs
one rounded L3/L4 line
```

Prove it with:

```text
same exact bundle
full precision
per-epsilon smoothness
L1→L5
central-FD convergence
DC closure
error propagation
```

Then:

```text
STOP FOR OWNER
```

No new Gate-M semantics are authorized in Round 11I.
