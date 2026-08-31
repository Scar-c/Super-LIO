#!/usr/bin/env bash
# P0 baseline runner for Super-LIO (prob-lio branch).
#
# Runs the unmodified production path (super_lio_node + NTU.launch equivalent:
# rosparam load <config> then the node) against a caller-selected rosbag, with
# bounded, observable commands. Trajectory topics are recorded to a small bag.
#
# Usage:
#   tools/prob_lio/run_baseline.sh \
#     --bag <path-to.bag> \
#     [--offline]                   (use super_lio_offline_node instead of rosbag play)
#     [--config <path-to.yaml>]     (default: <ws>/src/super_lio/config/NTU.yaml)
#     [--out <results-dir>]         (default: results/prob_lio)
#     [--duration <seconds>]        (default: whole bag)
#     [--rate <playback-rate>]      (default: 1.0, online mode only)
#     [--set <key=value>]           (rosparam override, repeatable; e.g.
#                                    --set /lio/prob_lio/point_cov_enable=true)
#     [--canonical]                 (authoritative closure mode: REFUSES a
#                                    dirty worktree, per the clean-source
#                                    project rule; diagnostic runs may stay
#                                    dirty without this flag)
#     [--algorithm-commit <sha>]    (focused algorithm implementation commit
#                                    under test; recorded in meta.txt)
#     [--play-topics <t1,t2>]       (default: /os1_cloud_node1/points,/imu/imu)
#     [--record-topics <t1,t2>]     (default: /lio/odom,/lio/path)
#
# Exit code: 0 only if bag playback returned 0 and the estimator exited
# cleanly (rc 0) and no fatal-error marker appears in the node log.
# Last stdout line is always: __P0_RUN_DONE_RC=<rc>

set -u

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# This checkout is nested inside the catkin workspace used to build it. Keep
# Git/artifact paths rooted at the checkout, but source the workspace that
# owns the current devel product. A standalone checkout still uses itself.
CATKIN_WS="${PROB_LIO_CATKIN_WS:-$REPO_ROOT}"
if [ -z "${PROB_LIO_CATKIN_WS:-}" ] && [ -f "$REPO_ROOT/../../devel/setup.bash" ] \
    && [ -d "$REPO_ROOT/../../build" ]; then
  CATKIN_WS="$(cd "$REPO_ROOT/../.." && pwd)"
fi
source /opt/ros/noetic/setup.bash
[ -f "$CATKIN_WS/devel/setup.bash" ] && source "$CATKIN_WS/devel/setup.bash"

BAG=""
CONFIG="$REPO_ROOT/src/super_lio/config/NTU.yaml"
OUT="${PROB_LIO_RUNTIME_ROOT:-$(cd "$REPO_ROOT/../.." && pwd)/results/prob_lio_runtime}"
DURATION=""
RATE="1.0"
OFFLINE=0
CANONICAL=0
ALGORITHM_COMMIT=""
RUN_ID=""
PARAM_OVERRIDES=()
PLAY_TOPICS="/os1_cloud_node1/points,/imu/imu"
RECORD_TOPICS="/lio/odom,/lio/path"

while [ $# -gt 0 ]; do
  case "$1" in
    --bag) BAG="$2"; shift 2 ;;
    --offline) OFFLINE=1; shift ;;
    --config) CONFIG="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    --duration) DURATION="$2"; shift 2 ;;
    --rate) RATE="$2"; shift 2 ;;
    --set) PARAM_OVERRIDES+=("$2"); shift 2 ;;
    --canonical) CANONICAL=1; shift ;;
    --algorithm-commit) ALGORITHM_COMMIT="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --play-topics) PLAY_TOPICS="$2"; shift 2 ;;
    --record-topics) RECORD_TOPICS="$2"; shift 2 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

if [ -z "$BAG" ] || [ ! -f "$BAG" ]; then
  echo "ERR: --bag must point to an existing rosbag" >&2
  exit 2
fi
if [ ! -f "$CONFIG" ]; then
  echo "ERR: --config must point to an existing yaml: $CONFIG" >&2
  exit 2
fi
if [ -n "$RUN_ID" ] && ! [[ "$RUN_ID" =~ ^[A-Za-z0-9_.-]+$ ]]; then
  echo "ERR: --run-id may contain only letters, numbers, dot, underscore, hyphen" >&2
  exit 2
fi

