#!/usr/bin/env python3
"""Check that Dec-LIO does not track runtime data or Prob-LIO estimator code."""

import pathlib
import subprocess
import sys


def main():
    repo = pathlib.Path(__file__).resolve().parents[2]
    tracked = subprocess.check_output(["git", "-C", str(repo), "ls-files"], text=True).splitlines()
    bad_suffixes = (".bag", ".db3", ".tum", ".pcd", ".log")
    errors = []
    for name in tracked:
        lowered = name.lower()
        if lowered.endswith(bad_suffixes):
            errors.append(f"tracked runtime artifact: {name}")
        if name.startswith("src/super_lio/") and any(
            token in lowered for token in ("prob_lio", "dcreg", "pcg", "sa_gate")
        ):
            errors.append(f"forbidden production estimator name: {name}")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"HYGIENE_PASS: {len(tracked)} tracked files contain no forbidden runtime artifacts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
