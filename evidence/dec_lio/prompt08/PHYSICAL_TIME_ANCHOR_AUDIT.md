# Corrected physical point-time and end-time audit

For every accepted point the corrected branch uses
`t_i = header.stamp + raw_point_time`, with `point_time_scale = 1.0`. The bag
raw field is therefore an offset around a header time anchor, not a field whose
meaning is inferred from the variable name `start_time`.

Raw offset summary (seconds):

| scene | min | median | max |
|---|---:|---:|---:|
| Stairs | -0.100947454572 | -0.0510019846261 | 0.00130636803806 |
| Tunnel2 | -0.101028352976 | -0.0457914881408 | 0.00130636803806 |

Corrected per-frame audit:

| scene | arm | rows | accepted span min/median/P95/max s | last-selected − max-selected min/median/max s | end-time margin min/median/max s | pc-size invariant | end-time invariant |
|---|---|---:|---|---|---|---|---|
| stairs | S_correct | 3446 | 0.02969036624/0.09953023976/0.1008877456/0.1022492145 | -2.073601354e-05/0/0 | 0/0/0 | YES | YES |
| stairs | A_correct | 3446 | 0.04878336564/0.09954240179/0.10089139/0.1022492145 | -2.073601354e-05/0/0 | 0/0/0 | YES | YES |
| tunnel2 | S_correct | 2739 | 0.09745613113/0.09955046524/0.100850685/0.1023324169 | -6.912043318e-06/0/0 | 0/0/0 | YES | YES |
| tunnel2 | A_correct | 2739 | 0.09745613113/0.09955046524/0.100850685/0.1023324169 | -6.912043318e-06/0/0 | 0/0/0 | YES | YES |

The accepted spans are approximately 0.1 s, and the end-time margin is zero
because corrected code sets `lidar_data.end_time` to the header timestamp plus
the maximum accepted point offset. The `last_selected - max_selected`
distribution is retained as a diagnostic; its small negative tail proves why
using the last point alone is not a sufficient end-time definition.

Native compatibility-OFF deliberately retains the historical last-point
end-time path because exact native trajectory SHA parity is a Prompt08 hard
gate. The corrected GEODE finite→stride path has the required max-offset
correction; no point query-time semantics were changed.
