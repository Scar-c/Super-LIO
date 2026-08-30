#!/usr/bin/env bash
# Prob-LIO G-P4.C2 — canonical clean-run preflight guard test.
#
# Cases:
#   clean worktree + --canonical    -> allowed (guard passes)
#   dirty worktree + --canonical    -> refused (rc 3, message)
#   dirty worktree + no --canonical -> allowed (diagnostic run)
#
# Negative mutation: if the dirty-state guard is bypassed (removed/weakened),
# case 2 stops refusing (rc != 3) and this test FAILS — the guard's presence
# is the tested invariant.
set -u
WS_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RUNNER="$WS_ROOT/tools/prob_lio/run_baseline.sh"
BAG="${1:-$HOME/super_livo/bag/NTU/eee_01/eee_01.bag}"
FAILURES=0

git -C "$WS_ROOT" status --short | grep -q . && { echo "SKIP: starting worktree not clean"; exit 2; }

# 1) clean + canonical -> allowed (bounded 2 s window to keep it cheap)
"$RUNNER" --bag "$BAG" --canonical --duration 2 --out /tmp/opencode/gpc2 >/tmp/opencode/gpc2_clean.out 2>&1
RC=$?
echo "clean+canonical rc=$RC"
[ "$RC" -eq 0 ] || { echo "FAIL: clean+canonical refused"; FAILURES=$((FAILURES+1)); }

# 2) dirty + canonical -> refused (this assertion fails if the guard is
#    bypassed/removed — the negative mutation)
touch "$WS_ROOT/.gpc2_dirty_marker"
"$RUNNER" --bag "$BAG" --canonical --duration 2 --out /tmp/opencode/gpc2 >/tmp/opencode/gpc2_dirty.out 2>&1
RC=$?
grep -q "requires a clean worktree" /tmp/opencode/gpc2_dirty.out
GREP_OK=$?
rm -f "$WS_ROOT/.gpc2_dirty_marker"
echo "dirty+canonical rc=$RC grep=$GREP_OK"
{ [ "$RC" -eq 3 ] && [ "$GREP_OK" -eq 0 ]; } || { echo "FAIL: dirty+canonical not refused (guard bypassed?)"; FAILURES=$((FAILURES+1)); }

# 3) dirty + diagnostic (no --canonical) -> allowed
touch "$WS_ROOT/.gpc2_dirty_marker"
"$RUNNER" --bag "$BAG" --duration 2 --out /tmp/opencode/gpc2 >/tmp/opencode/gpc2_diag.out 2>&1
RC=$?
rm -f "$WS_ROOT/.gpc2_dirty_marker"
echo "dirty+diagnostic rc=$RC"
[ "$RC" -eq 0 ] || { echo "FAIL: dirty+diagnostic refused"; FAILURES=$((FAILURES+1)); }

echo "G-P4.C2 guard test failures=$FAILURES"
[ "$FAILURES" -eq 0 ] && echo "RESULT: PASS" || echo "RESULT: FAIL"
exit $FAILURES