# Clean-source project rule (prompt6 §1): canonical closure runs require a
# clean committed worktree; diagnostic runs may be dirty when explicitly
# requested (no --canonical).
GIT_STATUS_SHORT="$(git -C "$REPO_ROOT" status --short | tr '\n' ';')"
if [ "$CANONICAL" -eq 1 ] && [ -n "$GIT_STATUS_SHORT" ]; then
  echo "ERR: --canonical requires a clean worktree; git status: $GIT_STATUS_SHORT" >&2
  exit 3
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
if [ -n "$RUN_ID" ]; then
  RUN_DIR="$OUT/$RUN_ID"
else
  RUN_DIR="$OUT/run_${STAMP}"
fi
mkdir -p "$OUT"
if [ -e "$RUN_DIR" ]; then
  echo "ERR: refusing to overwrite existing run directory: $RUN_DIR" >&2
  exit 2
fi
mkdir "$RUN_DIR"
# The managed execution environment may have a read-only ~/.ros. Keep ROS
# master/node logs inside the immutable run artifact instead of relying on a
# host-global ROS log directory.
ROS_LOG_DIR="$RUN_DIR/ros_log"
mkdir "$ROS_LOG_DIR"
export ROS_LOG_DIR
NODE_LOG="$RUN_DIR/node.log"
CORE_LOG="$RUN_DIR/roscore.log"
PLAY_LOG="$RUN_DIR/play.log"
META_LOG="$RUN_DIR/meta.txt"
RESULT_BAG="$RUN_DIR/result.bag"

GIT_DIRTY="no"
GIT_STATUS_SHORT="$(git -C "$REPO_ROOT" status --short | tr '\n' ';')"
if [ -n "$GIT_STATUS_SHORT" ]; then GIT_DIRTY="yes"; fi
GIT_DIFF_SHA256=""
if [ "$GIT_DIRTY" = "yes" ]; then
  GIT_DIFF_SHA256="$(git -C "$REPO_ROOT" diff | sha256sum | cut -d' ' -f1)"
fi
{
  echo "repository_root: $REPO_ROOT"
  echo "catkin_workspace: $CATKIN_WS"
  echo "git_branch: $(git -C "$REPO_ROOT" branch --show-current)"
  echo "run_git_head: $(git -C "$REPO_ROOT" rev-parse HEAD)"
  echo "run_git_dirty: $GIT_DIRTY"
  echo "run_git_status_short: $GIT_STATUS_SHORT"
  echo "production_tree_oid: $(git -C "$REPO_ROOT" rev-parse HEAD:src/super_lio)"
  echo "production_code_oid: $(python3 "$REPO_ROOT/tools/prob_lio/production_identity.py" --repo-root "$REPO_ROOT" | sed -n 's/^production_code_oid: //p')"
  echo "dataset_config_sha256: $(sha256sum "$CONFIG" | cut -d' ' -f1)"
  echo "algorithm_commit: ${ALGORITHM_COMMIT:-<unset>}"
  echo "git_diff_sha256: ${GIT_DIFF_SHA256:-clean}"
  echo "config: $CONFIG"
  echo "bag: $BAG"
  echo "rate: $RATE"
  echo "duration: ${DURATION:-whole-bag}"
  echo "record_topics: $RECORD_TOPICS"
  echo "start: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "host: $(hostname)  cores: $(nproc)"
} | tee "$META_LOG"

CORE_PID=""
NODE_PID=""
RECORD_PID=""
cleanup() {
  for pat in "super_lio_node" "super_lio_offline_node" "rosbag record" "rosbag play"; do
    pkill -f "$pat" 2>/dev/null
  done
  [ -n "$CORE_PID" ] && kill "$CORE_PID" 2>/dev/null
}
trap cleanup EXIT

# Isolate this run: fresh master on a dedicated port; never reuse a stale one.
pkill -f "rosmaster -p 11311" 2>/dev/null
sleep 1
MASTER_PORT=$((11311 + RANDOM % 200))
export ROS_MASTER_URI="http://localhost:${MASTER_PORT}"
echo "ROS_MASTER_URI=$ROS_MASTER_URI" | tee -a "$META_LOG"
rosmaster -p "$MASTER_PORT" >"$CORE_LOG" 2>&1 &
CORE_PID=$!
sleep 3
if ! timeout 5 rosnode list >/dev/null 2>&1; then
  echo "ERR: rosmaster did not come up (see $CORE_LOG)" >&2
  exit 2
fi

