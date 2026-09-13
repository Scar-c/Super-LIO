#!/usr/bin/env python3
"""Adversarial tests for the raw-byte parity gate."""

import pathlib
import tempfile
import unittest

from eval.dec_lio.exact_parity import validate


class ExactParityTest(unittest.TestCase):
    def write(self, directory, name, content):
        path = pathlib.Path(directory) / name
        path.write_bytes(content)
        return path

    def test_valid_and_single_byte_mutation(self):
        with tempfile.TemporaryDirectory() as directory:
            good = self.write(directory, "good.tum", b"1 0 0 0 0 0 0 1\n2 1 0 0 0 0 0 1\n3 2 0 0 0 0 0 1\n")
            bad = self.write(directory, "bad.tum", b"1 0 0 0 0 0 0 1\n2 1 0 0 0 0 0 0\n3 2 0 0 0 0 0 1\n")
            self.assertEqual(validate(good)[0], good.stat().st_size)
            self.assertNotEqual(validate(good)[1], validate(bad)[1])

    def test_reorder_and_partial_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            reordered = self.write(directory, "reordered.tum", b"2 1 0 0 0 0 0 1\n1 0 0 0 0 0 0 1\n")
            partial = self.write(directory, "partial.tum", b"1 0 0 0 0 0 0 1\n2 1 0 0 0 0 0 1\n")
            with self.assertRaises(ValueError):
                validate(reordered)
            with self.assertRaises(ValueError):
                validate(partial)


if __name__ == "__main__":
    unittest.main()
