#!/usr/bin/env python3
"""Wrong-dataset adversarial test for the exact input gate."""

import pathlib
import subprocess
import tempfile
import unittest


class InputIdentityTest(unittest.TestCase):
    def test_wrong_dataset_is_rejected(self):
        repo = pathlib.Path(__file__).resolve().parents[2]
        script = repo / "tools/dec_lio/validate_input.py"
        with tempfile.TemporaryDirectory() as directory:
            paths = [pathlib.Path(directory) / name for name in ("bag", "config", "ground_truth")]
            for path in paths:
                path.write_bytes(b"wrong dataset\n")
            result = subprocess.run(
                ["python3", str(script), "--bag", str(paths[0]), "--config", str(paths[1]),
                 "--ground-truth", str(paths[2])], text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("INPUT_FAIL", result.stderr)


if __name__ == "__main__":
    unittest.main()
