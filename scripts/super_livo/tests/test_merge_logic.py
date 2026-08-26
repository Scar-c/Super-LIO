#!/usr/bin/env python3
"""Merge-spec unit tests (deterministic, mirrors OfflineReader k-way merge).

Properties:
  1. every input iterator advances monotonically
  2. every MessageInstance is dispatched at most once
  3. output timestamps are globally non-decreasing
  4. an EOF source never causes the current pick to be replayed
  5. total consumed <= total input relevant messages
"""

import random
import sys


def merge(iters):
    """Correct k-way merge: consume exactly once per picked message."""
    n = [0] * len(iters)
    consumed = [0] * len(iters)
    out = []
    while True:
        pick = -1
        best = None
        for i in range(len(iters)):
            if n[i] >= len(iters[i]):
                continue  # EOF source: skip, never replay
            if pick == -1 or iters[i][n[i]] < best:
                pick = i
                best = iters[i][n[i]]
        if pick == -1:
            break
        t = iters[pick][n[pick]]
        n[pick] += 1          # consume exactly once
        consumed[pick] += 1
        out.append((pick, t))
    return out, consumed


def broken_merge_continue_without_advance(iters):
    """Buggy variant: continue inside processing without advancing -> replay."""
    n = [0] * len(iters)
    out = []
    guard = 0
    while True:
        guard += 1
        if guard > 100000:
            raise RuntimeError("infinite replay")
        pick = -1
        best = None
        for i in range(len(iters)):
            if n[i] >= len(iters[i]):
                continue
            if pick == -1 or iters[i][n[i]] < best:
                pick = i
                best = iters[i][n[i]]
        if pick == -1:
            break
        t = iters[pick][n[pick]]
        # simulate a processing branch that "continues" without advancing
        if t % 2 == 0:
            continue  # BUG: n[pick] not incremented -> same message replayed
        n[pick] += 1
        out.append((pick, t))
    return out, []


def check(props):
    ok = True
    for name, cond in props:
        print('%s: %s' % (name, 'PASS' if cond else 'FAIL'))
        ok &= cond
    return ok


def main():
    ok = True
    # deterministic case: three sorted streams with interleaving + ties
    iters = [[1.0, 3.0, 3.5], [0.5, 2.0, 4.0], [2.5, 3.0]]
    out, consumed = merge(iters)
    times = [t for _, t in out]
    p = [
        ('P1 monotonic advance', consumed == [3, 3, 2] and sum(consumed) == 8),
        ('P2 dispatch at most once', len(out) == 8 and len(set((p, t) for p, t in out)) == 8),
        ('P3 non-decreasing', all(times[i] <= times[i + 1] for i in range(len(times) - 1))),
        ('P4 EOF no replay', times[0] == 0.5 and times[-1] == 4.0 and out[-1][0] == 1),
        ('P5 total == input', sum(consumed) == sum(len(x) for x in iters)),
    ]
    ok &= check(p)
    # random fuzz: compare against sorted ground truth
    rng = random.Random(42)
    for trial in range(50):
        iters = [sorted(rng.uniform(0, 100) for _ in range(rng.randint(1, 30)))
                 for _ in range(rng.randint(2, 5))]
        out, consumed = merge(iters)
        times = [t for _, t in out]
        gt = sorted(t for it in iters for t in it)
        assert times == gt, 'fuzz mismatch'
        assert sum(consumed) == len(gt)
    print('fuzz 50 trials: PASS')
    # the buggy variant must be caught (replay) by the guard
    caught = False
    try:
        broken_merge_continue_without_advance([[0.0, 2.0, 4.0]])
    except RuntimeError:
        caught = True
    print('bug detector catches no-advance continue: %s' % ('PASS' if caught else 'FAIL'))
    ok &= caught
    print('MERGE TDD:', 'ALL PASS' if ok else 'FAIL')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())