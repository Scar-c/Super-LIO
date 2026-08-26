#!/usr/bin/env python3
"""Deterministic MCD Day10 ground-truth preparation.

Accepted MCD semantics (Round11X/W):
  - source: official pose_inW.csv (num,t,x,y,z,qx,qy,qz,qw)
  - Body = VN100 (IMU); GT pose is W_T_B
  - no prism / lever-arm compensation
  - no scale
  - output: TUM lines  timestamp tx ty tz qx qy qz qw
Output is byte-deterministic for a given input file.
"""
import argparse
import csv
import pathlib
import sys


def convert(path_in, path_out):
    rows = 0
    first = None
    last = None
    with open(path_in, newline="", encoding="utf-8") as fin:
        reader = csv.DictReader(fin)
        if reader.fieldnames is None or "t" not in reader.fieldnames:
            raise ValueError("pose_inW.csv must have a 't' column")
        lines = []
        for record in reader:
            stamp = record["t"].strip()
            values = [record[k].strip() for k in
                      ("x", "y", "z", "qx", "qy", "qz", "qw")]
            if any(not v for v in (stamp,) + tuple(values)):
                raise ValueError("empty field in pose_inW row")
            timestamp = float(stamp)
            if first is None:
                first = timestamp
            last = timestamp
            rows += 1
            # %.9f matches the historical Round11W-X GT artifact byte-for-byte
            lines.append("%.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f\n" % (
                float(stamp), float(values[0]), float(values[1]),
                float(values[2]), float(values[3]), float(values[4]),
                float(values[5]), float(values[6])))
    with open(path_out, "w", encoding="utf-8") as fout:
        fout.writelines(lines)
    return rows, first, last


def main(argv=None):
    parser = argparse.ArgumentParser(description="MCD Day10 GT -> TUM")
    parser.add_argument("--csv", required=True, help="pose_inW.csv path")
    parser.add_argument("--out", required=True, help="TUM output path")
    args = parser.parse_args(argv)
    rows, first, last = convert(args.csv, args.out)
    print("rows=%d first=%.9f last=%.9f" % (rows, first, last))
    return 0


if __name__ == "__main__":
    sys.exit(main())
