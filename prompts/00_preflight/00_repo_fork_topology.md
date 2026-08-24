# Super-LIVO Preflight — GitHub Fork 与仓库拓扑初始化

在执行 `Round 0 Source Archaeology` 之前，先完成本 Preflight。

本步骤只负责建立正确的 Git/GitHub 开发拓扑。

---

## 1. 目标拓扑

最终必须形成：

```text
Official:
Liansheng-Wang/Super-LIO
        │
        │ upstream
        ▼
Local:
Super-LIO/
        │
        │ origin
        ▼
User GitHub Fork:
<github-user>/Super-LIO
        │
        └── super-livo
```

其中：

```text
upstream = https://github.com/Liansheng-Wang/Super-LIO.git
origin   = 用户自己的 GitHub fork
```

开发分支：

```text
super-livo
```

必须从当前本地 Super-LIO 的现有 HEAD 建立。

不要擅自切换到 `main`、`master`、`ros1` 或其它分支重新开始。

---

# 2. Reference repositories 不做 fork

以下仓库严格 READ ONLY：

```text
refs/FAST-LIVO2
refs/open_vins
```

禁止：

- fork；
- push；
- 创建开发 branch；
- commit；
- 修改源码。

它们只是本项目的 reference repositories。

---

# 3. 首先检查当前状态

执行：

```bash
git -C Super-LIO status --short
git -C Super-LIO branch --show-current
git -C Super-LIO rev-parse HEAD
git -C Super-LIO remote -v
```

记录：

```text
current_branch
current_HEAD
current_remotes
working_tree_status
```

### 如果存在未提交修改

不要删除、reset、checkout 覆盖或 stash 后遗忘。

保留当前所有工作。

本 Preflight 必须做到不破坏已有工作树。

---

# 4. 检查 GitHub CLI

执行：

```bash
gh auth status
```

如果已经登录 GitHub：

继续。

如果 `gh` 不存在或尚未登录：

### STOP

不要猜用户名，不要自己改 remote 到不存在的地址。

报告：

```text
BLOCKED: GitHub CLI authentication unavailable.
```

然后停止 Preflight，等待用户完成 GitHub 登录。

不要继续 Round 0。

---

# 5. 获取当前 GitHub 用户

执行：

```bash
GH_USER="$(gh api user --jq .login)"
echo "${GH_USER}"
```

记录真实 GitHub username。

禁止根据系统用户名或目录名称猜测 GitHub username。

---

# 6. 检查 fork 是否已经存在

执行：

```bash
gh repo view "${GH_USER}/Super-LIO" \
  --json nameWithOwner,isFork,parent,url 2>/dev/null
```

分两种情况。

---

## Case A — fork 已经存在

确认：

```text
isFork = true
```

并且：

```text
parent.nameWithOwner
=
Liansheng-Wang/Super-LIO
```

如果成立：

使用现有 fork。

不要重复创建。

---

## Case B — fork 不存在

创建：

```bash
gh repo fork Liansheng-Wang/Super-LIO --clone=false
```

创建完成后再次验证：

```bash
gh repo view "${GH_USER}/Super-LIO" \
  --json nameWithOwner,isFork,parent,url
```

必须满足：

```text
nameWithOwner = <GH_USER>/Super-LIO
isFork        = true
parent        = Liansheng-Wang/Super-LIO
```

否则：

```text
FAIL
```

停止，不要继续修改 remotes。

---

# 7. 整理本地 remotes

首先检查当前 `origin` 指向哪里：

```bash
git -C Super-LIO remote get-url origin 2>/dev/null || true
```

目标必须最终是：

```text
upstream → Liansheng-Wang/Super-LIO
origin   → <GH_USER>/Super-LIO
```

---

## 7.1 官方仓库 remote

如果当前：

```text
origin
```

指向：

```text
Liansheng-Wang/Super-LIO
```

则：

```bash
git -C Super-LIO remote rename origin upstream
```

如果已经存在正确的 `upstream`：

不要重复添加。

如果没有：

```bash
git -C Super-LIO remote add upstream \
  https://github.com/Liansheng-Wang/Super-LIO.git
```

---

## 7.2 用户 fork remote

优先沿用当前机器已经可用的 GitHub transport。

如果仓库其它 GitHub remote 使用 SSH，则使用：

```bash
git -C Super-LIO remote add origin \
  "git@github.com:${GH_USER}/Super-LIO.git"
```

如果环境明显使用 HTTPS，则使用：

```bash
git -C Super-LIO remote add origin \
  "https://github.com/${GH_USER}/Super-LIO.git"
```

如果 `origin` 已存在但指向错误地址：

不要直接删除后猜测。

先记录旧值，然后修改：

```bash
git -C Super-LIO remote set-url origin <correct-fork-url>
```

---

# 8. 验证 remotes

