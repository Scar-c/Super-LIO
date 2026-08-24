# Super-LIVO 文档索引

> 本 README 是 `docs/super_livo/` 的统一入口。**CURRENT SOURCE OF TRUTH** 与 **HISTORICAL/SUPERSEDED** 必须区分，未来 agent 不得把 v0 spec 当 current。

## CURRENT SOURCE OF TRUTH（当前有效，按优先级）

| 文档 | 内容 | 状态 |
|---|---|---|
| `redesign/architecture_owner_decisions.md` | Architecture Owner 冻结决定（最高优先） | CURRENT |
| `specs/super_livo_v1_spec.md` | v1 implementation spec（TB-1..M-0） | CURRENT |
| `redesign/`（micro_surfel_architecture_draft / memory_tradeoff / feasibility_spec / migration_matrix / v1_tracer_bullets_draft） | v1 架构草案与可行性规格（含冻结附录 A-D） | CURRENT |
| `CONTEXT.md` | 领域术语 / 状态语义 / invariants | CURRENT（I-01/03/09 等按 migration matrix AMEND 后语义） |
| `adr/ADR-001..007` | 架构决定 | CURRENT（migration matrix 标注 AMEND 项生效） |
| `round0_source_archaeology.md` | 三仓库源码考古事实 | REFERENCE |
| `datasets/`（dataset_registry / evaluation_protocol / calibration_time_sync） | 数据集注册 / 评测语义 / 标定时间同步 | CURRENT |
| `parameters/parameter_policy.md` | 参数 provenance 策略（P-A..P-D） | CURRENT |
| `offline/offline_runner_design.md` | offline backend 设计 | CURRENT |
| `recovery/round4_tb0_recovery_status.md` | TB-0 恢复状态（RECOVERED AND COMPLETED） | REFERENCE |
| `recovery/offline_timestamp_audit.md` | offline 时间语义审计 | REFERENCE |
| `recovery/offline_debug_handoff.md` | 调试交接（历史，问题已解决） | REFERENCE |

## HISTORICAL / SUPERSEDED

| 文档 | 状态 |
|---|---|
| `specs/super_livo_v0_spec.md` | **SUPERSEDED**（v1 spec wins；仅历史参考） |
| `.scratch/super-livo-v0/issues/` | SUPERSEDED AFTER TB-1（v1 tracker 替代） |
| `round0` 相关早期结论 | REFERENCE（不推翻，但以 migration matrix 为准） |

## 分类索引

- **Architecture Owner Decisions**：redesign/architecture_owner_decisions.md
- **Current v1 Spec**：specs/super_livo_v1_spec.md
- **Context / invariants**：CONTEXT.md
- **ADRs**：adr/ADR-001..007
- **Source archaeology**：round0_source_archaeology.md
- **Offline infrastructure**：offline/offline_runner_design.md
- **Recovery / baseline**：recovery/*
- **Micro-surfel redesign**：redesign/*
- **Dataset / evaluation**：datasets/*
- **Parameter policy**：parameters/parameter_policy.md
- **Historical v0 spec**：specs/super_livo_v0_spec.md（SUPERSEDED）
- **Prompt history**：`prompts/README.md`（repo 根 prompts/ 目录）
- **Local tracker**：`.scratch/super-livo-v1/issues/`（active）、`.scratch/super-livo-v0/issues/`（历史）

## 文档约定

见 `DOCUMENT_CONVENTIONS.md`；模块实况见 `PROJECT_MAP.md`。