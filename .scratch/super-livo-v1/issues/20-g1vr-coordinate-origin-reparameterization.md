# 20: [Super-LIVO v1][G-1VR] Preserve patch identity under centroid reparameterization

**Status:** ready-for-agent（Round 11 corrective；coordinate-origin Scheme-B）

## Parent

本地 tracker parent：`00-parent.md`（Super-LIVO v1）

## Source specification

`docs/super_livo/specs/super_livo_v1_spec.md`；`docs/super_livo/redesign/architecture_owner_decisions.md`（D1-D5，Round 11 冻结）；Round 10 evidence g1v。

## Status note

Architecture Owner 正式冻结（D1）：**centroid = coordinate origin only**。
P_patch = μ_sync + δ_sync；geometry sync 只重参数化 offset（P_fixed 保持），
**不得因为 centroid/normal 更新移动物理 patch anchor**。

## Goal

把 Round 10 的"immutable d0 + moving surfel → moving P_B"修正为：
centroid 更新 → δ_new = P_fixed − μ_new（P_fixed 不移动）；3° event-triggered sync；
geometry lifecycle 与 observation lifecycle 解耦。

## What changes

- VisualLandmark 概念结构：parent_id、parent_generation、source_child_idx、mu_sync、delta_sync、n_sync、observations[3]、active_ref/best_alt/latest 槽、quality 元数据。
- 3° geometry sync：P_fixed = μ_old + δ_old；δ_new = P_fixed − μ_new；n_new = sign-consistent parent normal；atomic（mu_sync/delta_sync/n_sync/generation）。
- 每 sync 记录 e_P = ‖(μ_old+δ_old) − (μ_new+δ_new)‖（median/P99/max）。
- parent invalidation（eviction/generation change/plane invalid）→ landmark geometry deactivate；禁止 stale pointer。
- 视觉观察生命周期（D2/D3）：3-slot hard cap；slot A active reference、B best alternate、C latest candidate；patch uint8[64] persistent；individual patch immutable；active reference 可替换/重选（FAST-LIVO2-inspired lifecycle）。

## What does NOT change

- estimator 数值路径（G-1VR 本身 shadow；trajectory bitwise）。
- HKNN + plane fit remains authoritative（E-L deferred，D5）。
- 无 offset 优化 / landmark 位置优化 / patch-shift state / exposure state。

## Architectural invariants

- P_patch 物理位置不因 centroid 更新移动（creation anchor invariant + 3° sync anchor invariant）。
- ‖n‖=1（1e-6 内）；d_n ≤ ‖d‖、d_t ≤ ‖d‖、‖d‖² = d_n² + d_t²（float tolerance 记录）。
- 1 surfel : N landmarks；1 landmark : ≤3 observations。
- VisualMap sparse side table（禁止内嵌每个 OctVox parent）。
- 3° = BIEVR-inspired / reference-derived（P-B initial default，NO SWEEP；1/2/5° diagnostic-only）。

## Likely source seams

- `geometry/SchemeBShadow.h`（或迁移为 production 结构）；VisualMap 新模块；super_lio.cpp 挂接。

## Instrumentation

- creation anchor error、sync anchor error（median/P99/max）、sync 计数、eviction/generation 事件、offset identity invariant 断言。

## Tests required

- unit：creation anchor invariant、3° sync anchor invariant、多连 reparameterization、normal sign flip、parent centroid 大位移、eviction、generation reuse。
- real：eee_01 / nya_01 shadow + trajectory parity（bitwise）。

## Dataset / execution policy

- eee_01 / nya_01（required）。

## Acceptance criteria

- [ ] P_patch 在任意次 sync 后不变（数值零误差，float tolerance 记录）。
- [ ] offset identity invariant 全部通过。
- [ ] 无 stale pointer（parent 失效 → landmark 状态机）。
- [ ] eee/nya trajectory MD5 与 baseline bitwise 一致。
- [ ] 输出 sync 计数/anchor error/eviction 事件。

## Failure rules

- HB-1：reparameterization 移动 P_patch → STOP。
- HB-2：offset invariant 统计仍数学错误 → STOP。

## Allowed next step

S-0（camera-epoch sync，visual OFF）

## Blocked by

G-1V（`19-g1v-scheme-b-surfel-local-patch-audit.md`，historical shadow）

## Unblocks

S-0（`03-s0.md`）→ S-1 → V-0 → V-1 → V-2 → V-3 → V-4 → OWNER REVIEW

## Commit policy

`fix(super-livo): preserve patch identity across surfel sync`——source/tests/evidence/tracker 同 logical commit；push。

## Reference repo rule

refs/BIEVR-LIO / FAST-LIVO2 / open_vins = READ ONLY（FAST-LIVO2 检查 VisualPoint/Feature/updateVisualMapPoints/updateReferencePatch/patch extraction/warp/reference selection/NCC/viewing-angle scoring 只记录语义）。

## Scope prohibitions

MODE-B / Common-FEJ / MODE-C / direct parent LiDAR plane update / offset optimization / landmark position optimization / patch-shift state / exposure state / unbounded observation history / voxel resolution sweep / accepted gate sweep / patch-size sweep / observation-cap sweep。