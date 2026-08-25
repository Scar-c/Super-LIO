# Super-LIVO Execution Hygiene

Round 11H（Owner Round11H §1.8）起强制执行。

- 一次 shell 调用只做一个 bounded operation（build / test / run / post-process 分开）。
- 每个重要命令暴露真实 rc：`set -o pipefail; set +e; ... ; rc=${PIPESTATUS[0]}; echo "__DONE_RC=$rc__"; exit $rc`。
- assert/SIGABRT（rc≈134）= COMPLETED FAIL，不是 hang。
- 重跑前先查真实进程：`pgrep -af '<proc>'`；进程已退出且有结果 → DO NOT RERUN。
- 禁止 `cmd | grep` 吞掉 rc；build 只跑一次，grep 在单独调用。
- cleanup 只杀自己启动的进程（runner trap EXIT INT TERM，仅自启 roscore/node，保留原始 rc）。
- 禁止 `pkill roscore`/`killall` 等宽泛清理。
- 禁交互/pager（GIT_PAGER=cat, PAGER=cat）。
- 用户中断 OpenCode spinner 不使已完成 evidence 失效（先查进程再判）。
- 不 background 实验进程（除非 wrapper 记录 PID + trap + 保留 rc）。
