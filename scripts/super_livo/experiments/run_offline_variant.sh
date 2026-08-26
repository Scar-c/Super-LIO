#!/bin/bash
# Reusable Super-LIVO offline variant runner (single variant per invocation).
# Persisted in Git. Fail-closed: explicit variant matrix + param readback +
# isolated ROS master + bounded readiness poll + required-file checks.
#
# Usage:
#   run_offline_variant.sh <config_yaml> <bag|bags_csv> <out_prefix> <variant>
#                          [camera_topic] [camera_calib] [duration] [s0_audit]
#                          [lidar_update_policy]
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
LIDAR_UPDATE_POLICY="${9:-partial}"
LAYER_AUDIT="${10:-0}"

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
rosparam set /lio/offline/layer_audit "$LAYER_AUDIT"
rosparam set /lio/camera_epoch/lidar_update_policy "$LIDAR_UPDATE_POLICY"
# Reconstructed last-known-good (code-gate basis): the V-4A/V-4C blocks in
# super_lio.cpp require g_lio_v4_apply && g_lio_camera_epoch && g_lio_v2_enabled
# && g_lio_v0_enabled; historical C0/A0/A1 runs therefore had v0=true,
# v2=true (otherwise A1 would equal C0 exactly). B0 is unaffected because
# camera_epoch=false.
rosparam set /lio/v0/enabled true
rosparam set /lio/v2/enabled true

# ---- explicit variant matrix (fail-closed; no YAML-default reliance) ----
case "$VARIANT" in
  b0)
    rosparam set /camera/enabled false
    rosparam set /lio/camera_epoch/enabled false
    rosparam set /lio/v4/apply false
    rosparam set /lio/v4/outlier_gate false
    ;;
  c0)
    rosparam set /camera/enabled true
    rosparam set /lio/camera_epoch/enabled true
    rosparam set /lio/v4/apply false
    rosparam set /lio/v4/outlier_gate false
    ;;
  a0)
    rosparam set /camera/enabled true
    rosparam set /lio/camera_epoch/enabled true
    rosparam set /lio/v4/apply true
    rosparam set /lio/v4/outlier_gate false
    ;;
  a1)
    rosparam set /camera/enabled true
    rosparam set /lio/camera_epoch/enabled true
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
read_param() {
  rosparam get "$1" 2>/dev/null || { echo "FAIL: param $1 unreadable"; exit 4; }
}
is_true() { [ "$1" = "True" ] || [ "$1" = "true" ]; }
is_false() { [ "$1" = "False" ] || [ "$1" = "false" ]; }
cam_en=$(read_param /camera/enabled)
cam_ep=$(read_param /lio/camera_epoch/enabled)
app=$(read_param /lio/v4/apply)
gate=$(read_param /lio/v4/outlier_gate)
v0=$(read_param /lio/v0/enabled)
v2=$(read_param /lio/v2/enabled)
skip_fd=$(read_param /lio/v2/skip_fd)
hb0=$(read_param /lio/hb0/enabled)
vp=$(read_param /lio/vp/enabled)
s0_aud=$(read_param /lio/s0/audit)
lidar_policy=$(read_param /lio/camera_epoch/lidar_update_policy)
echo "=== variant=$VARIANT readback: camera=$cam_en camera_epoch=$cam_ep apply=$app gate=$gate v0=$v0 v2=$v2 skip_fd=$skip_fd hb0=$hb0 vp=$vp s0_audit=$s0_aud lidar_update_policy=$lidar_policy ==="
case "$LIDAR_UPDATE_POLICY" in
  partial|shadow_fullscan|imu_fullscan) ;;
  *) echo "FAIL readback unknown lidar update policy $LIDAR_UPDATE_POLICY"; exit 4 ;;
esac
[ "$lidar_policy" = "$LIDAR_UPDATE_POLICY" ] || { echo "FAIL readback lidar_update_policy"; exit 4; }
case "$VARIANT" in
  b0) is_false "$cam_en" || { echo "FAIL readback camera"; exit 4; }
      is_false "$cam_ep" || { echo "FAIL readback camera_epoch"; exit 4; } ;;
  *) is_true "$cam_en" || { echo "FAIL readback camera"; exit 4; }
     is_true "$cam_ep" || { echo "FAIL readback camera_epoch"; exit 4; } ;;
esac
case "$VARIANT" in
  b0|c0) is_false "$app" || { echo "FAIL readback apply"; exit 4; } ;;
  a0|a1) is_true "$app" || { echo "FAIL readback apply"; exit 4; } ;;
esac
case "$VARIANT" in
  a1) is_true "$gate" || { echo "FAIL readback gate"; exit 4; } ;;
  *) is_false "$gate" || { echo "FAIL readback gate"; exit 4; } ;;
esac
is_true "$v0" || { echo "FAIL readback v0"; exit 4; }
is_true "$v2" || { echo "FAIL readback v2"; exit 4; }
is_true "$skip_fd" || { echo "FAIL readback skip_fd"; exit 4; }
is_false "$hb0" || { echo "FAIL readback hb0"; exit 4; }
is_true "$vp" || { echo "FAIL readback vp"; exit 4; }

echo "=== running variant=$VARIANT out=$OUT_DIR ==="
set +e
"$NODE" > "$OUT_DIR/node_stdout.log" 2>&1
NODE_RC=$?
set -e
echo "NODE_RC=$NODE_RC"
echo "WRAPPER_RC=0"
echo "=== COMMAND_COMPLETE rc=$NODE_RC ==="
exit "$NODE_RC"
