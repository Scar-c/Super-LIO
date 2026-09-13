#!/usr/bin/env python3
"""Validate the exact Prompt00R GEODE input triplet by SHA256."""

import argparse
import hashlib
import pathlib
import sys


EXPECTED = {
    "bag": "1fb14937289172c1fa694a817430c142568b205a1535e16a2d0406f7c471b7b6",
    "config": "1f039b0c70b4d7a2a63d420587b8e05ff4653f0385d2aa95db1eb7ccbfaa3558",
    "ground_truth": "0b68151c9c4c9a1978bf2d8fdef8294d22cff5f53b8f62f401a7ab9e11ce5e39",
}


def sha256(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main(argv=None):
    parser = argparse.ArgumentParser()
    for name in EXPECTED:
        parser.add_argument(f"--{name.replace('_', '-')}", required=True)
    args = parser.parse_args(argv)
    errors = []
    for name, expected in EXPECTED.items():
        path = getattr(args, name)
        try:
            actual = sha256(path)
        except OSError as error:
            errors.append(f"{name}: {error}")
            continue
        print(f"{name}: {path} sha256={actual}")
        if actual != expected:
            errors.append(f"{name}: expected {expected}, got {actual}")
    if errors:
        print("INPUT_FAIL: " + "; ".join(errors), file=sys.stderr)
        return 1
    print("INPUT_PASS: exact GEODE Bridge01 Alpha triplet")
    return 0


if __name__ == "__main__":
    sys.exit(main())
