# Round 13 — DS Takeover / GTP Work Reconciliation + Canonical Remote Sync ONLY

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

This is a **handoff reconciliation + remote audit delivery** task only.

You are NOT authorized to continue:

```text
camera-event Visual placement
camera payload lifetime corrective
D_VISUAL_APPLY
D_VISUAL_SHADOW rerun
any experiment
any algorithm change
any runner redesign
any parameter change
```

The Owner requires:

> First understand and reconcile everything GTP completed after the earlier DS interruption, then make the canonical remote fork contain exactly the accepted local Git history needed for independent Origin review.

After remote/local synchronization is proven:

```text
STOP
```

Do not continue functional work.

---

# 1. Known current frontier

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Latest reported local HEAD:

```text
45ffc091c88e2e4a3d82c24d02a7289bee6b8684
```

This came from:

```text
Round 13 — Minimal D Visual-Apply Connectivity Bridge / eee_01
```

Previous DS Final HEAD:

```text
99c3c88996d0191bb41b08251d2fdb0977f9ac80
```

Previous GTP canonical Shadow closure HEAD:

```text
19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0
```

Earlier GTP takeover/recovery HEAD:

```text
2bde100b15cd79fb14d3b3ac4cacfd61cb6aa526
```

Earlier semantic-recovery HEAD:

```text
711a6674d2e22363b68002eb12cde83bb614fc88
```

Historical GTP own frontier before later takeover:

```text
3798c10785477ab36297549e4af5753019dcdf98
```

These are expected landmarks only.

Git history is authoritative.

---

# 2. Important interrupted DS context

Before GTP takeover, the User had assigned DS:

```text
/home/lc/super_livo/src/Super-LIO/prompts/
Round 13 Corrective — DS_ Restore Canonical Measurement-Active D0 - eee_01 Only.md
```

That DS task was interrupted by the User.

After the interruption, work continued through GTP and later DS takeovers.

Therefore:

> DO NOT resume that interrupted prompt.

Your first responsibility is to reconcile the full post-interruption shared history.

---

# 3. Mandatory startup consensus

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -80
git diff --check
git remote -v
git branch -vv
```

Record:

```text
EXPECTED_HEAD =
45ffc091c88e2e4a3d82c24d02a7289bee6b8684

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH =
YES/NO

WORKTREE =
CLEAN/DIRTY

CURRENT_BRANCH =
...

UPSTREAM_TRACKING_BRANCH =
...
```

If HEAD differs:

do not reset.

Explain why and reconcile mechanically.

---

# 4. No functional work before takeover audit

Before touching any remote, reconstruct:

```text
DS interruption
→ GTP takeover
→ semantic normalization
→ Shadow recovery
→ Shadow evidence closure
→ DS Apply reachability audit
→ DS minimal bridge audit
→ current frontier
```

Use:

```bash
git log --reverse --format='%H %an <%ae> %ad %s' \
  --date=iso <appropriate-old-frontier>..HEAD
```

and inspect relevant diffs.

Do not rely solely on Final Reports.

---

# 5. Mandatory Prompt reconciliation

Locate and inspect the actual registered prompts corresponding to the post-interruption history.

At minimum reconcile:

```text
Prompt58
Prompt59
Prompt60
Prompt61
Prompt62
```

Also locate the interrupted DS prompt explicitly named by the User.

For each report:

```text
prompt path
executor
starting HEAD
ending HEAD
status
what it authorized
what it actually changed
whether superseded
whether still semantically authoritative
```

Create:

```text
docs/super_livo/evidence/
round13_ds_takeover_gtp_prompt_commit_reconciliation.md
```

---

# 6. Mandatory commit→executor→prompt mapping

Create a mechanically grounded table:

| Commit SHA | Author/executor | Prompt | Type | What changed | Current status |
|---|---|---|---|---|---|

Allowed types:

```text
PROMPT
DOCUMENTATION
TEST
RUNNER
TRANSACTION
INSTRUMENTATION
PRODUCTION
REVERT
EXPERIMENT_EVIDENCE
LEDGER
TRACKER
```

Current status:

```text
ACCEPTED
SUPERSEDED
REVERTED
QUARANTINED
HISTORICAL_EVIDENCE
CURRENT
```

Particularly identify which commits:

```text
introduced normalized semantic profiles
restored producer gates
added Shadow measurement instrumentation
forward-reverted Round13 production changes
audited Apply reachability
audited minimal bridge impossibility
```

---

# 7. DS must explicitly demonstrate understanding of GTP's work

Before remote sync, produce a concise technical handoff summary answering all of these.

## 7.1 GTP semantic normalization

Explain:

```text
legacy labels
vs
normalized semantic profiles
```

including:

```text
D_SCHEDULER_BASE
D_VISUAL_SHADOW
D_VISUAL_APPLY
```

---

## 7.2 Runner regression

Explain the confirmed:

```text
4543347 runner producer-gate regression
```

and the role of the missing historical producer gates.

---

## 7.3 Round13 production reverts

Explain why:

```text
33c1b3d
7d9be50
ce3d1a9
```

were forward-reverted rather than retained.

State their current disposition.

---

## 7.4 Prompt60 Shadow closure

Explain what Prompt60 actually proved:

```text
producer active
landmarks active
queries active
residual active
finite nonzero H/b
Visual state apply OFF
```

and what it did NOT prove.

---

## 7.5 Prompt61/62 finding

Explain the current event-placement finding:

```text
normalized D = imu_fullscan

