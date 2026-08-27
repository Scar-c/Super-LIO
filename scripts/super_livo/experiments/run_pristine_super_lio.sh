#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "usage: $0 PRISTINE_WS CONFIG BAG OUTPUT_DIR ROS_PORT" >&2
  exit 2
fi

PRISTINE_WS=$1
CONFIG=$2
BAG=$3
OUTPUT_DIR=$4
ROS_PORT=$5
REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)

mkdir -p "$OUTPUT_DIR"
export ROS_MASTER_URI="http://127.0.0.1:${ROS_PORT}"
export ROS_HOME="$OUTPUT_DIR/ros_home"
mkdir -p "$ROS_HOME"

source /opt/ros/noetic/setup.bash
source "$PRISTINE_WS/devel/setup.bash"

pids=()
cleanup() {
  local pid
  for pid in "${pids[@]:-}"; do
    kill -INT "$pid" 2>/dev/null || true
  done
  sleep 1
  for pid in "${pids[@]:-}"; do
    kill -TERM "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
  done
}
trap cleanup EXIT INT TERM

roscore -p "$ROS_PORT" >"$OUTPUT_DIR/roscore.log" 2>&1 &
pids+=("$!")
for _ in $(seq 1 100); do
  if rosparam list >/dev/null 2>&1; then break; fi
  sleep 0.1
done
rosparam list >/dev/null

rosparam load "$CONFIG"
rosrun super_lio super_lio_node >"$OUTPUT_DIR/super_lio.log" 2>&1 &
pids+=("$!")
for _ in $(seq 1 100); do
  if rostopic list 2>/dev/null | grep -qx '/lio/odom'; then break; fi
  sleep 0.1
done
rostopic list | grep -qx '/lio/odom'
rosparam dump "$OUTPUT_DIR/effective_rosparams.after_launch.yaml"

rosbag record -O "$OUTPUT_DIR/odometry.bag" /lio/odom >"$OUTPUT_DIR/record.log" 2>&1 &
pids+=("$!")
sleep 1
rosbag play --rate=5 --delay=0.5 "$BAG" >"$OUTPUT_DIR/play.log" 2>&1
sleep 2
cleanup
trap - EXIT INT TERM

python3 "$REPO_ROOT/scripts/super_livo/evaluation/odom_bag_to_tum.py" \
  --bag "$OUTPUT_DIR/odometry.bag" --output "$OUTPUT_DIR/trajectory.tum"

git -C "$PRISTINE_WS/src/Super-LIO" rev-parse HEAD >"$OUTPUT_DIR/git_revision.txt"
git -C "$PRISTINE_WS/src/Super-LIO" status --porcelain >"$OUTPUT_DIR/git_dirty.txt"
sha256sum "$CONFIG" "$BAG" "$OUTPUT_DIR/effective_rosparams.after_launch.yaml" \
  "$OUTPUT_DIR/trajectory.tum" >"$OUTPUT_DIR/SHA256SUMS"
printf '%q ' "$0" "$@" >"$OUTPUT_DIR/command.txt"
printf '\n' >>"$OUTPUT_DIR/command.txt"
echo ROUND12_PRISTINE_SUPER_LIO_RUN_COMPLETE
