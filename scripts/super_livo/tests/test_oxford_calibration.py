#!/usr/bin/env python3
"""prepare_oxford_calibration TDD: quaternion/matrix derivation checks."""
import math
import pathlib
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "super_livo/datasets"))

from prepare_oxford_calibration import (config_extrinsic_12, flatten,
                                        mat_mul, quat_to_rot, rigid_inv,
                                        to_4x4)


def main():
    ok = True

    def expect(name, cond):
        nonlocal ok
        print("%s: %s" % (name, "PASS" if cond else "FAIL"))
        ok &= cond

    # identity quaternion -> identity rotation
    I = quat_to_rot((0.0, 0.0, 0.0, 1.0))
    expect("identity quat", all(abs(I[i][j] - (1.0 if i == j else 0.0)) < 1e-12
                                for i in range(3) for j in range(3)))

    # 90 deg about z: q = (0,0,sin(45),cos(45))
    s = math.sin(math.pi / 4)
    R = quat_to_rot((0.0, 0.0, s, s))
    expect("90deg z rotation", abs(R[0][0]) < 1e-12 and abs(R[0][1] + 1.0) < 1e-12
           and abs(R[1][0] - 1.0) < 1e-12 and abs(R[2][2] - 1.0) < 1e-12)

    # rigid inverse roundtrip
    T = [[0.9999460020163569, 0.0035427429705001123, 0.009769443369467168,
          0.00035060884033447846],
         [0.003467861351881818, -0.9999645627043141, 0.007671197638838445,
          -0.079574053851818],
         [0.009796274248324188, -0.007636904334543504, -0.9999228523756403,
          -0.053826061545697405],
         [0.0, 0.0, 0.0, 1.0]]
    Tinv = rigid_inv(T)
    prod = mat_mul(T, Tinv)
    expect("rigid inverse roundtrip",
           all(abs(prod[i][j] - (1.0 if i == j else 0.0)) < 1e-9
               for i in range(4) for j in range(4)))

    # to_4x4 + flatten shape
    F = to_4x4([[1, 0, 0], [0, 1, 0], [0, 0, 1]], [1.0, 2.0, 3.0])
    flat = flatten(F)
    expect("to_4x4 flatten 16", len(flat) == 16 and abs(flat[3] - 1.0) < 1e-12
           and abs(flat[7] - 2.0) < 1e-12 and abs(flat[11] - 3.0) < 1e-12
           and abs(flat[15] - 1.0) < 1e-12)

    # config extrinsic: t first + transposed rotation (Eigen column-major)
    T = to_4x4([[0, -1, 0], [1, 0, 0], [0, 0, 1]], [0.1, 0.2, 0.3])
    v12 = config_extrinsic_12(T)
    expect("extrinsic t first", abs(v12[0] - 0.1) < 1e-12 and
           abs(v12[1] - 0.2) < 1e-12 and abs(v12[2] - 0.3) < 1e-12)
    import numpy as np
    R_parsed = np.array(v12[3:]).reshape(3, 3).T  # column-major Eigen fill
    R_intended = np.array([[0, -1, 0], [1, 0, 0], [0, 0, 1]])
    expect("extrinsic transpose convention",
           np.allclose(R_parsed, R_intended, atol=1e-12))

    print("OXFORD CALIBRATION DERIVATION TDD: %s" % ("ALL PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
