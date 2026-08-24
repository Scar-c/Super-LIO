# Super-LIVO Prompt History Index

Canonical directory：本 `prompts/` 目录。文件命名规范：`NN_short_snake_case_description.md`。

## 顺序与状态

| Seq | Canonical file | Original title | Status | Purpose | Input HEAD | Output HEAD | Superseded by |
|---|---|---|---|---|---|---|---|
| 00 | 00_preflight/00_repo_fork_topology.md | Super-LIVO Preflight — GitHub Fork & Repository Topology | EXECUTED | fork/remotes/super-livo 分支拓扑 | unknown（启动基线） | unknown（无功能 commit；P0-GIT 1..7 PASS） | — |
| 01 | 01_v0_architecture/01_round0_source_archaeology.md | Super-LIVO Round 0 — Source Archaeology & Architecture Evidence Pack | EXECUTED | 三仓库源码考古 / Evidence Pack | unknown | 9796b21 | — |
| 02 | 01_v0_architecture/02_round1_architecture_adr_freeze.md | Super-LIVO Round 1 — Architecture Domain Model & ADR Freeze | EXECUTED | CONTEXT + ADR-001..007 冻结 | 9796b21 | 41e27ea | — |
| 03 | 01_v0_architecture/03_round2_v0_implementation_spec.md | Super-LIVO Round 2 — v0 Implementation Specification | EXECUTED | v0 spec（TB-0..TB-13） | 41e27ea | 6983ad0 | SUPERSEDED by v1 spec（Round 5） |
| 04 | 01_v0_architecture/04_round3_v0_publish_tickets.md | Super-LIVO Round 3 — Publish Implementation Tickets | EXECUTED | v0 local tracker（.scratch/super-livo-v0/） | 6983ad0 | 6983ad0（no commit，tracker only） | SUPERSEDED after TB-1 by v1 tracker（Round 6） |
| 05 | 02_infrastructure/05_round4_tb0_baseline_instrumentation.md | Super-LIVO_Round4_TB0_Implementation | SUPERSEDED | TB-0 baseline+instrumentation（在线） | 6983ad0 | 7099f61（recovery report 中途停止） | MERGED into 07（offline-first recovery） |
| 06 | 02_infrastructure/06_tb_offline_bievr_style_corrective.md | Super-LIVO_TB-OFFLINE_Correct_BIEVRStyle_DS | EXECUTED | 修正 offline 语义（TopicQuery/publish/Publisher 生命周期/EOF） | ba98e3a | fecbdc6 | — |
| 07 | 02_infrastructure/07_offline_first_recovery_tb0_redesign.md | Super-LIVO_OfflineFirst_Recovery_TB0_MicroSurfel_Redesign_DS | EXECUTED（MERGED） | 统一计划：recovery + TB-OFFLINE + TB-0 + micro-surfel redesign（Phase R/O/B/D） | 7099f61 | 9e0ebe9 | —（其中 Phase D redesign 并入 v1 主线） |
| 08 | 03_v1_redesign/08_micro_surfel_architecture_redesign.md | Super-LIVO_MicroSurfel_Architecture_Redesign_DS | SUPERSEDED | 独立 micro-surfel redesign 草案 | unknown（未单独执行） | — | MERGED into 07 Phase D |
| 09 | 03_v1_redesign/09_round5_v1_architecture_freeze_spec.md | Super-LIVO_Round5_Architecture_Freeze_v1_Spec_DS | EXECUTED | 冻结决定 + v1 spec（TB-1/G-0..G-3/DG-0/S/V/L/M） | fecbdc6 | affa016 | — |
| 10 | 03_v1_redesign/10_round6_v1_publish_tickets.md | Super-LIVO_Round6_v1_ToTickets_DS | EXECUTED | v1 local tracker（.scratch/super-livo-v1/，17 items） | affa016 | affa016（no commit，tracker only） | — |
| 11 | 04_v1_implementation/11_round7_project_hygiene_dataset_registry_tb1.md | Super-LIVO_Round7_ProjectHygiene_DatasetRegistry_TB1_DS | EXECUTED | 项目整理 + dataset registry + TB-1 | affa016 | 8125e8b/6f36260/4964c7c | — |
| 12 | 04_v1_implementation/12_round8_g0_g3_micro_surfel_feasibility.md | Super-LIVO_Round8_G0_G3_MicroSurfel_Feasibility_DS | EXECUTED — BLOCKED AT G-1 | G-0..G-3 micro-surfel shadow feasibility；G-1 NO-GO 后按规则停止 | 4964c7c | 8f355a1（G-0）→ a7d7313（G-1 NO-GO） | G-1 NO-GO → G-1R（Round 9） |
| 13 | 04_v1_implementation/13_round9_g1r_support_scale_corrective.md | Super-LIVO_Round9_G1R_ParentSupport_Corrective_DS | ACTIVE | Evaluate parent-scale plane support after 0.25m child-plane NO-GO, then continue G-2/G-3 shadow evidence if viable | a7d7313 | 本轮产出 | — | — |

## 说明

- Status 词汇：EXECUTED / SUPERSEDED / MERGED / ACTIVE / REFERENCE。
- Input/Output HEAD：凡无法从 prompt 文本或 git log 确认的填 `unknown`；`no commit` 表示该轮只产出 local tracker/docs 未推送的 commit。
- 当前 active workflow：`04_v1_implementation/`（v1 tracer graph：TB-1 → G-0 → … → M-0，见 `docs/super_livo/specs/super_livo_v1_spec.md` 与 `.scratch/super-livo-v1/issues/`）。
- Round 4 与 08 均被 07（offline-first recovery）合并替代；07 的 Phase D 再经 Round 5 固化为 v1 冻结。