import sys
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parents[2] / "tools" / "dec_lio"
sys.path.insert(0, str(TOOLS))

from prompt07r_semantics import run_synthetic_tests  # noqa: E402


class Prompt07RPreprocessingTests(unittest.TestCase):
    def test_synthetic_contract(self):
        results = run_synthetic_tests()
        self.assertTrue(all(results.values()), results)


if __name__ == "__main__":
    unittest.main()
