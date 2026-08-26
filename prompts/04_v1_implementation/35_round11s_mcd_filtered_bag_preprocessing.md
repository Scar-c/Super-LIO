# Super-LIVO — MCD Canonical Filtered-Bag Preprocessing + Prompt Registration Cleanup

You are continuing the Super-LIVO Round-11 implementation.

This is an execution task, not a redesign task.

Repository:

```text
~/super_livo/src/Super-LIO
branch: super-livo
```

Reference repositories remain READ ONLY.

---

# 0. Owner Intent

We have identified that repeated MCD Day10 B0/C0/A0/A1 benchmarking currently pays unnecessary runtime cost from:

- multiple ROS1 bags;
- runtime multi-bag merge;
- BZ2 decompression;
- irrelevant image topics;
- repeated traversal of data not required by a given benchmark mode.

We want to create **canonical preprocessed benchmark bags** once, then use those for repeated offline evaluation.

This preprocessing must preserve estimator semantics.

Do NOT alter sensor timestamps or calibration semantics.

---

# 1. Mandatory Prompt Registration Hygiene

Before implementation, register this exact Owner prompt into the repository's canonical prompt history under the appropriate:

```text
prompts/04_v1_implementation/
```

Use the next correct sequential prompt number/name according to the current repository state.

Also:

1. update:

```text
prompts/README.md
```

2. update the active tracker under:

```text
.scratch/super-livo-v1/issues/
```

to reference the canonical registered prompt.

## CRITICAL: registration is MOVE semantics, not COPY-and-leave-duplicate semantics

The Owner has noticed that the **latest two registered prompts were copied into the canonical prompt directory while their original `.md` files were left behind**.

This is NOT the desired repository hygiene.

After successful canonical registration:

```text
old/transient/source prompt .md
        ↓
canonical prompts/... .md
```

there must not remain two active duplicate prompt documents representing the same Owner instruction.

### Required behavior

Prefer:

```bash
git mv <old_prompt_path> <canonical_prompt_path>
```

or:

```bash
mv <old_prompt_path> <canonical_prompt_path>
```

when appropriate.

If for some reason registration requires creating/copying the canonical file first:

```bash
cp old.md canonical.md
```

then:

1. verify the canonical copy is complete;
2. verify README/tracker reference the canonical path;
3. delete the obsolete source:

```bash
rm old.md
```

4. include deletion in the same logical commit.

### Also audit the latest two prompt registrations

Find the latest two cases where:

- a canonical registered prompt exists under `prompts/...`;
- an older/transient `.md` containing the same prompt was accidentally retained.

For each case:

- identify canonical vs obsolete source;
- verify the canonical document is complete;
- remove only the obsolete duplicate;
- do NOT delete historical prompts that are genuinely distinct revisions;
- do NOT delete the canonical registered copy.

Report exactly which duplicate files were removed.

The intended invariant is:

```text
one Owner prompt
→ one canonical registered prompt history entry
```

not:

```text
one Owner prompt
→ old.md
→ cp → canonical.md
→ both files remain forever
```

---

# 2. MCD Problem Being Solved

Current MCD Day10 source includes large BZ2-compressed sensor bags.

Example D435i bag:

```text
duration ≈ 324.8 s
compressed ≈ 6.5 GB
uncompressed ≈ 14 GB
```

Contains approximately:

```text
/d435i/color/image_raw
/d435i/infra1/image_rect_raw
/d435i/infra2/image_rect_raw
/d435i/imu
/os_cloud_node/imu
```

plus LiDAR data in the corresponding MCD sensor bag(s).

For our current benchmark:

### B0 needs only

```text
LiDAR
required estimator IMU
```

No RGB image traversal should occur.

### C0 / A0 / A1 need

```text
LiDAR
required estimator IMU
RGB camera
```

Infra1/infra2 are not part of the current Super-LIVO visual frontend and should not be present in the canonical benchmark bag unless actual current code proves otherwise.

Do not guess topic names.

First audit actual MCD Day10 bags and runner configuration.

---

# 3. Deliverables

Create a deterministic preprocessing tool under the repository, not only `/tmp`.

Suggested location:

```text
tools/offline/
```

