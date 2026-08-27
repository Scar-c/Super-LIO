# Round12 M3 transaction lifecycle/exclusivity corrective

## Incident and disposition

The User identified two simultaneous `rosbag play Outdoor01.bag` processes.
Both overlapping Outdoor01 experiments are `INVALID_CONTAMINATED`; none of
their trajectory or evaluator output is consumable.

Ownership attribution from the immutable run directories was:

| rosbag PID | RUN_ID | supervisor PID | prior state | corrective state |
|---:|---|---:|---|---|
| 160188 | `outdoor01/20260827T161453Z_56740a53` | 159790 | `FAILED/OUTPUT_FAIL` without cleanup evidence | `CANCELLED/USER_CANCELLED`, `cleanup_verified=true`, `experiment_valid=false` |
| 162348 | `outdoor01/20260827T161702Z_d8d41d0f` | 161938 | `PLAYBACK_STARTED`; old implementation failed to persist bag PID | `CANCELLED/USER_CANCELLED`, `cleanup_verified=true`, `experiment_valid=false` |

At corrective takeover the two supervisors and their recorded process trees
were already absent, so there was no live authoritative supervisor to signal.
The original JSON files are retained as `state.pre_corrective.json`; controlled
stale-owner recovery verified the recorded processes absent before closing the
transactions. No name-based `pkill` or global process cleanup was used.

## Root cause

`TRANSACTION_LIFECYCLE_EXCLUSIVITY_BUG`: the watcher could be interrupted while
the `setsid`/detached supervisor continued. A later run had no active/stale-run
gate. Both could write the shared FAST-LIVO2 `Log/mat_out.txt`. The attempted
corrective implementation also acquired `flock` and immediately unlocked it,
which did not provide transaction exclusivity.

## Corrected contract

- adapter-wide `flock` is held from admission until terminal cleanup;
- metadata records run ID, supervisor PID/start token, start time, bag and
  shared trajectory path;
- active or cleanup-unverified transaction refuses admission;
- conflicting M3DGR rosbag, FAST-LIVO2 roslaunch or mapping process refuses
  admission;
- PID, PGID and Linux process-start token are recorded separately;
- every child receives a transaction token;
- cleanup signals only a group whose leader identity and every live member's
  transaction token match;
- watcher reads the authoritative supervisor PID/token from `state.json`,
  requests cancellation, sends TERM only on identity match, and waits for a
  terminal state plus `cleanup_verified=true`;
- terminal experiment state and cleanup state are separate fields.

Canonical implementation:

- `scripts/super_livo/experiments/run_m3_transaction.sh`
- `scripts/super_livo/experiments/wait_for_m3_run.py`

## TDD evidence

The first unit run failed specifically because the lock was released early.
After correction, unit and fake-ROS end-to-end tests pass. The end-to-end suite
drives the public supervisor/watcher CLIs and covers SUCCESS cleanup, algorithm
failure cleanup, watcher SIGINT cancellation, supervisor SIGTERM, held-lock
and duplicate refusal, stale transaction refusal, PID-reuse/token mismatch,
owned process-tree absence, and preservation of terminal state alongside
cleanup state.

This is a supervisory gap found and required closed by the User on the DS
branch. It is not an estimator accuracy result.
