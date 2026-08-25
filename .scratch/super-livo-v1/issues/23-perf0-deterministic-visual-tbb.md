# [Super-LIVO v1][PERF-0] Deterministic visual TBB with production H/b parity

**Status:** ACTIVE（Round 11L owner contract）

Graph:
```text
HB-0 PASS (80f6d09)
   ↓
PERF-0 (this ticket)
   ↓
OWNER REVIEW
   ↓
V-4 future
```

Scope: parallel read-only photometric prepare (landmark-level TBB, 8x8 serial
inside), serial H/b commit preserving exact per-sample production order,
serial/TBB bitwise parity, state-off trajectory MD5 parity, phase timing,
performance selection per frozen policy (§25). No V-4/ATE/weights.
