# Evidence Index

Status: accepted（Round 8-9）
Scope: micro-surfel feasibility evidence 索引

| Doc | Status | Commit | Datasets | 内容 |
|---|---|---|---|---|
| g0_micro_surfel_statistics.md | PASS | 8f355a1 | eee/nya | sidecar/Welford oracle、parity、memory |
| g1_visual_geometry_support.md | NO-GO | a7d7313 | eee/nya | 0.25m child plane 16-gate coverage、根因（q_flat） |
| g1r_parent_support_corrective.md | GO | f0f6cea | eee/nya | parent(0.5m) aggregate 32-gate、N-bin、provisional gate |
| g2_maturity_geometry_sync.md | PASS | 5279f1c | eee/nya | maturity（parent 更快）、E0/E1/E3 事件 |
| g3_direct_plane_shadow.md | PASS(shadow) | 5279f1c | eee/nya | child/parent vs HKNN 三路、HIGH COVERAGE / MODERATE AGREEMENT |
| g1v_scheme_b_patch_geometry.md | PASS(shadow) | 1a157a4 | eee/nya | Scheme-B surfel+offset：B-PARENT ≈ O-HKNN oracle；\|Δu*\|≈5px 系统性；DC 归一化必需 |
| dg0_micro_surfel_decision_pack.md | BLOCKED FOR OWNER REVIEW | 本轮 | — | support-scale/storage/gate/lifecycle 决策包 + MISSING EVIDENCE |

MISSING EVIDENCE：Corridor01 / Corridor02 / SFS（topic audit / config / CompressedImage 适配待建）。