# Super-LIVO Round 11W-P0R1 — Owner Audit Corrective
## Production S0 / Infra Evidence Reconciliation Before Round11W Resume

**Role:** DS/OpenCode implementation agent  
**Owner:** Super-LIVO Origin  
**Purpose:** correct concrete mismatches found by Owner review between the Round11W-P0 preemption report and the actual committed production/tooling code at `175e22b`, then resume the already-active Round11W **only if every corrective gate passes**.

---

# 0. INPUT STATE

Committed HEAD:

```text
175e22b
```

Active Round11W prompt:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

Round11W-P0 infra preemption prompt:

```text
prompts/04_v1_implementation/40_round11w_p0_infra_corrective_preemption.md
```

There is reported uncommitted authorized Round11W WIP in:

```text
src/super_lio/src/offline/OfflineReader.cpp
```

This WIP MUST NOT be lost, reset, silently rewritten, stashed away and forgotten, or mixed into unrelated commits.

Before any edit:

```bash
git status --short
git diff -- src/super_lio/src/offline/OfflineReader.cpp
git diff --stat
git rev-parse HEAD
```

Save the exact WIP diff under `/tmp` for evidence only, and include a SHA256 of that diff in the report.

Do NOT reset/rebase/rewrite history.

---

# 1. WHY THIS CORRECTIVE IS REQUIRED

Owner independently reviewed the actual committed code at `175e22b`.

The preemption report may NOT yet be accepted as closure evidence because several report claims do not match the committed implementation.

The following are **Owner-confirmed concrete findings**.

---

## F1 — Production pending-tail re-slice is alias-unsafe

Current caller:

```cpp
sliceLidarAt(
    t_c,
    lidar_buffer_,
    pending_lidar_,
    pending_lidar_,
    ...
);
```

passes the SAME `PendingLidarSlice` object as:

```text
pending_in
pending_out
```

But current `sliceLidarAt` does:

```cpp
pending_out.has = false;
pending_out.points.clear();

if (pending_in.has) {
    ...
}
```

Since input/output alias, clearing `pending_out` clears the same object referenced by `pending_in` **before it is read**.

Therefore the committed implementation does NOT prove repeated pending-tail re-slicing.

This is a production implementation bug under the already-frozen S0 semantics.

### Frozen required semantic

For every new camera epoch `tc`:

```text
pending point physical time <= tc
    -> current exactly once

pending point physical time > tc
    -> remains pending
```

No loss, duplicate, or wrong-side emission.

### Required corrective

Make `sliceLidarAt` alias-safe.

Preferred minimal approaches:

```text
A. move/copy pending_in to a local immutable snapshot before clearing output
or
B. change API so current pending is consumed by value/move and new pending is returned
```

Do NOT alter the frozen slicing rule.

Add an explicit TDD fixture where `pending_in` and `pending_out` are the SAME object. This fixture MUST fail on the pre-fix implementation and pass after correction.

---

## F2 — Production audit "raw_scan_id" is not actually a raw-scan ID

Current LiDAR ingestion assigns:

```cpp
p.audit_scan_id = s0_scan_seq_;
```

But current code increments:

```cpp
s0_scan_seq_++;
```

inside camera-epoch sync after a slice attempt.

Thus the ID source is tied to camera/slice progression rather than one monotonically increasing ID per raw LiDAR message.

That violates the frozen stable identity:

```text
(raw_scan_id, original_point_index)
```

### Required corrective

Create a dedicated raw-LiDAR-ingestion sequence counter.

Rules:

```text
increment exactly once per accepted raw LiDAR message
all selected points from that raw message share the same raw_scan_id
independent of number of camera epochs
independent of empty slices
independent of pending re-slices
```

Do not use camera epoch number as raw scan lineage.

Required TDD:
- two raw LiDAR scans before one camera epoch -> distinct scan IDs;
- one raw scan split across multiple camera epochs -> SAME raw scan ID for all its points;
- stable identity remains unique with original point index.

---

## F3 — Slice audit report claims integer-ns hard comparison, but committed oracle still uses float seconds

Current committed `audit_lidar_slice_accounting.py` stores:

```python
"time": float(start_time) + float(offset)
```

and compares float timestamps.

The preemption report states:

```text
integer-ns hard comparison: YES
```

That claim is not supported by the committed slice oracle.

### Required corrective

Change the independent slice oracle to integer nanoseconds end-to-end:

```text
scan_start_ns
offset_ns
point_time_ns
tc_ns
```

