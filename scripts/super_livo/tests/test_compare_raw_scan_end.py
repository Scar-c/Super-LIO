#!/usr/bin/env python3
"""Round 11X raw-scan-end state comparator contract test."""
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[3]
SCRIPT = ROOT / "scripts/super_livo/evaluation/compare_raw_scan_end.py"


def write_row(path, px, qz, qw, vx, p00):
    header = ["scan_end", "state_time", "px", "py", "pz", "qx", "qy",
              "qz", "qw", "vx", "vy", "vz"]
    header += [f"P{r}_{c}" for r in range(18) for c in range(18)]
    values = [1.0, 1.0, px, 0, 0, 0, 0, qz, qw, vx, 0, 0]
    covariance = [0.0] * (18 * 18)
    covariance[0] = p00
    values += covariance
    path.write_text(",".join(header) + "\n" +
                    ",".join(str(v) for v in values) + "\n")


def main():
    with tempfile.TemporaryDirectory(prefix="round11x-scan-end-") as tmp:
        tmp = pathlib.Path(tmp)
        baseline = tmp / "b.csv"
        candidate = tmp / "c.csv"
        write_row(baseline, 0.0, 0.0, 1.0, 0.0, 1.0)
        write_row(candidate, 3.0, 1.0, 0.0, 4.0, 6.0)
        result = subprocess.run(
            [sys.executable, str(SCRIPT), str(baseline), str(candidate)],
            cwd=str(ROOT), text=True, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, check=False)
        assert result.returncode == 0, result.stdout
        for token in (
                "matched raw scan ends: 1",
                "position diff (m) P50/P90/P99/max: 3/3/3/3",
                "rotation diff (rad) P50/P90/P99/max: 3.14159",
                "velocity diff (m/s) P50/P90/P99/max: 4/4/4/4",
                "covariance Frobenius diff P50/P90/P99/max: 5/5/5/5"):
            assert token in result.stdout, (token, result.stdout)
    print("RAW SCAN END COMPARATOR TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
