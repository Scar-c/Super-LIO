#!/usr/bin/env python3
"""Static native-boundary and ancestry guard."""

import pathlib
import subprocess
import unittest


class NativeIdentityTest(unittest.TestCase):
    def test_identity_script_passes(self):
        repo = pathlib.Path(__file__).resolve().parents[2]
        script = repo / "tools/dec_lio/production_identity.py"
        result = subprocess.run(
            ["python3", str(script), "--repo-root", str(repo)],
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("IDENTITY_PASS", result.stdout)

    def test_no_estimator_core_diff(self):
        repo = pathlib.Path(__file__).resolve().parents[2]
        result = subprocess.run(
            ["git", "-C", str(repo), "diff", "--name-only", "origin/ros1", "--",
             "src/super_lio/src/lio", "src/super_lio/include/lio"],
            text=True,
            capture_output=True,
            check=True,
        )
        self.assertEqual(result.stdout.strip(), "")


if __name__ == "__main__":
    unittest.main()