执行：

```bash
git -C Super-LIO remote -v
```

必须满足：

```text
origin
  → <GH_USER>/Super-LIO

upstream
  → Liansheng-Wang/Super-LIO
```

然后：

```bash
git -C Super-LIO fetch upstream
git -C Super-LIO fetch origin
```

fetch 必须成功。

---

# 9. 创建开发分支

## 关键原则

开发分支：

```text
super-livo
```

必须基于 **Preflight 开始时记录的 current\_HEAD**。

不要擅自：

```text
git checkout upstream/main
git checkout upstream/master
git checkout upstream/ros1
```

然后再创建。

我们要保留当前 Super-LIO checkout 所使用的准确版本。

---

## 如果 `super-livo` 不存在

从当前 HEAD：

```bash
git -C Super-LIO switch -c super-livo
```

验证：

```bash
git -C Super-LIO rev-parse HEAD
```

必须仍然等于 Preflight 开始时记录的：

```text
current_HEAD
```

---

## 如果 `super-livo` 已存在

先确认它与当前工作是否兼容。

不得使用：

```bash
git reset --hard
```

如果切换会覆盖未提交工作：

### STOP

报告冲突，由用户决定。

不得自行处理。

---

# 10. 将开发分支推到 fork

执行：

```bash
git -C Super-LIO push -u origin super-livo
```

注意：

这一步允许推送当前已有 HEAD，即使 `super-livo` 尚无新功能 commit。

目的只是建立远端开发分支和 upstream tracking。

---

# 11. 验证 branch tracking

执行：

```bash
git -C Super-LIO status -sb
git -C Super-LIO branch -vv
```

必须看到类似：

```text
* super-livo ... [origin/super-livo]
```

---

# 12. Reference repositories 再验证一次

执行：

```bash
git -C refs/FAST-LIVO2 status --short
git -C refs/open_vins status --short
```

必须为空。

记录它们的：

```bash
git -C refs/FAST-LIVO2 rev-parse HEAD
git -C refs/open_vins rev-parse HEAD
```

不要改变它们的 remote topology。

---

# 13. 最终 Git 规则

从此项目遵守：

## Super-LIO

```text
origin/super-livo
```

是我们的开发目标。

允许：

- 修改；
- commit；
- push。

---

## upstream

```text
Liansheng-Wang/Super-LIO
```

只用于：

- fetch；
- diff；
- 查看上游更新。

禁止向 upstream push。

---

## refs/FAST-LIVO2

READ ONLY。

---

## refs/open\_vins

READ ONLY。

---

# 14. 本步骤禁止

禁止：

- 修改 Super-LIO 源码；
- 创建实现 commit；
- merge upstream；
- rebase；
- cherry-pick；
- reset；
- 修改 reference repositories；
- 创建 PR；
- 创建 release/tag；
- 改 repository visibility；
- 修改 GitHub repository settings。

本步骤只建立：

```text
fork
remotes
super-livo branch
remote tracking
```

---

# 15. Preflight Gate

只有全部满足才 PASS：

```text
P0-GIT-1
用户 fork 存在且 parent =
Liansheng-Wang/Super-LIO
```

```text
P0-GIT-2
origin =
用户 fork
```

```text
P0-GIT-3
upstream =
Liansheng-Wang/Super-LIO
```

```text
P0-GIT-4
当前 branch =
super-livo
```

```text
P0-GIT-5
super-livo 基于 Preflight 开始时的原始 HEAD，
没有无授权换 base
```

```text
P0-GIT-6
super-livo tracking origin/super-livo
```

```text
P0-GIT-7
FAST-LIVO2 / open_vins working tree clean
```

任意一项失败：

不要继续 Round 0。

---

# 16. 完成后的终端回复

只输出：

```text
Super-LIVO Git Preflight completed.

GitHub user:
<user>

Official upstream:
Liansheng-Wang/Super-LIO

Fork:
<user>/Super-LIO

Original local branch:
<branch>

Original HEAD:
<sha>

Development branch:
super-livo

Remotes:
origin   -> ...
upstream -> ...

Tracking:
super-livo -> origin/super-livo

Reference repos:
FAST-LIVO2: clean / HEAD ...
open_vins: clean / HEAD ...

Gates:
P0-GIT-1: PASS/FAIL
P0-GIT-2: PASS/FAIL
P0-GIT-3: PASS/FAIL
P0-GIT-4: PASS/FAIL
P0-GIT-5: PASS/FAIL
P0-GIT-6: PASS/FAIL
P0-GIT-7: PASS/FAIL

Next:
Proceed to Round 0 Source Archaeology only if all gates PASS.
```

全部 PASS 后，立即继续执行此前提供的：

```text
Super-LIVO Round 0 — Source Archaeology & Architecture Evidence PackScar-c/Super-LIO
```
