#!/bin/bash
# Canonical pristine FAST-LIVO2 NTU adapter. Uses the pinned official launch/config unchanged.
set -euo pipefail
RUN_ID="${1:?run_id}"; OUT_ROOT="${2:?output root}"; BAG="${3:?bag}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"; RUN_DIR="$OUT_ROOT/$RUN_ID"
F2=/home/lc/super_livo/base_ws/src/fast_livo2_upstream
export NTU_BAG="$BAG" NTU_CONFIG="$F2/config/NTU_VIRAL.yaml" NTU_LAUNCH_PATH="$F2/launch/mapping_ouster_ntu.launch"
export NTU_LAUNCH_PACKAGE=fast_livo NTU_LAUNCH_FILE=mapping_ouster_ntu.launch NTU_LAUNCH_ARGS=rviz:=false NTU_REQUIRED_NODE=laserMapping
export NTU_REQUIRED_TOPICS="/os1_cloud_node1/points /imu/imu /left/image_raw"
export NTU_WS_SETUP=/home/lc/super_livo/base_ws/ws_fast_livo2/devel/setup.bash NTU_OUTPUT_PATH="$F2/Log/result/eee_01.txt" NTU_MIN_ROWS=100
export NTU_POST_DRAIN_CMD="python3 '$ROOT/scripts/super_livo/evaluation/pose_bag_to_tum.py' --bag '$BAG' --topic /leica/pose/relative --output '$RUN_DIR/leica.tum'"
export NTU_PARITY_CMD="python3 - '$F2/config/NTU_VIRAL.yaml' '$RUN_DIR/effective_rosparams.after_launch.yaml' <<'PY'
import sys,yaml
official=yaml.safe_load(open(sys.argv[1])); runtime=yaml.safe_load(open(sys.argv[2]));
missing=[k for k in official if k not in runtime]
print('CONFIG_PARITY = '+('PASS' if not missing else 'FAIL'), 'missing=',missing)
raise SystemExit(bool(missing))
PY"
export NTU_EVAL_CMD="python3 '$ROOT/scripts/super_livo/evaluation/eval_ntu_viral_official.py' \"\$EVAL_TRAJ\" '$RUN_DIR/leica.tum' --out '$RUN_DIR/metrics.yaml'"
exec "$ROOT/scripts/super_livo/experiments/run_ntu_transaction.sh" "$RUN_ID" "$OUT_ROOT"
