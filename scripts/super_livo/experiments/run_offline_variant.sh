#!/bin/bash
# Reusable Super-LIVO offline variant runner (single variant per invocation).
# Persisted in Git. Fail-closed: explicit variant matrix + param readback +
# isolated ROS master + bounded readiness poll + required-file checks.
#
# Usage:
#   run_offline_variant.sh <config_yaml> <bag|bags_csv> <out_prefix> <variant>
#                          [camera_topic] [camera_calib] [duration] [s0_audit]
#   variant: b0|c0|a0|a1
set -euo pipefail

CFG="${1:?config yaml}"
BAGSPEC="${2:?bag or comma-separated bags}"
OUT_PREFIX="${3:?out prefix}"
VARIANT="${4:?variant: b0|c0|a0|a1}"
CAM_TOPIC="${5:-}"
CAM_CALIB="${6:-}"
DURATION="${7:--1}"
S0_AUDIT="${8:-0}"

ROOT=/home/lc/super_livo
NODE="$ROOT/devel/.private/super_lio/lib/super_lio/super_lio_offline_node"
SETUP_ROS="$ROOT/devel/setup.bash"
MCD_CALIB="$ROOT/results/super_livo/tb0/config/mcd_camera.yaml"

# ---- required-file checks (fail before roscore) ----
for f in "$SETUP_ROS" "$CFG" "$NODE"; do
  [ -f "$f" ] || { echo "FAIL: missing required file $f"; exit 2; }
done
if [ "$VARIANT" != "b0" ]; then
  [ -f "$CAM_CALIB" ] || { echo "FAIL: missing camera calib $CAM_CALIB"; exit 2; }
fi
IFS=',' read -ra BAGS <<< "$BAGSPEC"
for b in "${BAGS[@]}"; do
  [ -f "$b" ] || { echo "FAIL: missing bag $b"; exit 2; }
done

# ---- isolated ROS master on a free localhost port ----
PORT=$(python3 -c 'import socket; s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()')
export ROS_MASTER_URI="http://127.0.0.1:$PORT"

OUT_DIR="$OUT_PREFIX"
mkdir -p "$OUT_DIR"

source "$SETUP_ROS"
RCORE=""
cleanup() {
  if [ -n "$RCORE" ] && kill -0 "$RCORE" 2>/dev/null; then
    kill "$RCORE" 2>/dev/null || true
    wait "$RCORE" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

setsid roscore -p "$PORT" > "$OUT_DIR/roscore.log" 2>&1 < /dev/null &
RCORE=$!

# ---- bounded readiness poll (up to 15 s) ----
ready=0
for i in $(seq 1 30); do
  if timeout 2 rosparam list > /dev/null 2>&1; then ready=1; break; fi
  sleep 0.5
done
if [ "$ready" != "1" ]; then
  echo "FAIL: isolated master never became ready"
  exit 3
fi

rosparam load "$CFG" /
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
rosparam set /lio/v2/skip_fd true
rosparam set /lio/hb0/enabled false
rosparam set /lio/vp/enabled true
rosparam set /lio/s0/audit "$S0_AUDIT"

# ---- explicit variant matrix (fail-closed; no YAML-default reliance) ----
case "$VARIANT" in
  b0)
    rosparam set /camera/enabled false
    rosparam set /lio/v4/apply false
    rosparam set /lio/v4/outlier_gate false
    ;;
  c0)
    rosparam set /camera/enabled true
    rosparam set /lio/v4/apply false
    rosparam set /lio/v4/outlier_gate false
    ;;
  a0)
    rosparam set /camera/enabled true
    rosparam set /lio/v4/apply true
    rosparam set /lio/v4/outlier_gate false
    ;;
  a1)
    rosparam set /camera/enabled true
    rosparam set /lio/v4/apply true
    rosparam set /lio/v4/outlier_gate true
    ;;
  *) echo "unknown variant $VARIANT" >&2; exit 2 ;;
esac

if [ -n "$CAM_TOPIC" ]; then
  rosparam set /camera/topic "$CAM_TOPIC"
fi
if [ -n "$CAM_CALIB" ]; then
  rosparam set /camera/calib_file "$CAM_CALIB"
fi

# ---- param readback (fail before node start if any key param missing) ----
cam_en=$(rosparam get /camera/enabled)
app=$(rosparam get /lio/v4/apply)
gate=$(rosparam get /lio/v4/outlier_gate)
echo "=== variant=$VARIANT readback: camera=$cam_en apply=$app gate=$gate ==="
case "$VARIANT" in
  b0) [ "$cam_en" = "False" ] || [ "$cam_en" = "false" ] || { echo "FAIL readback camera"; exit 4; } ;;
  *) [ "$cam_en" = "True" ] || [ "$cam_en" = "true" ] || { echo "FAIL readback camera"; exit 4; } ;;
esac
case "$VARIANT" in
  b0|c0) [ "$app" = "False" ] || [ "$app" = "false" ] || { echo "FAIL readback apply"; exit 4; } ;;
  a0|a1) [ "$app" = "True" ] || [ "$app" = "true" ] || { echo "FAIL readback apply"; exit 4; } ;;
esac
case "$VARIANT" in
  a1) [ "$gate" = "True" ] || [ "$gate" = "true" ] || { echo "FAIL readback gate"; exit 4; } ;;
  *) [ "$gate" = "False" ] || [ "$gate" = "false" ] || { echo "FAIL readback gate"; exit 4; } ;;
esac

echo "=== running variant=$VARIANT out=$OUT_DIR ==="
set +e
"$NODE" > "$OUT_DIR/node_stdout.log" 2>&1
NODE_RC=$?
set -e
echo "NODE_RC=$NODE_RC"
echo "WRAPPER_RC=0"
echo "=== COMMAND_COMPLETE rc=$NODE_RC ==="
exit "$NODE_RC"