rosparam delete /lio/offline 2>/dev/null || true
rosparam load "$CONFIG"
rosparam set /lio/offline/bag "$BAG"
[ -n "$DURATION" ] && rosparam set /lio/offline/duration "$DURATION"

for kv in "${PARAM_OVERRIDES[@]:-}"; do
  [ -n "$kv" ] || continue
  rosparam set "${kv%%=*}" "${kv#*=}"
  echo "param override: $kv" | tee -a "$META_LOG"
done

if [ "$OFFLINE" -eq 1 ]; then
  rosparam set /lio/offline/out_dir "$RUN_DIR"
fi

# Capture the actual ROS parameter tree after config loading and all variant
# overrides, before the estimator starts. The orchestration layer hashes
# this file and uses it for exact variant-isolation checks.
EFFECTIVE_CONFIG="$RUN_DIR/effective_rosparams.yaml"
if ! rosparam dump "$EFFECTIVE_CONFIG" /lio; then
  echo "ERR: unable to dump effective /lio parameters" >&2
  exit 2
fi
echo "effective_config: $EFFECTIVE_CONFIG" | tee -a "$META_LOG"
echo "effective_config_sha256: $(sha256sum "$EFFECTIVE_CONFIG" | cut -d' ' -f1)" | tee -a "$META_LOG"

if [ "$OFFLINE" -eq 1 ]; then
  rosrun super_lio super_lio_offline_node __name:=lio_offline >"$NODE_LOG" 2>&1 &
else
  rosrun super_lio super_lio_node __name:=super_lio_node >"$NODE_LOG" 2>&1 &
fi
NODE_PID=$!

echo "waiting for /lio/odom publisher ..." | tee -a "$META_LOG"
for i in $(seq 1 60); do
  rostopic list 2>/dev/null | grep -qx "/lio/odom" && break
  sleep 1
done

if [ -n "$RECORD_TOPICS" ] && [ "$OFFLINE" -eq 0 ]; then
  IFS=',' read -r -a RT <<< "$RECORD_TOPICS"
  # Online mode: rosbag record for trajectory capture. (Offline mode captures
  # the trajectory in-process via /lio/offline/out_dir -> trajectory.tum.)
  rosbag record -O "$RESULT_BAG" "${RT[@]}" >"$RUN_DIR/record.log" 2>&1 &
  RECORD_PID=$!
fi

IFS=',' read -r -a PT <<< "$PLAY_TOPICS"
set -o pipefail
if [ "$OFFLINE" -eq 1 ]; then
  # Offline node reads the bag itself (no playback pacing); wait for it to
  # finish, then collect its return code.
  wait "$NODE_PID" 2>/dev/null
  NODE_RC=$?
  PLAY_RC=$NODE_RC
else
  # NOTE: bag path must precede --topics (rosbag's --topics callback consumes
  # every following non-dash arg, which would swallow the bag path).
  rosbag play "$BAG" --rate "$RATE" ${DURATION:+--duration "$DURATION"} \
    --topics "${PT[@]}" 2>&1 | tee "$PLAY_LOG"
  PLAY_RC=${PIPESTATUS[0]}

  sleep 2
  kill -INT "$NODE_PID" 2>/dev/null
  wait "$NODE_PID" 2>/dev/null
  NODE_RC=$?
fi

sleep 2
RECORD_RC=0
if [ -n "$RECORD_PID" ]; then
  kill -INT "$RECORD_PID" 2>/dev/null
  wait "$RECORD_PID" 2>/dev/null
  RECORD_RC=$?
fi

echo "play_rc=$PLAY_RC node_rc=$NODE_RC record_rc=$RECORD_RC" | tee -a "$META_LOG"

FATAL_MARKERS="FATAL|nan.*NaN|Segmentation fault|Aborted|Assertion"
grep -Eq "$FATAL_MARKERS" "$NODE_LOG"
GREP_RC=$?
echo "fatal_marker_grep_rc=$GREP_RC (1 = none found)" | tee -a "$META_LOG"

if [ "$PLAY_RC" -eq 0 ] && [ "$NODE_RC" -eq 0 ] && [ "$GREP_RC" -eq 1 ]; then
  RC=0
else
  RC=1
fi

echo "node_log: $NODE_LOG" | tee -a "$META_LOG"
echo "result_bag: $RESULT_BAG"
echo "__P0_RUN_DONE_RC=$RC"
exit "$RC"
