#!/bin/bash
# Dataset-only adapter: no protected algorithm semantics are reconstructed here.
# Prompt70 ambient-environment guard: a production adapter must never enter a
# test path merely because the parent shell carries stale test variables.
# Fail closed before the supervisor/runner/node start (preferred over silent
# sanitization: contaminated shells are exposed, not hidden).
set -euo pipefail

ambient=""
for var in SLV_TEST_MODE SLV_TEST_NODE_CMD SLV_TEST_VALIDATOR SLV_RUNNER SLV_LOCK_FILE; do
  if [ -n "${!var:-}" ]; then
    ambient="$ambient $var=${!var}"
  fi
done
ambient="$ambient$(env | sed -n 's/^\(SLV_TEST_[A-Z0-9_]*=\).*/ \1<set>/p' | tr -d '\n')"
if [ -n "$ambient" ]; then
  echo "PRODUCTION_ADAPTER_PREFLIGHT_FAIL: ambient test/executable/resource override present:$ambient" >&2
  exit 2
fi

RUN_ID="${1:?run id}"
export SLV_CFG=/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml
export SLV_BAG=/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
export SLV_CAM_TOPIC=/left/image_raw
export SLV_CAM_CALIB=/home/lc/super_livo/bag/NTU/eee_01/camera_left.yaml
export SLV_CAM_OFFSET=-0.0199575325817
export SLV_DATASET=NTU
export SLV_SEQUENCE=eee_01
export SLV_MIN_ROWS=3000
exec /home/lc/super_livo/src/Super-LIO/scripts/super_livo/experiments/run_superlivo_transaction.sh \
  "$RUN_ID" /home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01