camera payload is popped in the IMU_ONLY path

statePropagateOnly does not execute Visual measurement

Prompt60 H/b is generated later in the full-LiDAR Observe
convergence callback

legacy Apply block is PARTIAL-only
```

Therefore current unresolved problem is larger than a one-line Apply gate.

---

# 8. Required semantic statement

DS must explicitly acknowledge:

```text
D_VISUAL_SHADOW measurement existence:
PROVEN

D_VISUAL_SHADOW camera-epoch measurement placement:
NOT ESTABLISHED by Prompt60

D_VISUAL_APPLY production connectivity:
NOT ESTABLISHED

camera-event corrective:
NOT AUTHORIZED IN THIS TASK
```

If DS disagrees, provide source/commit evidence.

Do not modify code to resolve the disagreement.

---

# 9. Remote-sync purpose

The Owner's established workflow requires completed Super-LIVO work to be available on the canonical remote fork so Origin can independently audit:

```text
production code
tests
runners
transaction infrastructure
prompts
documentation
evidence
ledger/tracker
```

Local commits alone are not sufficient audit delivery.

This task restores that contract.

---

# 10. Remote policy

Determine the actual repository remotes from:

```bash
git remote -v
```

The intended destination is the User's own canonical fork/branch, expected conceptually to be:

```text
origin/super-livo
```

but DO NOT blindly assume the remote name/branch.

Mechanically confirm:

```text
which remote is the User's fork
which branch is the canonical Super-LIVO branch
which remote is upstream/reference-only
```

Forbidden:

```text
push to upstream/reference repositories

force push

--force-with-lease

rebase published history

history rewriting

branch deletion
```

---

# 11. Fetch before push

Before modifying remote state:

```bash
git fetch --all --prune
```

Then record:

```text
LOCAL_HEAD
CANONICAL_REMOTE_HEAD
merge-base
ahead count
behind count
divergence
```

Use appropriate commands such as:

```bash
git rev-parse HEAD
git rev-parse <remote>/<branch>
git merge-base HEAD <remote>/<branch>
git rev-list --left-right --count <remote>/<branch>...HEAD
git log --oneline <remote>/<branch>..HEAD
git log --oneline HEAD..<remote>/<branch>
```

Persist the full list of local commits missing from remote.

---

# 12. Remote sync decision

## Case A — remote is a clean ancestor of local

If:

```text
CANONICAL_REMOTE_HEAD
```

is an ancestor of:

```text
LOCAL_HEAD
```

and local canonical history is clean:

authorized action:

> normal fast-forward push of the canonical local branch to the User's canonical fork.

No force.

---

## Case B — local and remote already identical

Do not push unnecessarily.

Verify equality and proceed to final audit report.

---

## Case C — remote contains commits absent locally / histories diverge

DO NOT:

```text
merge
rebase
force push
```

Classify:

```text
REMOTE_DIVERGENCE_STOP
```

and STOP_FOR_OWNER.

Provide both commit lists.

---

# 13. Scope of what must reach remote

The remote canonical branch must contain all canonical local commits through the accepted current frontier needed to reproduce/audit:

```text
Round11AA+
Round11AB
Round12
GTP transaction corrective
Round13 semantic normalization
producer-gate restoration
normalized profiles
Shadow evidence instrumentation
production forward reverts
Prompt61/62 audits
current documentation/tests
```

Do not cherry-pick selected recent commits while leaving required ancestors local-only.

Push the canonical DAG normally if safe.

---

# 14. Do not push transient runtime artifacts

Before push inspect:

```bash
git status
git ls-files
```

Do not newly commit:

```text
large rosbag files
temporary logs
/tmp artifacts
build products
core dumps
runtime lock files
ephemeral transaction state
unbounded raw diagnostic dumps
```

Only already-intended tracked project artifacts/evidence belong in the repository.

If large unintended tracked files are discovered:

STOP before rewriting history.

Report them.

---

# 15. No "cleanup" of historical commits

Do not remove old/superseded Prompt files merely because they are superseded.

They are provenance.

Do not squash:

```text
DS commits
GTP commits
corrective commits
revert commits
```

Origin needs the actual history for audit.

---

# 16. Push authorization

For THIS task only, the Owner explicitly authorizes:

> normal non-force push of the verified canonical Super-LIVO branch to the User's canonical fork so remote and local canonical history match.

This supersedes the recent per-prompt:

```text
PUSH = NOT AUTHORIZED
```

for this audit-delivery action.

It does NOT authorize:

```text
upstream push
force push
tag deletion
history rewrite
unrelated branches
```

---

# 17. Required push execution evidence

If Case A applies, perform the normal push.

Then immediately:

```bash
git fetch <canonical-remote>
git rev-parse HEAD
git rev-parse <canonical-remote>/<canonical-branch>
git rev-list --left-right --count \
  <canonical-remote>/<canonical-branch>...HEAD
