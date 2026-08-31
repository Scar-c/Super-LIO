#!/usr/bin/env python3
"""Deterministic MCD pose_inW.csv to TUM conversion.

This adapter is used for the audited MCD day10 and night08 sequences. The
verified contract is: Body = VN100 IMU, CSV pose is W_T_B, no prism/lever-arm
compensation, and no scale alignment.
"""
import argparse
import csv
import math
import pathlib
import sys


REQUIRED_FIELDS = ("t", "x", "y", "z", "qx", "qy", "qz", "qw")


def convert(path_in, path_out):
    rows = 0
    first = None
    last = None
    lines = []
    with open(path_in, newline="", encoding="utf-8") as fin:
        reader = csv.DictReader(fin)
        if reader.fieldnames is None or any(
            field not in reader.fieldnames for field in REQUIRED_FIELDS
        ):
            raise ValueError(
                "pose_inW.csv must contain t,x,y,z,qx,qy,qz,qw columns"
            )
        for record in reader:
            values = [record[field].strip() for field in REQUIRED_FIELDS]
            if any(not value for value in values):
                raise ValueError("empty field in pose_inW row")
            timestamp = float(values[0])
            if not math.isfinite(timestamp):
                raise ValueError("non-finite timestamp in pose_inW.csv")
            if last is not None and timestamp <= last:
                raise ValueError(
                    "pose_inW.csv timestamps must be strictly increasing"
                )
            quaternion = [float(value) for value in values[4:8]]
            if not all(math.isfinite(value) for value in quaternion):
                raise ValueError("non-finite quaternion in pose_inW.csv")
            if math.sqrt(sum(value * value for value in quaternion)) <= 0.0:
                raise ValueError("zero quaternion in pose_inW.csv")
            first = timestamp if first is None else first
            last = timestamp
            rows += 1
            # Nine decimal places matches the existing TUM adapters.
            lines.append(
                "%.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f\n"
                % tuple(float(value) for value in values)
            )
    if rows == 0:
        raise ValueError("pose_inW.csv contains no data rows")
    pathlib.Path(path_out).write_text("".join(lines), encoding="utf-8")
    return rows, first, last


def main(argv=None):
    parser = argparse.ArgumentParser(description="MCD pose_inW.csv -> TUM")
    parser.add_argument("--csv", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args(argv)
    try:
        rows, first, last = convert(args.csv, args.out)
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2
    print(f"rows={rows} first={first:.9f} last={last:.9f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