Hard rule:

```text
point_time_ns <= tc_ns -> current
point_time_ns >  tc_ns -> pending
```

No epsilon branch for the formal identity/time partition gate.

The production estimator may retain its existing double timestamp representation for this corrective; the independent oracle must provide exact integer-ns classification.

Required TDD:
- `point_time_ns == tc_ns` current exactly once;
- `tc_ns-1`, `tc_ns`, `tc_ns+1`;
- repeated pending re-slice.

---

## F4 — `sync_camera_epoch` entry guard contradicts pending coverage semantics

Current function begins effectively with:

```cpp
if (camera_buffer_.empty() ||
    lidar_buffer_.empty() ||
    imu_buffer_.empty()) {
    return false;
}
```

but immediately afterwards defines LiDAR coverage as:

```cpp
pending_lidar_.has ||
(!lidar_buffer_.empty() && lidar_buffer_.front().start_time <= t_c)
```

Therefore a camera epoch cannot progress when:

```text
lidar_buffer is empty
BUT
pending_lidar contains sufficient previously received points
```

even though the subsequent coverage logic explicitly treats pending as valid coverage.

### Owner-frozen correction

Pending LiDAR is valid already-received data.

The entry condition must permit processing when:

```text
camera exists
IMU exists
AND
(pending exists OR lidar_buffer exists)
```

This is not a new architecture; it is consistency with frozen S0 pending-slice semantics.

Required TDD:
- pending-only + adequate IMU + points <= tc -> legal current slice;
- pending-only + all points > tc -> existing empty-slice path, not fake emission;
- no pending + no LiDAR -> wait.

---

## F5 — Camera time offset is currently applied twice

Current production flow:

```cpp
HandleImage:
frame.timestamp = header + g_camera_time_offset

sync_camera_epoch:
t_c = cf.timestamp + g_camera_time_offset
```

The committed causal oracle explicitly mirrors `2 * offset`.

This is latent while current MCD offset is zero, but it becomes a real correctness bug for any future nonzero-offset dataset.

### Owner-frozen semantic

`/camera/time_offset` is applied exactly ONCE.

For this architecture:

```text
CameraFrame.timestamp
=
physical camera epoch after configured offset
```

Therefore:

```text
HandleImage:
frame.timestamp = header + offset

sync_camera_epoch:
t_c = cf.timestamp
```

Required TDD with a nonzero synthetic offset.

Do NOT tune or sweep offsets.

This corrective changes only the accidental double application, not any dataset-specific offset value.

---

# 2. VARIANT RUNNER AUDIT — CURRENT MATRIX IS NOT FULLY EXPLICIT

The committed `run_offline_variant.sh` claims:

```text
explicit variant matrix
no YAML-default reliance
```

but currently readbacks only:

```text
/camera/enabled
/lio/v4/apply
/lio/v4/outlier_gate
```

Production scheduler selection is controlled separately by:

```text
/lio/camera_epoch/enabled
```

through:

```cpp
if (g_lio_camera_epoch)
    sync_camera_epoch(...)
else
    sync_legacy_lidar_end(...)
```

Therefore the current runner does NOT prove B0/C0 scheduler semantics independent of YAML.

---

## 2.1 Required canonical variant contract

Reconstruct the exact last-known-good B0/C0/A0/A1 settings from:
- committed prompt history;
- known-good runner history;
- canonical run evidence.

Do not guess.

At minimum the following MUST be explicit and read back:

```text
/camera/enabled
/lio/camera_epoch/enabled
/lio/v4/apply
/lio/v4/outlier_gate
/lio/v2/skip_fd
/lio/hb0/enabled
```

Also explicitly freeze/read back any `v0/v2/vp/g0/g1` switches whose value changes experiment semantics in the current production code.

Minimum scheduler contract:

```text
B0:
camera = false
camera_epoch = false
apply = false
outlier_gate = false

C0:
camera = true
camera_epoch = true
apply = false
outlier_gate = false

A0:
camera = true
camera_epoch = true
apply = true
outlier_gate = false

A1:
camera = true
camera_epoch = true
apply = true
outlier_gate = true
```

Do not claim "explicit matrix" until every semantics-bearing variant switch is explicitly set and read back.

---

## 2.2 Single-variant primitive remains canonical

Keep:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

as the formal primitive.

The 4-stack convenience wrapper is NOT a substitute for individual bounded invocations.

Its fail-fast behavior may remain.

---

