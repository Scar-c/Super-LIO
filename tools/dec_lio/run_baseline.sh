#!/usr/bin/env bash
# Dec-LIO runner for native ROS1 Super-LIO baselines.
set -Eeuo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CATKIN_WS="${DEC_LIO_CATKIN_WS:-$(cd "$REPO_ROOT/../.." && pwd)}"
RUNTIME_ROOT="${DEC_LIO_RUNTIME_ROOT:-$(cd "$REPO_ROOT/../.." && pwd)/runtime/prompt00r}"
source /opt/ros/noetic/setup.bash
[ -f "$CATKIN_WS/devel/setup.bash" ] && source "$CATKIN_WS/devel/setup.bash"

MODE="online"
SEQUENCE="bridge01"
BAG=""
CONFIG="$REPO_ROOT/src/super_lio/config/geode_alpha.yaml"
OUT="$RUNTIME_ROOT"
RUN_ID=""
GROUND_TRUTH_OVERRIDE=""
RATE="1.0"
DURATION=""
THREADS="$(nproc)"
D1_SHADOW="false"
D2_SHADOW="false"
CONSISTENCY_SHADOW="false"
AXIS_SHADOW="false"
D3_SOLVER_SHADOW="false"
PAIRED_ATTENUATION="false"
PAIRED_ATTENUATION_SHADOW="false"
PAIRED_MODE="0"
P14_SHADOW="false"
P15_ENABLED="false"
P15_FRAME=""
BLIND_OVERRIDE=""
FILTER_RATE_OVERRIDE=""
VOXEL_OVERRIDE=""
MAXRANGE_OVERRIDE=""
POINT_TIME_SCALE_OVERRIDE=""
GEODE_FINITE_THEN_STRIDE_OVERRIDE=""
PLAY_TOPICS="/velodyne_points,/imu/data"
RECORD_TOPICS="/lio/odom"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --mode) MODE="$2"; shift 2 ;;
    --offline) MODE="offline"; shift ;;
    --sequence) SEQUENCE="$2"; shift 2 ;;
    --bag) BAG="$2"; shift 2 ;;
    --config) CONFIG="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --ground-truth) GROUND_TRUTH_OVERRIDE="$2"; shift 2 ;;
    --rate) RATE="$2"; shift 2 ;;
    --duration) DURATION="$2"; shift 2 ;;
    --threads) THREADS="$2"; shift 2 ;;
    --d1-shadow) D1_SHADOW="true"; shift ;;
    --d2-shadow) D2_SHADOW="true"; shift ;;
    --consistency-shadow) CONSISTENCY_SHADOW="true"; shift ;;
    --axis-shadow) AXIS_SHADOW="true"; shift ;;
    --d3-solver-shadow) D3_SOLVER_SHADOW="true"; shift ;;
    --paired-attenuation) PAIRED_ATTENUATION="true"; PAIRED_MODE="1"; shift ;;
    --paired-attenuation-shadow) PAIRED_ATTENUATION_SHADOW="true"; PAIRED_MODE="1"; shift ;;
    --paired-mode)
      case "$2" in
        off|0) PAIRED_MODE="0" ;;
        p1|directional|1) PAIRED_MODE="1" ;;
        utrace|u-trace|trace|2) PAIRED_MODE="2" ;;
        ugamma|u-gamma|gamma|3) PAIRED_MODE="3" ;;
        *) echo "ERR: --paired-mode must be off, p1, utrace, or ugamma" >&2; exit 2 ;;
      esac
      shift 2 ;;
    --prompt14-shadow|--p14-shadow) P14_SHADOW="true"; shift ;;
    --prompt15-frame|--p15-frame)
      P15_ENABLED="true"; P15_FRAME="$2"; shift 2 ;;
    --blind) BLIND_OVERRIDE="$2"; shift 2 ;;
    --filter-rate) FILTER_RATE_OVERRIDE="$2"; shift 2 ;;
    --voxel-size) VOXEL_OVERRIDE="$2"; shift 2 ;;
    --maxrange) MAXRANGE_OVERRIDE="$2"; shift 2 ;;
    --point-time-scale) POINT_TIME_SCALE_OVERRIDE="$2"; shift 2 ;;
    --geode-finite-then-stride) GEODE_FINITE_THEN_STRIDE_OVERRIDE="$2"; shift 2 ;;
    --play-topics) PLAY_TOPICS="$2"; shift 2 ;;
    --record-topics) RECORD_TOPICS="$2"; shift 2 ;;
    *) echo "ERR: unknown argument: $1" >&2; exit 2 ;;
  esac
