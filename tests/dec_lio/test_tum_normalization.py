import importlib.util
import pathlib
import unittest


class TumNormalizationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path = pathlib.Path(__file__).resolve().parents[2] / "eval/dec_lio/normalize_tum_timestamps.py"
        spec = importlib.util.spec_from_file_location("normalize_tum_timestamps", path)
        cls.module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.module)

    def test_keeps_rows_and_makes_serialized_stamps_strict(self):
        lines = [
            "1700000000.0 0 0 0 0 0 0 1",
            "1700000000.0000001 1 0 0 0 0 0 1",
            "1700000000.0000001 2 0 0 0 0 0 1",
            "1700000001.0 3 0 0 0 0 0 1",
        ]
        result = self.module.normalize_lines(lines)
        stamps = [float(line.split()[0]) for line in result]
        self.assertEqual(len(result), len(lines))
        self.assertTrue(all(a < b for a, b in zip(stamps, stamps[1:])))
        self.assertEqual([line.split()[1] for line in result], ["0", "1", "2", "3"])


if __name__ == "__main__":
    unittest.main()
