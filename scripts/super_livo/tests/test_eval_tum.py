#!/usr/bin/env python3
"""CLI contract tests for the persistent SE(3) translation evaluator."""
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[3]
EVALUATOR = ROOT / "scripts/super_livo/evaluation/eval_tum_translation.py"


def write_tum(path, rows):
    with path.open("w", encoding="utf-8") as stream:
        for timestamp, x, y, z in rows:
            stream.write(
                f"{timestamp:.9f} {x:.9f} {y:.9f} {z:.9f} 0 0 0 1\n"
            )


def run_eval(est, gt, *extra):
    return subprocess.run(
        [sys.executable, str(EVALUATOR), str(est), str(gt), *extra],
        cwd=str(ROOT),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )


def require(condition, message, output=""):
    if not condition:
        raise AssertionError(f"{message}\n{output}")


def main():
    with tempfile.TemporaryDirectory(prefix="round11t-eval-") as tmp:
        tmp = pathlib.Path(tmp)
        est = tmp / "est.tum"
        gt = tmp / "gt.tum"
        out = tmp / "metrics.txt"

        # One GT row is deliberately outside max_diff. Its extreme position must
        # not contaminate alignment or error statistics.
        write_tum(est, [
            (0.00, 0, 0, 0),
            (1.00, 1, 0, 0),
            (2.00, 2, 1, 0),
            (3.00, 3, 1, 1),
        ])
        write_tum(gt, [
            (0.01, 10, -2, 3),
            (1.02, 11, -2, 3),
            (2.20, 999, 999, 999),
            (3.01, 13, -1, 4),
        ])
        result = run_eval(
            est,
            gt,
            "--frame", "VN100",
            "--max-diff", "0.05",
            "--min-matches", "3",
            "--out", str(out),
        )
        require(result.returncode == 0, "valid evaluation failed", result.stdout)
        for token in (
            "git HEAD:",
            "script path:",
            "arguments:",
            "comparison frame: VN100",
            "alignment type: SE(3), no scale",
            "association max_diff: 0.050000 s",
            "matched count: 3",
            "RMSE=0.0000",
            "mean=0.0000",
            "median=0.0000",
            "max=0.0000",
            "P90=0.0000",
            "P95=0.0000",
        ):
            require(token in result.stdout, f"missing output token {token!r}", result.stdout)
        require(out.exists(), "--out metrics file not written", result.stdout)

        # SE(3) only: a scale mismatch must remain visible after alignment.
        scaled = tmp / "scaled.tum"
        write_tum(scaled, [
            (0.00, 0.0, 0.0, 0.0),
            (1.00, 1.1, 0.0, 0.0),
            (2.00, 2.2, 1.1, 0.0),
            (3.00, 3.3, 1.1, 1.1),
        ])
        scale_result = run_eval(
            est, scaled, "--max-diff", "0.001", "--min-matches", "4"
        )
        require(scale_result.returncode == 0, "scale evaluation failed", scale_result.stdout)
        require("RMSE=0.0000" not in scale_result.stdout,
                "scale was incorrectly absorbed", scale_result.stdout)

        # No temporal association must fail closed with an explicit message.
        no_match = tmp / "no_match.tum"
        write_tum(no_match, [(100.0 + i, i, 0, 0) for i in range(4)])
        no_match_result = run_eval(
            est, no_match, "--max-diff", "0.01", "--min-matches", "3"
        )
        require(no_match_result.returncode != 0,
                "no-match evaluation did not fail closed", no_match_result.stdout)
        require("insufficient matches" in no_match_result.stdout.lower(),
                "no-match failure is not explicit", no_match_result.stdout)

    print("EVAL CLI TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