done

if [ "$P15_ENABLED" = "true" ]; then
  if [[ ! "$P15_FRAME" =~ ^[0-9]+$ ]]; then
    echo "ERR: --prompt15-frame must be a non-negative integer" >&2; exit 2
  fi
  # Prompt15 requires the Prompt14 source diagnostics for the same frame.
  P14_SHADOW="true"
fi

if [ "$PAIRED_MODE" != "0" ]; then PAIRED_ATTENUATION="true"; fi

if [[ "$MODE" != online && "$MODE" != offline ]]; then
  echo "ERR: --mode must be online or offline" >&2; exit 2
fi
if [ -z "$BAG" ] || [ ! -f "$BAG" ]; then
  echo "ERR: --bag must point to an existing rosbag" >&2; exit 2
fi
if [ ! -f "$CONFIG" ]; then
  echo "ERR: missing config: $CONFIG" >&2; exit 2
fi
if [ -n "$GROUND_TRUTH_OVERRIDE" ]; then
  GROUND_TRUTH="$GROUND_TRUTH_OVERRIDE"
elif [ "$SEQUENCE" = "tunneling_tunnel2" ]; then
  GROUND_TRUTH="$(dirname "$BAG")/Tunneling_tunnel2.txt"
elif [ "$SEQUENCE" = "tunneling_tunnel1_gamma" ]; then
  GROUND_TRUTH="$(dirname "$BAG")/Tunneling_tunnel1.txt"
elif [ "$SEQUENCE" = "tunneling_tunnel2_alpha" ] || [ "$SEQUENCE" = "tunneling_tunnel2_gamma" ]; then
  GROUND_TRUTH="$(dirname "$BAG")/Tunneling_tunnel2.txt"
elif [ "$SEQUENCE" = "fyllingsdalen_tunnel" ] || [ "$SEQUENCE" = "runehamar_tunnel_hornbill" ]; then
  GROUND_TRUTH="$(dirname "$BAG")/gt_odometry.tum"
else
  GROUND_TRUTH="$(dirname "$BAG")/$SEQUENCE.txt"
fi
python3 "$REPO_ROOT/tools/dec_lio/validate_input.py" \
  --sequence "$SEQUENCE" --bag "$BAG" --config "$CONFIG" --ground-truth "$GROUND_TRUTH" >/dev/null
if [ -z "$RUN_ID" ]; then RUN_ID="${MODE}_$(date -u +%Y%m%dT%H%M%SZ)"; fi
if [[ ! "$RUN_ID" =~ ^[A-Za-z0-9_.-]+$ ]]; then
  echo "ERR: invalid run ID" >&2; exit 2
fi

if [ -n "$(git -C "$REPO_ROOT" status --short)" ]; then
  echo "ERR: canonical run requires a clean Dec-LIO worktree" >&2; exit 3
fi

RUN_DIR="$OUT/$RUN_ID"
if [ -e "$RUN_DIR" ]; then
  echo "ERR: refusing to overwrite $RUN_DIR" >&2; exit 2
fi
mkdir -p "$RUN_DIR/ros_log"
export ROS_LOG_DIR="$RUN_DIR/ros_log"
export TBB_NUM_THREADS="$THREADS" OMP_NUM_THREADS="$THREADS" OPENBLAS_NUM_THREADS="$THREADS"
NODE_LOG="$RUN_DIR/node.log"
CORE_LOG="$RUN_DIR/roscore.log"
PLAY_LOG="$RUN_DIR/play.log"
RECORD_LOG="$RUN_DIR/record.log"
META="$RUN_DIR/meta.txt"
RESULT_BAG="$RUN_DIR/online_odom.bag"