or another existing canonical tools location if the repo already has one.

The tool must produce:

```text
MCD Day10 LIO canonical bag
    LiDAR
    required IMU only

MCD Day10 LIVO canonical bag
    LiDAR
    required IMU
    RGB camera
```

Suggested names, adjusted to current dataset conventions:

```text
ntu_day_10_lio_filtered.bag
ntu_day_10_livo_filtered.bag
```

Do not overwrite raw source bags.

Raw bags are immutable inputs.

---

# 4. ROS1 Merge Semantics

Prefer the ROS1-native bag APIs over maintaining another custom estimator-time merge state machine.

For preprocessing, it is acceptable to implement a deterministic k-way stream writer or use an appropriate ROS1 `rosbag::View`/Python rosbag mechanism, but the resulting output MUST satisfy all invariants below.

Do NOT reuse the previously broken OfflineReader iterator logic blindly.

The preprocessing stage is independent of estimator synchronization.

---

# 5. Timestamp Contract — HARD REQUIREMENT

Do NOT rewrite:

```text
msg.header.stamp
```

Do NOT fake timestamps.

Do NOT normalize timestamps to zero.

Do NOT replace sensor timestamps with wall clock.

Preserve each message exactly.

When writing a message into the filtered bag, preserve the original bag record time:

```python
outbag.write(topic, msg, t=original_record_time)
```

The architecture remains:

```text
bag MessageInstance / record time
→ storage ordering / preprocessing / bag-level filtering

sensor header.stamp
→ estimator physical time / synchronization
```

This contract must not change.

---

# 6. Deterministic Ordering

The merged output stream must be globally non-decreasing in original bag record time.

For equal bag timestamps, ordering must be deterministic.

Use an explicit stable tie-break rule such as:

```text
record_time
source_index
per-source sequence number
```

Document it.

Every selected source message must be emitted:

```text
exactly once
```

No replay.

No omission.

No iterator stall.

No duplicate dispatch.

---

# 7. Topic Audit First

Before writing the final preprocessing command, print and record:

```bash
rosbag info <all relevant MCD Day10 bags>
```

Identify:

- actual LiDAR PointCloud2 bag;
- actual LiDAR topic;
- actual estimator IMU source;
- actual RGB camera topic;
- whether any additional required topic exists.

Do NOT assume `/os_cloud_node/points` if the actual bag differs.

Do NOT accidentally choose `/os_cloud_node/imu` merely because it exists if the current MCD configuration officially uses `/d435i/imu`.

Use the already frozen dataset configuration/provenance.

If there is ambiguity, stop and report the evidence before selecting a different IMU.

---

# 8. Output Compression

For repeated benchmark cache bags, default to:

```text
LZ4
```

not BZ2.

Reason:

```text
raw archival bag:
    preserve original compression/input

benchmark cache:
    optimize repeated decoding/iteration
```

Do not replace/delete/recompress the original dataset.

If ROS1 environment lacks usable LZ4 support, report that and use uncompressed output rather than silently falling back to BZ2.

Record:

```text
compression
output size
preprocessing wall time
```

---

# 9. Required Preprocessing Validation

Before any estimator benchmark, validate both generated bags.

For each output bag report:

```text
duration
start time
end time
size
compression
message count
topic counts
```

Compare against source relevant-message counts.

Required invariants:

## LIO bag

```text
LiDAR count == selected source LiDAR count
IMU count   == selected source IMU count

RGB          absent
infra1       absent
infra2       absent
```

## LIVO bag

```text
LiDAR count == selected source LiDAR count
IMU count   == selected source IMU count
RGB count   == selected source RGB count

infra1      absent
infra2      absent
```

Also validate:

```text
first record time
last record time
non-decreasing record time
no duplicate emitted source sequence
```

---

# 10. Header Timestamp Parity

Add an explicit parity audit.

For every retained topic, compare source vs filtered output for at least:

```text
message count
first header.stamp
last header.stamp
```

For a deterministic sampled subset, compare exact header timestamp sequence.

Preferred stronger check if practical:

```text
hash of ordered retained-topic:
(topic, header.stamp, relevant sequence identity)
```

