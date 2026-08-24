# Super-LIVO Document Conventions

所有较重要的设计/实验文档统一至少包含以下 frontmatter 与章节。不要求机械填满所有章节，但不得出现"不知道 draft/frozen、是否 superseded、对应哪个 commit"。

## 文档头（frontmatter 区块）

```markdown
Status:          draft | accepted | frozen | superseded | reference | historical
Scope:           <一句话范围>
Source of Truth: <本文档的上位依据，如 architecture_owner_decisions.md>
Related commits: <hash(es)>
Supersedes:      <旧文档/旧结论>
Superseded by:   <新文档/空>
Datasets:        <涉及数据集或 N/A>
Last updated:    <date>
```

## 章节模板

```markdown
## Purpose
## Current facts / evidence
## Decisions
## Interfaces / semantics
## Tests / gates
## Open decisions
## Outputs / artifacts
## Change log
```

- `Decisions`：只写已定内容；未定内容进 `Open decisions`。
- 数值/阈值：标注分类（P-A baseline inherited / P-B reference inherited / P-C new sweep candidate / P-D architecture constant），见 `parameters/parameter_policy.md`。
- 转换矩阵：一律 `p_A = T_A_B * p_B`（见 `datasets/calibration_time_sync.md`）。
- 数据集引用：一律用 `datasets/dataset_registry.md` 的 sequence ID；禁止自造 bag 名。

## 状态词

```text
draft      设计未冻结，可改
accepted   已采纳，未实施或实施中
frozen     Architecture Owner 冻结，不得单方面改
superseded 被新文档替代（须写明 Superseded by）
reference  事实记录（考古/审计/恢复报告），不随设计变化
historical 已过时但保留
```

## 命名

- 文件名：`snake_case.md`；prompt 文件：`NN_short_snake_case_description.md`（见 prompts/README.md）。
- 禁止文件名使用空格 / em dash / 超长自然语言。