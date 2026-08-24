# 04_v1_implementation — 当前 Active Workflow

本目录是 Super-LIVO v1 **当前 active** 的 prompt 工作流。

## 当前状态

```text
Ready frontier: TB-1 only（.scratch/super-livo-v1/issues/#01）
v1 spec:  docs/super_livo/specs/super_livo_v1_spec.md
冻结决定: docs/super_livo/redesign/architecture_owner_decisions.md
```

## Active chain

```text
TB-1 → G-0 → G-1 → G-2 → G-3 → DG-0（owner gate）→ S-0 → S-1
→ V-0 → V-1 → V-2 → V-3 → V-4 → V-5 → L-0 → V-6 → M-0
```

## 本目录文件

| Seq | File | Status | Purpose |
|---|---|---|---|
| 11 | 11_round7_project_hygiene_dataset_registry_tb1.md | ACTIVE | 项目整理 + dataset registry + TB-1 实现 |

后续 v1 实现轮（G-0 起）的 prompt 应放本目录，编号继续 12、13、…。

## 规则

- 每轮一个逻辑 milestone commit；`.scratch/` 与 `results/` 不入功能 commit。
- 数据集：eee_01 → Corridor01 → SFS（见 docs/super_livo/datasets/）；offline-first。
- DG-0 为 mandatory owner gate（NO AUTO-CLOSE）。