The preprocessing tool must not silently modify header stamps.

---

# 11. Synthetic Merge Test Before MCD

Add a small deterministic test for the preprocessing/merge logic.

At minimum cover:

### T1 — two ordered streams

```text
A: 1,3,5
B: 2,4,6
→ 1,2,3,4,5,6
```

### T2 — one source reaches EOF early

No replay of the other source.

### T3 — equal timestamps

Stable deterministic ordering.

### T4 — empty source

Other source passes exactly once.

### T5 — topic filtering

Irrelevant topics excluded.

### T6 — header timestamps preserved

Record-time ordering must not alter message header timestamps.

### T7 — output count

```text
output_count
==
sum(selected_input_counts)
```

No duplicate / no loss.

---

# 12. Short Real MCD Validation

After synthetic tests PASS, create a short MCD slice first.

Use approximately:

```text
1–2 s
```

of Day10.

Verify:

```text
record time advances;
header sensor time advances;
LiDAR count reasonable;
IMU count reasonable;
RGB count reasonable;
no replay;
no huge epoch count;
no NaN/Inf;
clean EOF.
```

Only then generate FULL canonical bags.

---

# 13. Benchmark Runner Update

Update MCD benchmark runners so that:

### B0 uses

```text
ntu_day_10_lio_filtered.bag
```

### C0/A0/A1 use

```text
ntu_day_10_livo_filtered.bag
```

The estimator should now consume a single canonical bag.

For this dataset benchmark path, runtime multi-bag merging should no longer be necessary.

Do NOT delete general multi-bag OfflineReader capability if other datasets require it.

This change is:

```text
MCD canonical benchmark input optimization
```

not:

```text
remove all multi-bag support from Super-LIVO
```

---

# 14. Baseline Semantic Parity Gate

Preprocessing is allowed only if estimator results remain semantically identical.

Run at least B0 before/after preprocessing:

```text
B0 raw multi-bag input
vs
B0 canonical filtered single bag
```

Compare:

```text
trajectory rows
trajectory start/end timestamps
trajectory MD5 if deterministic bitwise parity is expected
final pose
health counters
NaN/Inf
relevant message counts
```

Preferred acceptance:

```text
trajectory MD5 identical
```

If MD5 differs:

STOP.

Do not immediately accept numerical closeness.

First determine whether input ordering changed for equal timestamps or another input semantic changed.

---

# 15. LIVO Input Parity

For C0 visual-OFF camera-sync path, compare raw vs canonical input where practical.

Required:

```text
camera epoch count identical
LiDAR count identical
IMU count identical
camera header timestamp sequence identical
trajectory semantics identical
```

This proves the preprocessing step did not alter synchronization behavior.

---

# 16. Performance Measurement

Once correctness/parity passes, measure the benefit.

For B0 and at least one LIVO configuration, report:

```text
sensor duration
offline wall time
real-time factor

RTF = sensor_duration / wall_time

CPU usage
peak RSS
```

Also distinguish if available:

```text
bag read/decompression
dispatch
estimator compute
camera processing
EOF/finalization
```

Primary comparison:

```text
raw multi-bag BZ2 path
vs
canonical filtered LZ4 single-bag path
```

Do not claim estimator acceleration if the improvement comes from I/O/decompression.

Call it:

```text
offline benchmark pipeline acceleration
```

unless profiling proves estimator compute itself changed.

---

# 17. B0/C0/A0/A1 Benchmark Policy

After canonical input parity is proven:

Run serially:

```text
B0
C0
A0
A1
```

Do not launch them concurrently.

Use the same:

```text
dataset interval
GT
alignment
crop
calibration
timestamp association
```

as previously frozen.

No parameter sweep.

No tuning based on ATE.

---

# 18. Execution Hygiene

For all long commands:

1. Actual estimator program remains FOREGROUND.
2. One long build/test/experiment per logical shell invocation.
3. Do not wrap the long-running estimator with:
   ```text
   grep
   head
   tail
   tee
   ```
   as the execution pipeline.
4. Write output to a file if needed, then inspect that file in a separate short call.
5. Do not use:
   ```text
   nohup
   setsid
   &
   disown
   ```
   merely to avoid OpenCode spinner.
