#!/usr/bin/env python3
"""Static native-boundary and ancestry guard."""

import pathlib
import subprocess
import tempfile
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
        changed = set(result.stdout.splitlines()) if result.stdout.strip() else set()
        allowed = {
            "src/super_lio/include/dec_lio/D3Solver.h",
            "src/super_lio/src/dec_lio/D3Solver.cpp",
            "src/super_lio/include/lio/ESKF.h",
            "src/super_lio/src/lio/ESKF.cpp",
            "src/super_lio/include/lio/params.h",
            "src/super_lio/include/lio/point_selection.h",
            "src/super_lio/include/lio/super_lio.h",
            "src/super_lio/src/lio/params.cpp",
            "src/super_lio/src/lio/super_lio.cpp",
            "src/super_lio/src/ros/ROSWrapper.cpp",
        }
        self.assertTrue(changed <= allowed, sorted(changed - allowed))

    def test_wrong_ancestry_is_rejected(self):
        repo = pathlib.Path(__file__).resolve().parents[2]
        script = repo / "tools/dec_lio/production_identity.py"
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory)
            subprocess.run(["git", "init", "-q", str(path)], check=True)
            subprocess.run(["git", "-C", str(path), "config", "user.email", "test@example.invalid"], check=True)
            subprocess.run(["git", "-C", str(path), "config", "user.name", "test"], check=True)
            (path / "base.txt").write_text("base\n", encoding="utf-8")
            subprocess.run(["git", "-C", str(path), "add", "base.txt"], check=True)
            subprocess.run(["git", "-C", str(path), "commit", "-q", "-m", "base"], check=True)
            base = subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()
            subprocess.run(["git", "-C", str(path), "update-ref", "refs/remotes/origin/ros1", base], check=True)
            empty_tree = subprocess.check_output(["git", "-C", str(path), "mktree"], input=b"", text=False).strip()
            unrelated = subprocess.check_output(
                ["git", "-C", str(path), "commit-tree", empty_tree, "-m", "unrelated"], text=True
            ).strip()
            subprocess.run(["git", "-C", str(path), "update-ref", "refs/heads/master", unrelated], check=True)
            subprocess.run(["git", "-C", str(path), "symbolic-ref", "HEAD", "refs/heads/master"], check=True)
            result = subprocess.run(["python3", str(script), "--repo-root", str(path)], text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)

    def test_prob_lio_estimator_tokens_are_absent(self):
        repo = pathlib.Path(__file__).resolve().parents[2]
        production = repo / "src/super_lio"
        dcreg_pcg_paths = {
            "src/super_lio/include/dec_lio/AsymmetricEstimator.h",
            "src/super_lio/src/dec_lio/AsymmetricEstimator.cpp",
            "src/super_lio/include/dec_lio/DCRegCoreSolver.h",
            "src/super_lio/src/dec_lio/DCRegCoreSolver.cpp",
        }
        for path in production.rglob("*"):
            if not path.is_file():
                continue
            text = path.read_text(encoding="utf-8", errors="replace").lower()
            self.assertNotIn("prob_lio", text, str(path))
            self.assertNotIn("sa_gate", text, str(path))
            relative = path.relative_to(repo).as_posix()
            if relative not in {
                "src/super_lio/include/dec_lio/D3Solver.h",
                "src/super_lio/src/dec_lio/D3Solver.cpp",
            } and relative not in dcreg_pcg_paths:
                self.assertNotIn("pcg", text, str(path))


if __name__ == "__main__":
    unittest.main()
