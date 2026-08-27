#!/usr/bin/env python3
"""Convert FAST-LIVO2 20-column debug pose output to TUM trajectory."""
import argparse
import math
import numpy as np
from scipy.spatial.transform import Rotation

def convert(src, dst, epoch):
    data = np.atleast_2d(np.loadtxt(src))
    if data.shape[1] != 20:
        raise ValueError(f"expected 20 columns, got {data.shape[1]}")
    if not np.all(np.isfinite(data)) or np.any(np.diff(data[:, 0]) < 0):
        raise ValueError("non-finite or non-monotonic FAST-LIVO2 output")
    quat = Rotation.from_euler("xyz", data[:, 1:4], degrees=True).as_quat()
    tum = np.column_stack((epoch + data[:, 0], data[:, 4:7], quat))
    np.savetxt(dst, tum, fmt="%.9f")

def main():
    p=argparse.ArgumentParser();p.add_argument("--input",required=True);p.add_argument("--output",required=True);p.add_argument("--epoch",required=True,type=float);a=p.parse_args()
    convert(a.input,a.output,a.epoch)

if __name__ == "__main__": main()
