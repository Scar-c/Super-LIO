#!/bin/bash
# Canonical pristine Super-LIO NTU adapter. No estimator parameter override.
set -euo pipefail
RUN_ID="${1:?run_id}"; OUT_ROOT="${2:?output root}"; BAG="${3:?bag}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
RUN_DIR="$OUT_ROOT/$RUN_ID"; CFG=/home/lc/super_livo/base_ws/src/super_lio_upstream/src/super_lio/config/NTU.yaml
LAUNCH="$ROOT/scripts/super_livo/experiments/adapters/ntu_super_lio.launch"
export NTU_BAG="$BAG" NTU_CONFIG="$CFG" NTU_LAUNCH_PATH="$LAUNCH" NTU_LAUNCH_PACKAGE=__file__ NTU_LAUNCH_FILE=ntu_super_lio.launch
export NTU_REQUIRED_NODE=super_lio_node NTU_WS_SETUP=/home/lc/super_livo/base_ws/ws_super_lio/devel/setup.bash
export NTU_REQUIRED_TOPICS="/os1_cloud_node1/points /imu/imu"
export NTU_OUTPUT_PATH="$RUN_DIR/trajectory.parent.tum" NTU_MIN_ROWS=100 NTU_LAUNCH_ARGS="output:=$RUN_DIR/odometry.bag"
export NTU_POST_DRAIN_CMD="python3 '$ROOT/scripts/super_livo/evaluation/odom_bag_to_tum.py' --bag '$RUN_DIR/odometry.bag' --output '$NTU_OUTPUT_PATH' && python3 '$ROOT/scripts/super_livo/evaluation/pose_bag_to_tum.py' --bag '$BAG' --topic /leica/pose/relative --output '$RUN_DIR/leica.tum'"
export NTU_PARITY_CMD="python3 - '$CFG' '$RUN_DIR/effective_rosparams.after_launch.yaml' <<'PY'
import sys,yaml
official=yaml.safe_load(open(sys.argv[1])); runtime=yaml.safe_load(open(sys.argv[2]));
missing=[k for k in official if k not in runtime]
print('CONFIG_PARITY = '+('PASS' if not missing else 'FAIL'), 'missing=',missing)
raise SystemExit(bool(missing))
PY"
export NTU_EVAL_CMD="python3 '$ROOT/scripts/super_livo/evaluation/eval_ntu_viral_official.py' \"\$EVAL_TRAJ\" '$RUN_DIR/leica.tum' --out '$RUN_DIR/metrics.yaml'"
exec "$ROOT/scripts/super_livo/experiments/run_ntu_transaction.sh" "$RUN_ID" "$OUT_ROOT"
