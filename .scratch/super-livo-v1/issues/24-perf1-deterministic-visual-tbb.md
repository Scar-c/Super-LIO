# [Super-LIVO v1][PERF-1] Deterministic FAST-LIVO2-like visual TBB expansion

**Status:** ACTIVE（Round 11M owner contract）

Graph:
```text
PERF-0 PASS（deterministic TBB accepted）
   ↓
PERF-1 (this ticket): 1A H/b addend compute parallel / 1B fused landmark
kernel / 1C candidate-texture (conditional)
   ↓
OWNER REVIEW
   ↓
V-4 future
```

Scope: move pure per-landmark arithmetic into deterministic TBB worker;
serial commit only exact-order H/b addition + mutation. Frozen math
semantics. Full bitwise + production-like realtime validation.
