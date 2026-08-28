#!/bin/bash
# Dataset-only adapter: no protected algorithm semantics are reconstructed here.
set -euo pipefail
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
