#!/usr/bin/env python3
"""P0 trajectory-fidelity check: compare two trajectories.

Both runs use the identical production estimator; the offline runner must
reproduce the online trajectory. Poses are matched by stamp (authoritative
message header stamps); any unmatched stamps are reported.

Inputs: two trajectory files, each either a TUM text file
(timestamp x y z qx qy qz qw) or a rosbag containing --topic.

Usage:
  extract_and_compare.py <trajA> <trajB> [--topic /lio/odom] [--tol 1e-6]
"""
import argparse
import math
import os
import sys


def load_tum(path):
    out = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) < 8:
                continue
            t = float(parts[0])
            xyz = tuple(float(v) for v in parts[1:4])
            quat = tuple(float(v) for v in parts[4:8])
            out.append((t, xyz, quat))
    out.sort()
    return out


def load_bag(path, topic):
    import rosbag
    out = []
    with rosbag.Bag(path, "r") as b:
        for _, msg, _ in b.read_messages(topics=[topic]):
            t = msg.header.stamp.to_sec()
            p = msg.pose.pose.position
            o = msg.pose.pose.orientation
            out.append((t, (p.x, p.y, p.z), (o.x, o.y, o.z, o.w)))
    out.sort()
    return out


def load(path, topic):
    return load_tum(path) if path.endswith(".tum") else load_bag(path, topic)


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
    ap.add_argument("--topic", default="/lio/odom")
    ap.add_argument("--tol", type=float, default=1e-6)
    args = ap.parse_args()

    a = load(args.traj_a, args.topic)
    b = load(args.traj_b, args.topic)
    if not a or not b:
        print(f"ERROR: empty trajectory ({len(a)}/{len(b)})")
        sys.exit(2)

    bd = {round(t, 9): (p, q) for t, p, q in b}
    matched = unmatched = 0
    max_pos = max_rot = 0.0
    pos_deltas = []
    for t, p, q in a:
        key = round(t, 9)
        # exact stamp match first, then tolerance window
        cand = None
        if key in bd:
            cand = key
        else:
            for bt in bd:
                if abs(bt - t) <= args.tol:
                    cand = bt
                    break
        if cand is None:
            unmatched += 1
            continue
        bp, bq = bd.pop(cand)
        dp = math.sqrt(sum((x - y) ** 2 for x, y in zip(p, bp)))
        dq = quat_angle(q, bq)
        matched += 1
        max_pos = max(max_pos, dp)
        max_rot = max(max_rot, dq)
        pos_deltas.append(dp)

    print(f"traj A: {len(a)} poses  traj B: {len(b)} poses")
    print(f"matched: {matched}  unmatched: {unmatched}  (tol={args.tol}s)")
    if matched:
        mean_pos = sum(pos_deltas) / len(pos_deltas)
        print(f"max position delta: {max_pos:.3e} m   mean: {mean_pos:.3e} m")
        print(f"max orientation delta: {max_rot:.3e} rad")
    ok = matched > 0 and unmatched == 0 and max_pos < 1e-6 and max_rot < 1e-6
    print(f"RESULT: {'IDENTICAL' if ok else 'DIFFERS'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
