#!/usr/bin/env python3
"""P0/P1 two-layer trajectory parity contract.

Given two trajectories (each a TUM file produced by the production offline
node, or a rosbag with --topic /lio/odom), this tool:

1. Canonicalizes both inputs to TUM with the production serialization
   contract (%.17g, identical to the offline node's std::setprecision(17)
   writer), restricted to the intersection time window.
2. Reports PARITY-BYTE: cmp/hash of the canonicalized windowed files.
3. Reports PARITY-NUMERIC: row count, timestamp sequence, max/RMS translation
   difference, and max quaternion angle difference (quaternions are
   normalized before the angle comparison because the odom quaternion is
   emitted unnormalized; ~1e-8 rad diagnostics do NOT negate byte parity when
   file hashes are equal).

Usage:
  extract_and_compare.py <trajA> <trajB> [--topic /lio/odom] [--out prefix]
"""
import argparse
import hashlib
import math
import os
import sys

DEFAULT_TOPIC = "/lio/odom"


def canonical_line(stamp, xyz, quat):
    return " ".join(f"{v:.17g}" for v in (stamp, *xyz, *quat)) + "\n"


def canonicalize_tum(path):
    rows = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) < 8:
                continue
            rows.append([float(v) for v in parts[:8]])
    return rows


def load_bag(path, topic):
    import rosbag
    rows = []
    with rosbag.Bag(path, "r") as bag:
        for _, msg, _ in bag.read_messages(topics=[topic]):
            stamp = msg.header.stamp.to_sec()
            p = msg.pose.pose.position
            o = msg.pose.pose.orientation
            rows.append([stamp, p.x, p.y, p.z, o.x, o.y, o.z, o.w])
    return rows


def load(path, topic):
    if path.endswith(".tum"):
        return canonicalize_tum(path)
    return load_bag(path, topic)


def write_tum(rows, path):
    with open(path, "w") as f:
        for r in rows:
            f.write(canonical_line(r[0], r[1:4], r[4:8]))


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def quat_angle(qa, qb):
    na = math.sqrt(sum(x * x for x in qa))
    nb = math.sqrt(sum(x * x for x in qb))
    if na == 0.0 or nb == 0.0:
        return float("nan")
    dot = abs(sum(a * b for a, b in zip(qa, qb))) / (na * nb)
    return 2.0 * math.acos(min(1.0, max(-1.0, dot)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("traj_a")
    ap.add_argument("traj_b")
    ap.add_argument("--topic", default=DEFAULT_TOPIC)
    ap.add_argument("--out", help="write canonicalized windowed TUM files "
                                  "and report to <prefix>.a.tum/.b.tum/.report.txt")
    args = ap.parse_args()

    a = load(args.traj_a, args.topic)
    b = load(args.traj_b, args.topic)
    a.sort(key=lambda r: r[0])
    b.sort(key=lambda r: r[0])
    if not a or not b:
        print("ERROR: empty trajectory")
        sys.exit(2)
    for name, rows in (("A", a), ("B", b)):
        for i in range(1, len(rows)):
            if rows[i][0] <= rows[i - 1][0]:
                print(f"ERROR: {name} timestamps not strictly increasing "
                      f"at row {i}")
                sys.exit(2)

    # Window to the intersection of the two stamp ranges.
    t0 = max(a[0][0], b[0][0])
    t1 = min(a[-1][0], b[-1][0])
    if t1 <= t0:
        print("ERROR: no overlapping time window")
        sys.exit(2)
    wa = [r for r in a if t0 <= r[0] <= t1]
    wb = [r for r in b if t0 <= r[0] <= t1]
    if len(wa) < 3 or len(wb) < 3:
        print("ERROR: window has < 3 rows")
        sys.exit(2)

    lines_a = [canonical_line(r[0], r[1:4], r[4:8]) for r in wa]
    lines_b = [canonical_line(r[0], r[1:4], r[4:8]) for r in wb]
    bytes_a = "".join(lines_a).encode()
    bytes_b = "".join(lines_b).encode()

    byte_parity = bytes_a == bytes_b
    report = []
    report.append(f"window: {t0:.9f} .. {t1:.9f} s")
    report.append(f"PARITY-BYTE: {'PASS' if byte_parity else 'FAIL'}")
    report.append(f"  A bytes: {len(bytes_a)}  B bytes: {len(bytes_b)}")
    report.append(f"  A sha256: {hashlib.sha256(bytes_a).hexdigest()}")
    report.append(f"  B sha256: {hashlib.sha256(bytes_b).hexdigest()}")
    report.append(f"  A md5: {hashlib.md5(bytes_a).hexdigest()}")
    report.append(f"  B md5: {hashlib.md5(bytes_b).hexdigest()}")

    report.append(f"PARITY-NUMERIC:")
    report.append(f"  row count: A={len(wa)}  B={len(wb)}")
    report.append(f"  timestamp sequence: A strictly increasing "
                  f"{wa[0][0]:.9f}..{wa[-1][0]:.9f}; B strictly increasing "
                  f"{wb[0][0]:.9f}..{wb[-1][0]:.9f}")
    bd = {round(r[0], 9): r for r in wb}
    max_pos = rms_pos = max_rot = 0.0
    matched = unmatched = 0
    sq_sum = 0.0
    for r in wa:
        key = round(r[0], 9)
        if key not in bd:
            unmatched += 1
            continue
        s = bd[key]
        dp = math.sqrt(sum((x - y) ** 2 for x, y in zip(r[1:4], s[1:4])))
        dq = quat_angle(r[4:8], s[4:8])
        matched += 1
        max_pos = max(max_pos, dp)
        max_rot = max(max_rot, dq)
        sq_sum += dp * dp
    rms_pos = math.sqrt(sq_sum / matched) if matched else 0.0
    report.append(f"  stamp-matched poses: {matched}  unmatched: {unmatched}")
    report.append(f"  max translation delta: {max_pos:.3e} m")
    report.append(f"  RMS translation delta: {rms_pos:.3e} m")
    report.append(f"  max quaternion angle delta: {max_rot:.3e} rad "
                  f"(normalized; ~1e-8 rad diagnostics do NOT negate byte "
                  f"parity when hashes are equal)")

    if args.out:
        with open(args.out + ".a.tum", "w") as f:
            f.write("".join(lines_a))
        with open(args.out + ".b.tum", "w") as f:
            f.write("".join(lines_b))
        with open(args.out + ".report.txt", "w") as f:
            f.write("\n".join(report) + "\n")
        report.append(f"artifacts written: {args.out}.a.tum / .b.tum / .report.txt")

    print("\n".join(report))
    ok = byte_parity and matched > 0 and unmatched == 0
    print(f"RESULT: {'BYTE_PARITY=PASS' if ok else 'DIFFERS'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