# 3. `filter_mcd.py` REPORT CLAIMS DO NOT MATCH COMMITTED CODE

Two concrete issues must be corrected.

---

## F6 — Original rosbag record `Time` object is not preserved exactly

Current code obtains:

```python
rec_ns = ts.to_nsec()
rec_s = ts.to_sec()
```

but writes:

```python
out.write(topic, msg, t=rospy.Time.from_sec(rec_s))
```

This reconstructs a `Time` from floating seconds.

That is not "preserve original Time object".

### Required correction

Carry the original `ts` object in the heap item and write:

```python
out.write(topic, msg, t=ts)
```

Heap ordering stays:

```text
(ts.to_nsec(), bag_index, per_bag_sequence)
```

Required synthetic-bag TDD:
- timestamps with nontrivial nanoseconds;
- output `getTime().to_nsec()` exactly equals input record time;
- equal record-time tie ordering remains deterministic.

---

## F7 — Not all opened source bags are deterministically closed

Current code creates:

```python
b = rosbag.Bag(path)
```

but appends to `bags` only in the `else` branch where no wanted topic exists.

Thus source bags with relevant topics are not included in the final:

```python
for b in bags:
    b.close()
```

### Required correction

Every opened bag handle must be owned in one deterministic collection and closed in `finally`, including exceptions during merge/write.

Required behavioral test or mock/fixture proving all handles close on:
- success;
- required-topic failure after open;
- write/iteration exception.

Do not satisfy this with source-string matching only.

---

# 4. INFRA TDD QUALITY — STRING PRESENCE IS NOT BEHAVIORAL PROOF

Current `test_infra_p0.py` checks several properties by reading source files and searching for strings.

Examples include:
- `to_nsec` text exists;
- `.partial` text exists;
- `roscore -p` text exists;
- manifest code contains `bag_info(args.bag)`.

These are useful smoke guards, but they do NOT justify statements such as:

```text
original Time preserved
all bag handles closed
runner variant semantics correct
cache provenance validated
```

### Required correction

Keep cheap static guards if useful, but add behavioral tests for the claims above.

Round closure evidence must distinguish:

```text
STATIC GUARD PASS
BEHAVIORAL TDD PASS
REAL-BAG EVIDENCE PASS
```

Do not collapse them into one "P1-P22 all PASS".

---

# 5. PERSISTENT CANONICAL BAG CACHE — MANIFEST CHECK IS STILL INCOMPLETE

The manifest currently records source paths/topics/convention/generator HEAD but `check` mainly validates the cached output bag itself:

```text
bag MD5
size
duration
topic counts
```

That proves cached-file integrity, but not that current source/provenance still matches the manifest.

### Required cache contract

Applies to MCD and every future dataset where expensive:

```text
raw/multiple bags
-> merge/filter/canonical bag
-> repeated downstream benchmark use
```

Canonical/merged bags:

```text
MUST be persistent local non-Git cache
MUST NOT live only under /tmp
```

`/tmp` remains valid for:
- trajectory;
- logs;
- temporary CSV;
- forensic outputs.

---

## 5.1 Cache admission

Before a generated bag becomes canonical cache:

```text
known topic/message-count provenance PASS
baseline semantic parity PASS
```

For current MCD Day10 B0, historical trajectory semantic gate remains:

```text
9931f96e2a2fe2f524982edc5fe19372
```

Do NOT confuse:
- cached `.bag` file MD5 = integrity fingerprint;
- B0 trajectory MD5 = estimator semantic parity.

---

## 5.2 Cache reuse

Direct reuse only if:

```text
cached bag exists
cached bag MD5 matches manifest
cached bag size/duration/topic counts match manifest
current source identities match manifest
selected topics match manifest
mode/convention match manifest
generator/filter semantic identity matches manifest
```

For source identity, use a cheap durable identity at minimum:

```text
absolute path
size
mtime_ns
```

Optionally store a one-time source hash if already available.

Also store a hash/identity of the actual generator script, not merely an arbitrary repository HEAD.

If provenance mismatch:

```text
do not silently rebuild
report cache MISS reason
rebuild only when authorized/expected
```

---

# 6. OFFLINE READER PRODUCTION CODE AUDIT

Owner also found a non-estimator instrumentation bug in committed `OfflineReader.cpp`.

For raw image dispatch, current code accumulates:

```cpp
t_compute_ms += nowMs() - t_c1;
t_compute_ms += nowMs() - t_c1;
```

twice.

