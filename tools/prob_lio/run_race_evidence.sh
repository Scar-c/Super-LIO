#!/usr/bin/env bash
# Prob-LIO G-P2.C2 evidence: TSAN detection of the legacy shared non-atomic
# counter pattern vs the fixed atomic pattern (same tbb::parallel_for shape
# as production Observe()).
#
# Usage: tools/prob_lio/run_race_evidence.sh [--out results/prob_lio]
set -u
WS_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${1:-$WS_ROOT/results/prob_lio}"
STAMP="$(date +%Y%m%d_%H%M%S)"
RUN_DIR="$OUT/race_evidence_$STAMP"
mkdir -p "$RUN_DIR"
SRC="$WS_ROOT/tests/prob_lio/race_evidence.cpp"
INCS="-I/usr/include/eigen3 -I$WS_ROOT/src/super_lio/3rdparty"
LDFLAGS="-ltbb"

g++ -std=c++17 -O1 -g -fsanitize=thread -B/usr/lib/gcc/x86_64-linux-gnu/10 $INCS "$SRC" $LDFLAGS -o "$RUN_DIR/race_good" 2>"$RUN_DIR/build_good.log" || { echo "build good failed"; exit 2; }
g++ -std=c++17 -O1 -g -fsanitize=thread -DBAD -B/usr/lib/gcc/x86_64-linux-gnu/10 $INCS "$SRC" $LDFLAGS -o "$RUN_DIR/race_bad" 2>"$RUN_DIR/build_bad.log" || { echo "build bad failed"; exit 2; }

set -o pipefail
# TBB 2020.1 internals produce known TSAN false positives; suppress ONLY
# those library-internal frames (see tests/prob_lio/tsan_suppressions.txt).
# The counter pattern under test is not suppressed.
SUPP="$WS_ROOT/tests/prob_lio/tsan_suppressions.txt"
export TSAN_OPTIONS="suppressions=$SUPP:halt_on_error=1"
"$RUN_DIR/race_good" >"$RUN_DIR/good.out" 2>&1
GOOD_RC=$?
export TSAN_OPTIONS="suppressions=$SUPP"
"$RUN_DIR/race_bad" >"$RUN_DIR/bad.out" 2>&1
BAD_RC=$?

TSAN_RACE_LINES="$(grep -c "WARNING: ThreadSanitizer: data race" "$RUN_DIR/bad.out" || true)"
GOOD_CLEAN="$([ "$(grep -c "WARNING: ThreadSanitizer" "$RUN_DIR/good.out" || true)" = 0 ] && echo yes || echo no)"

echo "good(atomic) rc=$GOOD_RC tsan_clean=$GOOD_CLEAN" | tee "$RUN_DIR/summary.txt"
echo "bad(legacy)  rc=$BAD_RC tsan_data_race_lines=$TSAN_RACE_LINES" | tee -a "$RUN_DIR/summary.txt"

if [ "$GOOD_CLEAN" = yes ] && [ "$TSAN_RACE_LINES" -ge 1 ]; then
  echo "RESULT: PASS (legacy pattern detected by TSAN; fixed pattern clean)" | tee -a "$RUN_DIR/summary.txt"
  exit 0
else
  echo "RESULT: FAIL" | tee -a "$RUN_DIR/summary.txt"
  exit 1
fi
