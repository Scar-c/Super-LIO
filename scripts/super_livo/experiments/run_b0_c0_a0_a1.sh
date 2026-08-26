#!/bin/bash
# Run the full B0/C0/A0/A1 stack for a dataset with one reusable runner.
# Usage: run_b0_c0_a0_a1.sh <config_yaml> <bag|bags_csv> <out_prefix>
#                            [camera_topic] [camera_calib] [duration]
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CFG="${1:?config}"
BAG="${2:?bag}"
OUT="${3:?out prefix}"
CAM_TOPIC="${4:-}"
CAM_CALIB="${5:-}"
DUR="${6:--1}"

for v in b0 c0 a0 a1; do
  echo "========== $v =========="
  "$HERE/run_offline_variant.sh" "$CFG" "$BAG" "$OUT/$v" "$v" \
    "$CAM_TOPIC" "$CAM_CALIB" "$DUR"
  rc=$?
  echo "$v rc=$rc"
  if [ "$rc" -ne 0 ]; then
    echo "FAIL-FAST: variant $v failed with rc=$rc"
    exit "$rc"
  fi
done
echo "stack complete"