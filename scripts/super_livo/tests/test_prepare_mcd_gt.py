#!/usr/bin/env python3
"""prepare_mcd_gt synthetic TDD: deterministic TUM conversion."""
import csv
import pathlib
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "super_livo/evaluation"))

from prepare_mcd_gt import convert


def main():
    with tempfile.TemporaryDirectory(prefix="mcdgt-") as tmp:
        tmp = pathlib.Path(tmp)
        csv_path = tmp / "pose_inW.csv"
        with open(csv_path, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=["num", "t", "x", "y", "z",
                                              "qx", "qy", "qz", "qw"])
            w.writeheader()
            w.writerow({"num": "11", "t": "1645008761.153256178",
                        "x": "39.916532928019564", "y": "23.052257138316396",
                        "z": "7.299254388426964", "qx": "0.886432211042536",
                        "qy": "0.292027951287205", "qz": "0.156779604991748",
                        "qw": "0.323075480889327"})
            w.writerow({"num": "12", "t": "1645008761.251856327",
                        "x": "39.928504719071540", "y": "23.041861674393051",
                        "z": "7.286279849024381", "qx": "0.897067873569930",
                        "qy": "0.295160958602358", "qz": "0.150832903890918",
                        "qw": "0.292230514884218"})
        out = tmp / "gt.tum"
        rows, first, last = convert(str(csv_path), str(out))
        assert rows == 2, rows
        assert abs(first - 1645008761.153256178) < 1e-9
        assert abs(last - 1645008761.251856327) < 1e-9
        text = out.read_text()
        lines = text.splitlines()
        assert len(lines) == 2
        fields = lines[0].split()
        assert len(fields) == 8
        assert fields[0] == "1645008761.153256178"
        assert fields[1] == "39.916532928"
        assert fields[7] == "0.323075481"
        # deterministic: same input -> identical bytes
        out2 = tmp / "gt2.tum"
        convert(str(csv_path), str(out2))
        assert out.read_bytes() == out2.read_bytes()
    print("PREPARE MCD GT TDD: ALL PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
