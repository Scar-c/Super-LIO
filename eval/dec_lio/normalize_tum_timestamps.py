#!/usr/bin/env python3
"""Make a native TUM trajectory strictly increasing at serialization time."""

import argparse
import pathlib
import struct


def next_up(value):
    """Return the next representable IEEE-754 double above a finite value."""
    bits = struct.unpack(">Q", struct.pack(">d", value))[0]
    bits += 1 if value >= 0.0 else -1
    return struct.unpack(">d", struct.pack(">Q", bits))[0]


def normalize_lines(lines):
    output = []
    last_stamp = None
    for line_number, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped:
            continue
        fields = stripped.split()
        if len(fields) != 8:
            raise ValueError(f"line {line_number}: expected exactly 8 TUM fields")
        stamp = float(format(float(fields[0]), ".17g"))
        if last_stamp is not None and stamp <= last_stamp:
            stamp = next_up(last_stamp)
        fields[0] = format(stamp, ".17g")
        output.append(" ".join(fields) + "\n")
        last_stamp = stamp
    if not output:
        raise ValueError("trajectory is empty")
    return output


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=pathlib.Path)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    args = parser.parse_args(argv)
    if args.input.resolve() == args.output.resolve():
        raise ValueError("input and output must be different files")
    lines = args.input.read_text(encoding="utf-8").splitlines()
    normalized = normalize_lines(lines)
    args.output.write_text("".join(normalized), encoding="utf-8")
    print(f"rows={len(normalized)} output={args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
