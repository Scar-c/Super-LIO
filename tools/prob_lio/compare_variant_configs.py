#!/usr/bin/env python3
"""Compare effective ROS parameter snapshots for one declared A/B axis."""

import argparse
import pathlib
import sys

import yaml


def flatten(value, prefix=()):
    if isinstance(value, dict):
        result = {}
        for key, child in value.items():
            result.update(flatten(child, prefix + (str(key),)))
        return result
    return {"/".join(prefix): value}


def load(path):
    document = yaml.safe_load(pathlib.Path(path).read_text(encoding="utf-8"))
    if not isinstance(document, dict):
        raise ValueError(f"{path}: expected YAML mapping")
    flat = flatten(document)
    return {
        key.lstrip("/"): value
        for key, value in flat.items()
        if key.lstrip("/").startswith("lio/prob_lio/")
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--left", required=True)
    parser.add_argument("--right", required=True)
    parser.add_argument("--allowed-key", action="append", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args(argv)
    try:
        left = load(args.left)
        right = load(args.right)
        keys = sorted(set(left) | set(right))
        differences = [
            {
                "key": key,
                "left": left.get(key),
                "right": right.get(key),
            }
            for key in keys
            if left.get(key) != right.get(key)
        ]
        allowed = {key.lstrip("/").replace(".", "/") for key in args.allowed_key}
        actual = {item["key"] for item in differences}
        passed = actual == allowed
        result = {
            "schema_version": 1,
            "left": str(pathlib.Path(args.left).resolve()),
            "right": str(pathlib.Path(args.right).resolve()),
            "allowed_keys": sorted(allowed),
            "differences": differences,
            "status": "PASS" if passed else "INVALID",
        }
        pathlib.Path(args.out).write_text(
            yaml.safe_dump(result, sort_keys=False), encoding="utf-8"
        )
        if not passed:
            print(
                f"INVALID: expected {sorted(allowed)}, got {sorted(actual)}",
                file=sys.stderr,
            )
            return 1
        print(f"PASS: exact differences {sorted(actual)}")
        return 0
    except (OSError, ValueError, yaml.YAMLError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())