```

Required:

```text
LOCAL_HEAD =
REMOTE_HEAD =
<same full 40-char SHA>

AHEAD =
0

BEHIND =
0
```

---

# 18. Remote verification must be independent of push stdout

Do NOT conclude success only because:

```text
git push
```

returned 0.

After fetch, resolve the remote-tracking branch again and prove exact SHA equality.

---

# 19. Verify current expected remote endpoint

Final expected local canonical HEAD, assuming no audit-only documentation commit after startup:

```text
45ffc091c88e2e4a3d82c24d02a7289bee6b8684
```

However this prompt itself requires provenance documentation/README/tracker updates.

Therefore Final HEAD may legitimately advance.

The rule is:

```text
FINAL_LOCAL_HEAD
=
FINAL_CANONICAL_REMOTE_HEAD
```

not that Final HEAD must remain `45ffc091`.

Every new commit in this task must be:

```text
prompt/provenance/remote-audit documentation only
```

No production change.

---

# 20. Production freeze

During this task:

```text
Production algorithm changes = NONE
Runner changes = NONE
Transaction changes = NONE
Instrumentation changes = NONE
Experiment config changes = NONE
```

No exceptions.

If you discover a functional defect:

document it only.

Do not fix it.

---

# 21. No experiment execution

Forbidden:

```text
rosbag playback

eee Shadow rerun

eee Apply run

nya

sbs

Oxford

MCD

M3DGR

benchmark rerun
```

No evaluator work is required either.

This is a provenance/remote delivery task.

---

# 22. No build unless needed for provenance verification

Do not perform broad rebuild/testing.

Existing commits are being delivered for audit, not revalidated.

Permitted lightweight checks:

```text
git diff --check
repository status
prompt existence
tracked-file verification
commit ancestry
remote ancestry
```

No expensive experimentation.

---

# 23. Required Origin audit bundle

Create:

```text
docs/super_livo/evidence/
round13_remote_origin_audit_bundle.md
```

It must identify the specific files Origin should inspect first.

At minimum:

```text
production estimator files changed since Round11AA
canonical runner/profile implementation
transaction supervisor
semantic_profiles.py / equivalent
measurement instrumentation
fail-closed validator
Prompt59
Prompt60
Prompt61
Prompt62
Round13 evidence docs
canonical benchmark ledger
active trackers
```

For each:

```text
path
relevant commit(s)
why Origin should review it
current accepted/quarantined status
```

---

# 24. Required commit ranges for Origin

Record exact ranges such as:

```text
<remote-old-head>..FINAL_HEAD
```

and important subranges around:

```text
Round11AA → Round12
Round12 → Round13
GTP takeover
Prompt60
Prompt61
Prompt62
```

Use actual discovered SHAs, not guessed ones.

---

# 25. Required unresolved-issues section

The audit bundle must explicitly say that no functional continuation is authorized yet.

Current unresolved architectural issue to preserve for later review:

```text
Current D camera epoch performs IMU propagation/accounting,
but camera-event Visual measurement placement is not established.

Prompt60 proves real Visual measurement exists later in the
full-LiDAR Observe convergence callback.

Prompt61/62 prove existing Visual Apply is unreachable from
normalized D and a simple one-line bridge cannot satisfy the
same-camera-epoch contract.
```

Do not attempt to solve it here.

---

# 26. Prompt registration

Register this exact task as the next canonical Round13 prompt.

Suggested name:

```text
63_round13_ds_takeover_gtp_reconciliation_and_remote_sync.md
```

Use the project's actual prompt directory convention.

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Mark:

```text
functional work:
PAUSED FOR ORIGIN REMOTE AUDIT

next algorithm action:
NOT AUTHORIZED
```

---

# 27. Commit policy

This task may create commits ONLY for:

```text
Prompt63 registration

