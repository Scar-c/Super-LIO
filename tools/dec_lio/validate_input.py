#!/usr/bin/env python3
"""Validate exact local GEODE input identities by SHA256."""

import argparse
import hashlib
import pathlib
import sys


EXPECTED = {
    "bridge01": {
        "bag": "1fb14937289172c1fa694a817430c142568b205a1535e16a2d0406f7c471b7b6",
        "config": "1f039b0c70b4d7a2a63d420587b8e05ff4653f0385d2aa95db1eb7ccbfaa3558",
        "ground_truth": "0b68151c9c4c9a1978bf2d8fdef8294d22cff5f53b8f62f401a7ab9e11ce5e39",
    },
    "stairs_alpha": {
        "bag": "71cea6a30573ac6144776d873a6232707c04ee977a6a59e63bfdd2bce42fedb1",
        "config": "12d22a80abd21050d6e7b4984edd51f09f2e65d46bcaadb853a4c75a04a97d3b",
        "ground_truth": "94b2cb2d9e3f4e4bbfde0932ed3adf6b314b649b690c9e13d6f610c719d750f6",
    },
}


def sha256(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--sequence", choices=sorted(EXPECTED), default="bridge01")
    for name in ("bag", "config", "ground_truth"):
        parser.add_argument(f"--{name.replace('_', '-')}", required=True)
    args = parser.parse_args(argv)
    expected = EXPECTED[args.sequence]
    errors = []
    for name, expected_sha in expected.items():
        path = getattr(args, name)
        try:
            actual = sha256(path)
        except OSError as error:
            errors.append(f"{name}: {error}")
            continue
        print(f"{name}: {path} sha256={actual}")
        if expected_sha.startswith("__"):
            errors.append(f"{name}: validator configuration is incomplete")
        elif actual != expected_sha:
            errors.append(f"{name}: expected {expected_sha}, got {actual}")
    if errors:
        print("INPUT_FAIL: " + "; ".join(errors), file=sys.stderr)
        return 1
    print(f"INPUT_PASS: exact GEODE {args.sequence} input triplet")
    return 0


if __name__ == "__main__":
    sys.exit(main())