Correct this duplicate accounting line.

This must not alter estimator execution.

---

# 6.1 Preserve and inspect the existing Round11W ready-drain WIP

The uncommitted ready-drain WIP is authorized Round11W work.

Before resuming:
1. preserve its exact starting diff;
2. complete this corrective without losing it;
3. after corrective commits, rebase the working diff conceptually by hand only if needed—NO history rewrite;
4. rerun its TDD against corrected production S0 semantics.

Do not claim ready-drain closure from the audit simulator alone.

The final Round11W result must prove the REAL `OfflineReader + ROSWrapper + SuperLIO` path drains all currently-ready cameras under actual record-order dispatch.

---

# 7. PRODUCTION S0 HARD CLOSURE AFTER CORRECTIVE

Before any new ATE interpretation, run the real Day10 closure.

Use the persistent canonical bag cache if valid.

Do NOT regenerate it if manifest/provenance check passes.

Required order:

```text
1. B0 non-regression
2. production S0 stable-identity audit
3. causal camera accounting
4. corrected C0
5. only then B0/C0 accuracy classification
```

---

## 7.1 B0 gate

B0 must remain the historical canonical trajectory MD5:

```text
9931f96e2a2fe2f524982edc5fe19372
```

If not:

```text
STOP FOR OWNER
```

Do not continue C0.

---

## 7.2 Real production stable-identity gate

On Day10 full C0 camera-epoch path with S0 audit enabled:

```text
identity = (raw_scan_id, original_point_index)
```

Require:

```text
duplicate = 0
lost = 0
wrong_side = 0
retained/emitted overlap = 0
```

Also prove aliasing fixture and raw scan lineage fixture passed.

Aggregate count conservation alone is insufficient.

---

## 7.3 Camera accounting

Report separately:

```text
input
emitted epochs
capacity evictions
stale drops
empty-slice drops
EOF unemitted
duplicate epochs
unexplained mismatch
```

Existing empty-slice code-path drops must remain explicitly named, not mislabeled as unknown loss.

Require:

```text
unexplained mismatch = 0
duplicate epochs = 0
emit_without_lidar_coverage = 0
emit_without_imu_coverage = 0
```

---

# 8. BLAST RADIUS

After Day10 S0 hard gates pass:

```text
eee C0 state-off blast
nya C0 state-off blast
```

Use the already-frozen state-off parity references for those datasets.

Do not open:
- Gate M FD;
- HB oracle;
- heavy profiler;
- sanitizer,

unless a production anomaly specifically requires one.

---

# 9. ROUND11W RESUME CONDITION

Only after ALL of the following are true may DS resume the remaining Round11W task:

```text
F1 alias-safe production pending re-slice PASS
F2 raw scan lineage ID PASS
F3 integer-ns independent oracle PASS
F4 pending-only coverage path PASS
F5 camera offset applied exactly once PASS
variant matrix explicit/readback PASS
filter original record Time exact PASS
filter all bag handles close PASS
cache provenance/reuse contract PASS
OfflineReader timing duplicate fixed
B0 MD5 PASS
Day10 production duplicate/lost/wrong_side PASS
camera accounting/causal availability PASS
eee/nya blast PASS
Architecture deviations = NONE
```

Then resume:

```text
Round11W #39
ready-camera drain
P1 point exact accounting
P2 camera accounting
W-C/W-T TDD
Day10 B0/C0
classification
```

If any gate fails:

```text
STOP FOR OWNER
```

Do not run Night08 or Oxford.

---

# 10. EXPLICIT NON-ACTIONS

Not authorized:

```text
header reorder
timestamp retuning
camera offset sweep
LiDAR offset change
slice boundary semantic change
minimum slice-size heuristic
skip/merge small slices
visual weight tuning
A0/A1 accuracy experiments
FEJ
exposure
normal refinement
Night08
Oxford
```

The only nonzero-offset change authorized here is removing accidental DOUBLE application of the existing configured camera offset.

---

# 11. REQUIRED SKILLS

Report actual use:

```text
/tdd
/diagnosing-bugs
/grill-with-docs only for genuine unresolved semantics
```

Do not claim a skill was used merely because it is installed.

---

