#!/usr/bin/env python3
"""Mandatory raw trajectory parity gate for Prompt00R."""

import argparse
import hashlib
import pathlib
import sys


def digest(path):
    data = pathlib.Path(path).read_bytes()
    return len(data), hashlib.sha256(data).hexdigest(), data


def validate(path):
    size, sha, data = digest(path)
    if not data:
        raise ValueError(f"{path}: empty trajectory")
    rows = data.splitlines()
    if len(rows) < 3:
        raise ValueError(f"{path}: partial trajectory ({len(rows)} rows)")
    previous = None
    for line_number, raw in enumerate(rows, 1):
        fields = raw.split()
        if len(fields) != 8:
            raise ValueError(f"{path}:{line_number}: expected exactly 8 fields")
        timestamp = float(fields[0])
        if previous is not None and timestamp <= previous:
            raise ValueError(f"{path}:{line_number}: timestamps not increasing")
        previous = timestamp
    return size, sha


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("trajectories", nargs="+", type=pathlib.Path)
    args = parser.parse_args(argv)
    if len(args.trajectories) < 2:
        parser.error("at least two trajectories are required")
    try:
        facts = [(path, *validate(path)) for path in args.trajectories]
    except (OSError, ValueError) as error:
        print(f"PARITY_FAIL: {error}", file=sys.stderr)
        return 2
    authority = facts[0][2]
    print(f"authority={facts[0][0]} bytes={facts[0][1]} sha256={authority}")
    passed = True
    for path, size, sha in facts[1:]:
        same = size == facts[0][1] and sha == authority
        print(f"compare={path} bytes={size} sha256={sha} cmp={'PASS' if same else 'FAIL'}")
        passed = passed and same
    if not passed:
        print("PARITY_FAIL: raw trajectory bytes differ", file=sys.stderr)
        return 1
    print(f"PARITY_PASS: {len(facts)} non-empty trajectories are byte-identical")
    return 0


if __name__ == "__main__":
    sys.exit(main())