6. Completion means:
   ```text
   process exited
   + real RC known
   + expected artifacts exist
   ```
7. If OpenCode spins after OS child processes have already exited:
   - do NOT rerun;
   - inspect artifacts;
   - recover the tool turn.
8. For scripts that start a private `roscore`, install:
   ```bash
   trap cleanup EXIT INT TERM
   ```
   and clean up only the roscore started by that script.
9. Preserve the real estimator return code.
10. Before running a generated/edited shell script:
    ```bash
    bash -n script.sh
    ```
    and print the critical edited region.

---

# 19. Do Not Do

Do NOT:

- modify raw MCD bags;
- alter sensor `header.stamp`;
- use wall-clock estimator timestamps;
- keep infra1/infra2 just because they are present;
- merge every topic blindly;
- change calibration;
- change estimator algorithm;
- change B0/C0/A0/A1 semantics;
- tune visual parameters;
- introduce another handwritten Online/Offline estimator merge state machine without necessity;
- delete multi-bag support globally;
- rerun a failed/completed experiment merely because OpenCode UI is spinning;
- leave duplicate prompt `.md` files after canonical registration.

---

# 20. Documentation

Create/update documentation describing:

```text
source bags
selected topics
output canonical bags
record-time ordering semantics
header-time preservation
compression
validation counts
hash/parity evidence
benchmark speedup
```

The generated filtered bags themselves should normally remain dataset artifacts and not be committed to Git.

Commit:

```text
preprocessing scripts
tests
runner configuration
documentation
prompt registration
tracker updates
duplicate prompt cleanup
```

Do not commit multi-GB `.bag` files.

---

# 21. Required Final Report

Return one consolidated report with:

```text
Initial HEAD:
Final HEAD:

Architecture deviations:
NONE / exact deviation

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical prompt:
README updated:
tracker updated:

obsolete duplicate prompt files removed:
- ...
- ...

registration invariant:
PASS / FAIL

=== Source Audit ===
MCD source bags:
LiDAR bag/topic:
IMU bag/topic:
RGB bag/topic:
excluded topics:

=== Preprocessing Tool ===
path:
merge mechanism:
tie-break rule:
compression:

=== Synthetic Tests ===
T1:
T2:
T3:
T4:
T5:
T6:
T7:

=== Short MCD Validation ===
duration:
input relevant counts:
output counts:
timestamp monotonicity:
header timestamp parity:
EOF:
PASS / FAIL

=== Canonical Bags ===
LIO:
path:
size:
duration:
topic counts:

LIVO:
path:
size:
duration:
topic counts:

=== Raw vs Canonical B0 Parity ===
raw trajectory rows:
canonical rows:
raw MD5:
canonical MD5:
start/end timestamps:
final pose:
NaN/Inf:
PASS / FAIL

=== Camera/LIVO Input Parity ===
camera epochs:
camera header sequence:
LiDAR count:
IMU count:
PASS / FAIL

=== Performance ===
sensor duration:

raw multi-bag:
wall:
RTF:
CPU:
peak RSS:

canonical single-bag:
wall:
RTF:
CPU:
peak RSS:

speedup:

=== B0/C0/A0/A1 ===
B0:
C0:
A0:
A1:

=== Git Hygiene ===
git status:
duplicate prompt files remaining:
large bag files accidentally tracked:
NONE / exact issue

=== Commits ===
<hash> <subject>
...

Final recommendation:
```

---

# 22. STOP Conditions

STOP and report to Owner if any of the following occurs:

```text
source relevant message count != filtered count
header timestamps changed
merged record time non-monotonic
message replay
message loss
B0 raw vs canonical trajectory parity breaks
wrong MCD IMU source is ambiguous
output bag accidentally enters Git
canonical prompt registration leaves duplicate source .md
```

Do not paper over these failures with tolerance changes.

The goal of this round is:

```text
raw multi-bag MCD dataset
        ↓ one-time deterministic filtering/merge
canonical benchmark bags
        ↓
simple single-bag OfflineReader path
        ↓
repeatable faster B0/C0/A0/A1 benchmarking
```

while preserving estimator input semantics exactly.
