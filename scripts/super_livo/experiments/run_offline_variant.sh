#!/bin/bash
# Reusable Super-LIVO offline variant runner (B0/C0/A0/A1).
# Persisted under the repository (not /tmp) per execution-hygiene policy.
#
# Usage:
#   run_offline_variant.sh <config_yaml> <bag|bags_csv> <out_prefix> <variant>
#                          [camera_topic] [camera_calib]
#   variant: b0|c0|a0|a1
set -u
source /opt/ros/noetic/setup.bash
source /home/lc/super_livo/devel/setup.bash

CFG="${1:?config yaml}"
BAGSPEC="${2:?bag or comma-separated bags}"
OUT_PREFIX="${3:?out prefix}"
VARIANT="${4:?variant: b0|c0|a0|a1}"
CAM_TOPIC="${5:-}"
CAM_CALIB="${6:-}"
DURATION="${7:--1}"

RCORE=""
cleanup() {
  if [ -n "$RCORE" ] && kill -0 "$RCORE" 2>/dev/null; then
    kill "$RCORE" 2>/dev/null || true
    wait "$RCORE" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

OUT_DIR="$OUT_PREFIX"
mkdir -p "$OUT_DIR"

setsid roscore > "$OUT_DIR/roscore.log" 2>&1 < /dev/null &
RCORE=$!
sleep 6

rosparam load "$CFG" /
# single bag or multi-bag (comma-separated)
if [[ "$BAGSPEC" == *","* ]]; then
  rosparam set /lio/offline/bags "$BAGSPEC"
else
  rosparam set /lio/offline/bag "$BAGSPEC"
fi
rosparam set /lio/offline/start_offset 0
rosparam set /lio/offline/duration "$DURATION"
rosparam set /lio/offline/out_dir "$OUT_DIR"
rosparam set /lio/offline/publish false
rosparam set /lio/eva/instrumentation false
rosparam set /lio/eva/out_dir "$OUT_DIR"

case "$VARIANT" in
  b0)
    rosparam set /lio/g0/shadow false
    rosparam set /lio/g1/enabled false
    rosparam set /lio/camera_epoch/enabled false
    rosparam set /lio/v0/enabled false
    rosparam set /lio/v2/enabled false
    rosparam set /lio/camera/enabled false
    rosparam set /lio/v4/apply false
    ;;
  c0)
    rosparam set /lio/g0/shadow true
    rosparam set /lio/g1/enabled true
    rosparam set /lio/g1/out_dir "$OUT_DIR"
    rosparam set /lio/camera_epoch/enabled true
    rosparam set /lio/v0/enabled true
    rosparam set /lio/v2/enabled true
    rosparam set /lio/v2/skip_fd true
    rosparam set /lio/hb0/enabled false
    rosparam set /lio/vp/enabled true
    rosparam set /lio/camera/enabled true
    rosparam set /lio/v4/apply false
    ;;
  a0)
    rosparam set /lio/g0/shadow true
    rosparam set /lio/g1/enabled true
    rosparam set /lio/g1/out_dir "$OUT_DIR"
    rosparam set /lio/camera_epoch/enabled true
    rosparam set /lio/v0/enabled true
    rosparam set /lio/v2/enabled true
    rosparam set /lio/v2/skip_fd true
    rosparam set /lio/hb0/enabled false
    rosparam set /lio/vp/enabled true
    rosparam set /lio/camera/enabled true
    rosparam set /lio/v4/apply true
    ;;
  a1)
    rosparam set /lio/g0/shadow true
    rosparam set /lio/g1/enabled true
    rosparam set /lio/g1/out_dir "$OUT_DIR"
    rosparam set /lio/camera_epoch/enabled true
    rosparam set /lio/v0/enabled true
    rosparam set /lio/v2/enabled true
    rosparam set /lio/v2/skip_fd true
    rosparam set /lio/hb0/enabled false
    rosparam set /lio/vp/enabled true
    rosparam set /lio/camera/enabled true
    rosparam set /lio/v4/apply true
    rosparam set /lio/v4/outlier_gate true
    ;;
  *)
    echo "unknown variant $VARIANT" >&2
    exit 2
    ;;
esac

if [ -n "$CAM_TOPIC" ]; then
  rosparam set /camera/topic "$CAM_TOPIC"
fi
if [ -n "$CAM_CALIB" ]; then
  rosparam set /camera/calib_file "$CAM_CALIB"
fi

echo "=== running variant=$VARIANT out=$OUT_DIR ==="
set +e
/home/lc/super_livo/devel/.private/super_lio/lib/super_lio/super_lio_offline_node \
  > "$OUT_DIR/node_stdout.log" 2>&1
NODE_RC=$?
echo "NODE_RC=$NODE_RC"
exit $NODE_RC