DS/GTP commit-prompt reconciliation

remote audit bundle

README/tracker provenance state
```

No production/code/test/runner changes.

Use clear commits.

Do not amend GTP/DS historical commits.

---

# 28. Final STOP contract

Once:

```text
GTP work reconciled

Prompt/commit history mapped

canonical local branch audited

canonical remote safely synchronized

remote SHA == local SHA

Origin audit bundle committed and pushed
```

STOP.

Do NOT begin camera-event work.

Do NOT propose implementation patches inside the repository beyond a textual next-step recommendation.

The User/Origin will independently review remote contents first.

---

# 29. Final report format

Use exactly:

```text
Round 13 — DS Takeover / GTP Reconciliation + Canonical Remote Sync

Initial HEAD:
Final HEAD:

Production changes:
NONE

Runner changes:
NONE

Instrumentation changes:
NONE

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected initial HEAD:
45ffc091c88e2e4a3d82c24d02a7289bee6b8684

actual initial HEAD:

branch:
worktree:

frontier verified:
YES/NO

=== Interrupted DS Work ===
previous interrupted prompt:
<exact path>

interrupted work recovered:
YES/NO

status:
SUPERSEDED / HISTORICAL / other

=== GTP Takeover Reconciliation ===
GTP relevant commits:
...

GTP prompts:
...

GTP normalized semantic-profile work:
...

GTP producer-gate recovery:
...

GTP Round13 forward reverts:
...

GTP Shadow evidence closure:
...

DS understanding verified:
YES/NO

=== Post-GTP DS Work ===
Prompt61:
HEAD:
classification:
key finding:

Prompt62:
HEAD:
classification:
key finding:

=== Prompt / Commit Mapping ===
<full table or document path>

audit document:
...

=== Current Semantic State ===
D_SCHEDULER_BASE:
...

D_VISUAL_SHADOW:
measurement existence:
PROVEN

camera-epoch placement:
NOT ESTABLISHED

D_VISUAL_APPLY:
production connectivity:
NOT ESTABLISHED

camera-event corrective authorized:
NO

=== Remote Configuration ===
canonical fork remote:
...

canonical branch:
...

upstream/reference remotes:
...

push destination verified as User fork:
YES/NO

=== Pre-Sync Remote Audit ===
local HEAD:
...

remote HEAD:
...

merge-base:
...

local ahead:
...

local behind:
...

relationship:
REMOTE_ANCESTOR / IDENTICAL / DIVERGED / other

commits missing on remote:
...

remote-only commits:
...

=== Remote Sync Action ===
normal push authorized:
YES

force push used:
NO

upstream pushed:
NO

push performed:
YES/NO

push return code:

=== Post-Sync Verification ===
final local HEAD:
...

final remote HEAD:
...

local/remote SHA equal:
YES/NO

ahead:
0 / ...

behind:
0 / ...

remote fetch verification:
PASS/FAIL

=== Origin Audit Bundle ===
path:
...

production files listed:
YES/NO

runner/profile files listed:
YES/NO

transaction files listed:
YES/NO

tests/instrumentation listed:
YES/NO

prompt/evidence ranges listed:
YES/NO

unresolved camera-event issue recorded:
YES/NO

=== Prompt Registration ===
canonical:
...

README:
...

tracker:
...

functional work state:
PAUSED_FOR_ORIGIN_REMOTE_AUDIT

=== Git Safety ===
force push:
NO

rebase:
NO

history rewrite:
NO

upstream modification:
NO

branch deletion:
NO

=== WIP ===
present:
YES/NO

worktree clean:
YES/NO

=== Final Classification ===

Choose exactly one:

ROUND13_REMOTE_AUDIT_DELIVERY_READY

ROUND13_REMOTE_ALREADY_IN_SYNC

ROUND13_REMOTE_DIVERGENCE_STOP

ROUND13_REMOTE_SYNC_FAILED

ROUND13_TAKEOVER_PROVENANCE_UNRESOLVED

=== Next Step ===

STOP.

Do NOT continue camera-event/Visual work.

Await Origin independent review of the synchronized remote repository.
```

Full 40-character Final HEAD mandatory.

---

# 30. Permanent workflow correction

The current project workflow is:

```text
agent implements / experiments
        ↓
commit
        ↓
normal push to User canonical fork
        ↓
remote HEAD verified equal to local canonical HEAD
        ↓
Origin independently audits:
    production
    tests
    runner
    evidence
        ↓
only then authorize next functional work
```

Recent:

```text
Push: NO
```

deliveries did not satisfy the remote audit-delivery portion of this workflow.

This prompt exists to close that gap.

Do not proceed beyond it.