CORE_PID=""; NODE_PID=""; RECORD_PID=""; PLAY_PID=""
stop_group() {
  local pid="$1"
  [ -n "$pid" ] || return 0
  kill -INT -- "-$pid" 2>/dev/null || true
  sleep 1
  kill -TERM -- "-$pid" 2>/dev/null || true
}
cleanup() {
  stop_group "$PLAY_PID"
  stop_group "$RECORD_PID"
  stop_group "$NODE_PID"
  stop_group "$CORE_PID"
}
trap cleanup EXIT

{
  echo "repository_root: $REPO_ROOT"
  echo "catkin_workspace: $CATKIN_WS"
  echo "mode: $MODE"
  echo "sequence: $SEQUENCE"
  echo "run_id: $RUN_ID"
  echo "git_head: $(git -C "$REPO_ROOT" rev-parse HEAD)"
  echo "git_status: clean"
  echo "bag: $BAG"
  echo "bag_sha256: $(sha256sum "$BAG" | awk '{print $1}')"
  echo "config: $CONFIG"
  echo "config_sha256: $(sha256sum "$CONFIG" | awk '{print $1}')"
  echo "ground_truth: $GROUND_TRUTH"
  echo "ground_truth_sha256: $(sha256sum "$GROUND_TRUTH" | awk '{print $1}')"
  echo "duration: ${DURATION:-whole-bag}"
  echo "rate: $RATE"
  echo "requested_threads: $THREADS"
  echo "nproc: $(nproc)"
  echo "d1_shadow: $D1_SHADOW"
  echo "d2_shadow: $D2_SHADOW"
  echo "consistency_shadow: $CONSISTENCY_SHADOW"
  echo "axis_shadow: $AXIS_SHADOW"
  echo "d3_solver_shadow: $D3_SOLVER_SHADOW"
  echo "paired_attenuation: $PAIRED_ATTENUATION"
  echo "paired_attenuation_shadow: $PAIRED_ATTENUATION_SHADOW"
  echo "paired_attenuation_mode: $PAIRED_MODE"
  echo "prompt14_shadow: $P14_SHADOW"
  echo "prompt15_enabled: $P15_ENABLED"
  echo "prompt15_intervention_frame: ${P15_FRAME:-not-applicable}"
  echo "blind_override: ${BLIND_OVERRIDE:-config/default}"
  echo "filter_rate_override: ${FILTER_RATE_OVERRIDE:-config/default}"
  echo "voxel_override: ${VOXEL_OVERRIDE:-config/default}"
  echo "maxrange_override: ${MAXRANGE_OVERRIDE:-config/default}"
  echo "point_time_scale_override: ${POINT_TIME_SCALE_OVERRIDE:-config/default}"
  echo "geode_finite_then_stride_override: ${GEODE_FINITE_THEN_STRIDE_OVERRIDE:-config/default}"
  echo "effective_thread_policy: one sequential temporal epoch; native TBB backend"
  echo "play_topics: $PLAY_TOPICS"
  echo "record_topics: $RECORD_TOPICS"
  echo "start_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$META"

MASTER_PORT=$((11311 + RANDOM % 200))
export ROS_MASTER_URI="http://127.0.0.1:${MASTER_PORT}"
echo "ros_master_uri: $ROS_MASTER_URI" >> "$META"
setsid rosmaster -p "$MASTER_PORT" > "$CORE_LOG" 2>&1 & CORE_PID=$!
for _ in $(seq 1 60); do
  if timeout 2 rosnode list >/dev/null 2>&1; then break; fi
  sleep 1
done
if ! timeout 2 rosnode list >/dev/null 2>&1; then
  echo "ERR: rosmaster did not start" >&2; exit 4
fi

rosparam load "$CONFIG"
if [ -n "$BLIND_OVERRIDE" ]; then rosparam set /lio/sensor/blind "$BLIND_OVERRIDE"; fi
if [ -n "$FILTER_RATE_OVERRIDE" ]; then rosparam set /lio/sensor/filter_rate "$FILTER_RATE_OVERRIDE"; fi
if [ -n "$VOXEL_OVERRIDE" ]; then rosparam set /lio/sensor/voxel_fliter_size "$VOXEL_OVERRIDE"; fi
if [ -n "$MAXRANGE_OVERRIDE" ]; then rosparam set /lio/sensor/maxrange "$MAXRANGE_OVERRIDE"; fi
if [ -n "$POINT_TIME_SCALE_OVERRIDE" ]; then rosparam set /lio/sensor/point_time_scale "$POINT_TIME_SCALE_OVERRIDE"; fi
if [ -n "$GEODE_FINITE_THEN_STRIDE_OVERRIDE" ]; then rosparam set /lio/sensor/geode_finite_then_stride "$GEODE_FINITE_THEN_STRIDE_OVERRIDE"; fi
rosparam set /lio/offline/bag "$BAG"
rosparam set /lio/offline/start_offset -1.0
rosparam set /lio/offline/duration "${DURATION:--1.0}"
rosparam set /lio/offline/threads "$THREADS"
rosparam set /lio/dec_lio/d1_shadow/enabled "$D1_SHADOW"
rosparam set /lio/dec_lio/d1_shadow/condition_threshold "10.0"
rosparam set /lio/dec_lio/d1_shadow/output_csv "$RUN_DIR/dcreg_shadow.csv"
rosparam set /lio/dec_lio/d1_shadow/frame_summary_csv "$RUN_DIR/dcreg_frame_summary.csv"
rosparam set /lio/dec_lio/d2_shadow/enabled "$D2_SHADOW"
rosparam set /lio/dec_lio/d2_shadow/output_csv "$RUN_DIR/d2_frame_summary.csv"
rosparam set /lio/dec_lio/consistency_shadow/enabled "$CONSISTENCY_SHADOW"
rosparam set /lio/dec_lio/consistency_shadow/output_csv "$RUN_DIR/consistency_frame_summary.csv"
rosparam set /lio/dec_lio/axis_shadow/enabled "$AXIS_SHADOW"
rosparam set /lio/dec_lio/axis_shadow/output_csv "$RUN_DIR/axis_frame_summary.csv"
rosparam set /lio/dec_lio/d3_solver_shadow/enabled "$D3_SOLVER_SHADOW"
rosparam set /lio/dec_lio/d3_solver_shadow/output_csv "$RUN_DIR/d3_solver_shadow.csv"
rosparam set /lio/dec_lio/d3_solver_shadow/snapshot_path "$RUN_DIR/d3_solver_snapshots.bin"
rosparam set /lio/dec_lio/paired_attenuation/enabled "$PAIRED_ATTENUATION"
rosparam set /lio/dec_lio/paired_attenuation/shadow_only "$PAIRED_ATTENUATION_SHADOW"
rosparam set /lio/dec_lio/paired_attenuation/mode "$PAIRED_MODE"
rosparam set /lio/dec_lio/paired_attenuation/output_csv "$RUN_DIR/paired_attenuation_shadow.csv"
rosparam set /lio/dec_lio/prompt14_shadow/enabled "$P14_SHADOW"
rosparam set /lio/dec_lio/prompt14_shadow/frame_csv "$RUN_DIR/prompt14_shadow.csv"
rosparam set /lio/dec_lio/prompt14_shadow/mode_csv "$RUN_DIR/prompt14_modes.csv"
rosparam set /lio/dec_lio/prompt15/enabled "$P15_ENABLED"
rosparam set /lio/dec_lio/prompt15/intervention_frame "${P15_FRAME:--1}"
rosparam set /lio/dec_lio/prompt15/event_csv "$RUN_DIR/prompt15_event.csv"
rosparam set /lio/dec_lio/observation_stage_csv "$RUN_DIR/observation_stage.csv"
if [ "$MODE" = offline ]; then rosparam set /lio/offline/out_dir "$RUN_DIR"; fi
rosparam dump "$RUN_DIR/effective_rosparams.yaml" /lio
echo "effective_rosparams_sha256: $(sha256sum "$RUN_DIR/effective_rosparams.yaml" | awk '{print $1}')" >> "$META"

if [ "$MODE" = offline ]; then
  setsid rosrun super_lio super_lio_offline_node __name:=lio_offline > "$NODE_LOG" 2>&1 & NODE_PID=$!
  set +e
  wait "$NODE_PID"
  NODE_RC=$?
  set -e
  RUN_RC="$NODE_RC"
else
  setsid rosrun super_lio super_lio_node __name:=super_lio_node > "$NODE_LOG" 2>&1 & NODE_PID=$!
  for _ in $(seq 1 90); do
    if rostopic list 2>/dev/null | grep -qx /lio/odom; then break; fi
    sleep 1
  done
  IFS=',' read -r -a RECORD_ARRAY <<< "$RECORD_TOPICS"
  setsid rosbag record -O "$RESULT_BAG" "${RECORD_ARRAY[@]}" > "$RECORD_LOG" 2>&1 & RECORD_PID=$!
  sleep 3
  IFS=',' read -r -a PLAY_ARRAY <<< "$PLAY_TOPICS"
  PLAY_ARGS=(rosbag play "$BAG" --rate "$RATE")
  if [ -n "$DURATION" ]; then PLAY_ARGS+=(--duration "$DURATION"); fi
  PLAY_ARGS+=(--topics "${PLAY_ARRAY[@]}")
  set +e
  "${PLAY_ARGS[@]}" > "$PLAY_LOG" 2>&1
  PLAY_RC=$?
  set -e
  stop_group "$NODE_PID"
  set +e
  wait "$NODE_PID"
  NODE_RC=$?
  set -e
  stop_group "$RECORD_PID"
  set +e
  wait "$RECORD_PID"
  RECORD_RC=$?
  set -e
  RUN_RC="$PLAY_RC"
  [ "$NODE_RC" -eq 0 ] || RUN_RC="$NODE_RC"
  if [ "$RECORD_RC" -gt 1 ]; then RUN_RC="$RECORD_RC"; fi
  if [ "$RUN_RC" -eq 0 ]; then
    python3 "$REPO_ROOT/eval/dec_lio/pose_bag_to_tum.py" \
      --bag "$RESULT_BAG" --topic /lio/odom --output "$RUN_DIR/trajectory.tum" \
      > "$RUN_DIR/trajectory_convert.log" 2>&1 || RUN_RC=$?
  fi
fi

echo "node_rc: ${NODE_RC:-$RUN_RC}" >> "$META"
echo "play_rc: ${PLAY_RC:-not-applicable}" >> "$META"
echo "record_rc: ${RECORD_RC:-not-applicable}" >> "$META"
echo "trajectory: $RUN_DIR/trajectory.tum" >> "$META"
if [ -f "$RUN_DIR/trajectory.tum" ]; then
  if [ "$SEQUENCE" = "tunneling_tunnel2" ] ||
     [ "$SEQUENCE" = "tunneling_tunnel2_alpha" ] ||
     [ "$SEQUENCE" = "tunneling_tunnel2_gamma" ]; then
    python3 "$REPO_ROOT/eval/dec_lio/normalize_tum_timestamps.py" \
      --input "$RUN_DIR/trajectory.tum" \
      --output "$RUN_DIR/trajectory.strict.tum"
    mv "$RUN_DIR/trajectory.strict.tum" "$RUN_DIR/trajectory.tum"
  fi
  echo "trajectory_sha256: $(sha256sum "$RUN_DIR/trajectory.tum" | awk '{print $1}')" >> "$META"
  echo "trajectory_bytes: $(wc -c < "$RUN_DIR/trajectory.tum")" >> "$META"
else
  echo "trajectory_sha256: MISSING" >> "$META"
  RUN_RC=1
fi
if grep -Eiq 'fatal|segmentation fault|assertion failed|nan.*nan' "$NODE_LOG" 2>/dev/null; then
  echo "fatal_marker: PRESENT" >> "$META"; RUN_RC=1
else
  echo "fatal_marker: NONE" >> "$META"
fi
echo "end_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$META"
echo "__DEC_LIO_RUN_DONE_RC=$RUN_RC"
exit "$RUN_RC"