# 12. EXECUTION HYGIENE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail for pipelines
preserve PIPESTATUS[0]
explicit completion sentinel
check pgrep/ps before rerun because of UI spinner
no duplicate bag/node processes
no broad pkill/killall
preserve first failure evidence
forward commits only
explicit git staging
never git add .
never git add -A
```

Push verification:

```text
PUSH_RC=<real rc>
git rev-list --left-right --count @{u}...HEAD
```

Do not infer push success from `git log`.

---

# 13. COMMIT DISCIPLINE

Do not rewrite `175e22b`.

Recommended forward commits:

```text
fix(super-livo): make S0 pending re-slice alias-safe and lineage-correct
test(super-livo): harden integer-ns S0 production oracles
fix(tools): make variant/filter/cache contracts behavioral and fail-closed
fix(offline): correct compute accounting and ready-camera drain
docs(super-livo): record post-audit S0 corrective evidence
```

Do not combine unrelated fixes if avoidable.

---

# 14. PROMPT REGISTRATION

Register this exact Owner prompt as the next canonical prompt:

```text
prompts/04_v1_implementation/41_round11w_p0r1_owner_audit_corrective.md
```

Update:

```text
prompts/README.md
```

Do NOT create a new independent architecture tracker for this corrective.

Append this corrective as a child/preemption section to the existing active Round11W tracker.

If prompt `#41` already exists:

```text
STOP FOR OWNER
```

Do not autonomously renumber.

---

# 15. FINAL REPORT FORMAT

```text
Round 11W-P0R1 Owner Audit Corrective

Starting HEAD:
175e22b

Ending HEAD:
...

Architecture deviations:
NONE

=== Existing Round11W WIP ===
OfflineReader WIP present at start:
YES/NO

starting WIP diff SHA256:
...

WIP preserved:
YES/NO

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Owner Finding F1 — Alias Safety ===
pre-fix alias fixture:
FAIL as expected / ...
post-fix:
PASS/FAIL

same-object pending_in/out:
PASS/FAIL

=== F2 — Raw Scan Lineage ===
two scans before one epoch:
PASS/FAIL

one scan across multiple epochs:
PASS/FAIL

identity collisions:
...

=== F3 — Integer-ns Oracle ===
formal oracle timestamp type:
integer ns

tc-1/tc/tc+1:
PASS/FAIL

=== F4 — Pending-only Coverage ===
pending only + ready:
PASS/FAIL

pending only + all future:
PASS/FAIL

no pending/no lidar:
PASS/FAIL

=== F5 — Camera Offset ===
nonzero synthetic:
...

offset application count:
1

PASS/FAIL

=== Variant Matrix ===
B0 readback:
camera=
camera_epoch=
apply=
gate=
...

C0 readback:
...

A0:
...

A1:
...

behavioral test:
PASS/FAIL

=== filter_mcd ===
heap key exact to_nsec:
PASS/FAIL

output record Time exact to_nsec:
PASS/FAIL

equal-time deterministic tie:
PASS/FAIL

all source bag handles close:
PASS/FAIL

partial output atomic:
PASS/FAIL

=== Cache ===
persistent non-/tmp cache path:
...

bag MD5:
...

source identity:
...

generator identity:
...

manifest check:
PASS/FAIL

cache reuse without rebuild:
PASS/FAIL

B0 semantic parity MD5:
9931f96e2a2fe2f524982edc5fe19372
PASS/FAIL

=== Production Day10 S0 ===
input selected points:
...

unique emitted:
...

final retained:
...

duplicates:
0/...

lost:
0/...

wrong_side:
0/...

retained/emitted overlap:
0/...

PASS/FAIL

=== Camera Accounting ===
input:
...
emitted:
...
capacity evictions:
...
stale:
...
empty slice:
...
EOF:
...
duplicate:
...
unexplained mismatch:
...

emit_without_lidar_coverage:
...
emit_without_imu_coverage:
...

PASS/FAIL

=== OfflineReader ===
duplicate timing accumulation removed:
YES/NO

ready-drain production TDD:
PASS/FAIL

=== Blast Radius ===
eee C0:
...

nya C0:
...

=== Push ===
PUSH_RC:
...

remote/local divergence:
...

=== Resume Decision ===
RESUME ROUND11W
or
STOP FOR OWNER

Reason:
...
```

---

# 16. OWNER STOP RULE

Do not report the previous P1-P22 preemption as fully closed until these production/tool mismatches are reconciled.

Most importantly:

```text
audit-oracle PASS
!=
production implementation PASS
```

and:

```text
source-string test PASS
!=
behavioral contract PASS
```

The purpose of this corrective is to make the committed implementation, executable tools, real-bag evidence, and final report all describe the SAME semantics.
