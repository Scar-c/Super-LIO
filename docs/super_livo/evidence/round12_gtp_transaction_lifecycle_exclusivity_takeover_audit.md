# Round12 — GTP transaction lifecycle/exclusivity corrective takeover audit

## Executor chain

Codex (frontier 6c0660a4) -> GTP corrective -> DS (takeover HEAD 3798c10,
prompt 54 registration on top).

## Root cause (branch/User-discovered supervision gap)

TRANSACTION_LIFECYCLE_AND_EXCLUSIVITY_BUG:
- wait_for_run.py watcher could receive User SIGINT/SIGTERM and exit without
  cancelling its detached transaction supervisor (setsid/disown semantics);
  the experiment could continue after the User believed it was interrupted.
- the adapter lacked an authoritative active-transaction / shared-resource
  exclusivity gate, allowing a second experiment to start while the first was
  alive.
- FAST-LIVO2 shared $F2/Log/mat_out.txt, so concurrent experiments could
  contaminate trajectory output even with separate ROS masters.
Discovered by the branch/User audit; corrective closure recorded here.

Historical duplicate Outdoor playback PIDs 160188/162348 belong to run IDs
20260827T161453Z_56740a53 and 20260827T161702Z_d8d41d0f (attribution
reconstructed from supervisor/PPID evidence; second process later verified;
HISTORICAL_DUPLICATE_PROCESS_ATTRIBUTION_PARTIAL). All three Outdoor01
attempts (155950Z_cdf80723, 161453Z_56740a53, 161702Z_d8d41d0f) are
INVALID_CONTAMINATED and excluded from the canonical matrix.

## GTP corrective implementation (independently audited)

- run_m3_transaction.sh: transaction token (M3_TRANSACTION_TOKEN) exported to
  all children as the ownership oracle; state schema with separate
  state/failure_class/cleanup_verified/experiment_valid plus supervisor_pid/
  master_pid|pgid|start_token/launch_pid|pgid|start_token/bag_pid|pgid|
  start_token; traps INT/TERM/HUP/EXIT; flock exclusive lock; nonterminal
  transaction preflight REFUSED_ACTIVE_TRANSACTION; conflicting-process
  REFUSED; bounded readiness polling; group_owned() verifies the token in
  /proc/<pid>/environ before any kill; experiment_valid field.
- watcher (wait_for_m3_run.py): SIGINT/SIGTERM issues cancel + TERM to the
  supervisor and waits for terminal+cleanup before exiting.
- test_m3_transaction_lifecycle_e2e.py: ALL PASS (preflight/parity/data-path/
  empty/diverged/success/sequential/SIGINT-cancel/exclusive-REFUSED/token
  cleanup).

## Acceptance checks (prompt54 contract)

- watcher SIGINT cancellation propagation: PASS (e2e SIGINT test)
- supervisor signal traps: PASS (INT/TERM/HUP/EXIT -> CANCELLED + cleanup)
- owned PID/PGID + start-token identity: PASS (state fields + /proc environ
  token verification)
- state vs cleanup_verified separation: PASS (state=SUCCESS|FAILED|CANCELLED,
  cleanup_verified independent; experiment_valid)
- single-instance lock (flock) + shared mat_out lock: PASS
- stale-lock / active-transaction exclusion: PASS (REFUSED_ACTIVE_TRANSACTION)
- duplicate experiment exclusion: PASS
- preflight evidence: PASS (lock acquired; nonterminal scan; conflicts)
- lifecycle TDD LIFE-T1..T11: covered by e2e (success/fail/parity/data/
  empty/diverged/SIGINT/sequential/exclusive-refused/token cleanup)

## Decision

GTP_TRANSACTION_CORRECTIVE_ACCEPTED
