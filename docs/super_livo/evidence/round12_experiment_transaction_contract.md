# Super-LIVO Experiment Transaction Contract (Origin-frozen)

Origin (user) mandated unified transaction semantics for ALL formal bag
experiments (Super-LIVO, pristine Super-LIO, FAST-LIVO2, MCD, NTU, Oxford,
M2/M3). Applied from this point.

## Immutable per-run rules

- ONE RUN = ONE IMMUTABLE RUN_ID (timestamped; failed attempts never reused)
- ONE RUN = ONE ISOLATED ROS MASTER (free port, self-owned)
- ONE RUN = ONE SUPERVISOR-OWNED PROCESS GROUP
- NO broad pkill/killall/pgrep-f cleanup
- NO fixed sleep as readiness proof (bounded polling)
- NO reuse of failed output directory
- NO bag before config parity
- NO evaluation before output validation
- NO GREEN/AMBER/RED before EXPERIMENT_VALID=YES
- NO SIGKILL except last-resort recovery (SIGINT -> SIGTERM -> SIGKILL)
- NO manual mid-run repair
- NO second launch because UI spins/times out
- first failure log preserved
- canonical result promoted only after all gates pass

## State machine (CREATED -> CANONICAL_RESULT)

CREATED -> STATIC_PREFLIGHT_PASS -> ISOLATED_MASTER_READY -> NODES_READY ->
EFFECTIVE_CONFIG_CAPTURED -> CONFIG_PARITY_PASS -> DATA_PATH_PREFLIGHT_PASS ->
PLAYBACK_STARTED -> PLAYBACK_FINISHED -> ESTIMATOR_DRAINED -> OUTPUT_VALIDATED
-> EVALUATED -> CANONICAL_RESULT -> CLEANUP_VERIFIED

## Failure taxonomy (never GREEN/AMBER/RED)

INFRA_FAIL, CONFIG_FAIL, PROCESS_LIFECYCLE_FAIL, DATA_DELIVERY_FAIL,
OUTPUT_FAIL, EVALUATOR_FAIL.

## Reclassification of prior M3 evidence

- 72.53 m (dirty-state run): PROCESS_LIFECYCLE_FAIL
- 0-line mat_out run (duplicate republish shutdown): DATA_DELIVERY_FAIL /
  OUTPUT_FAIL
- 5.83 m (no effective snapshot): CONFIG_EVIDENCE_INCOMPLETE / diagnostic
  only (NOT a canonical FAST-LIVO2 result)
- Oxford 202/202 transport + 0.0397 m GREEN: transport contract evidence;
  standard image_transport confirmed viable.

rosbag play rc=0 != experiment succeeded.
