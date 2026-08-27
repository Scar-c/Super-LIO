#!/usr/bin/env python3
"""Convert a nav_msgs/Odometry bag topic to strict TUM trajectory text."""

import argparse
import rosbag


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--bag", required=True)
    parser.add_argument("--topic", default="/lio/odom")
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    count = 0
    last = None
    with rosbag.Bag(args.bag) as bag, open(args.output, "w", encoding="utf-8") as out:
        for _, msg, _ in bag.read_messages(topics=[args.topic]):
            stamp = msg.header.stamp.to_sec()
            if last is not None and stamp <= last:
                raise RuntimeError(f"non-increasing odometry stamp {stamp} <= {last}")
            p, q = msg.pose.pose.position, msg.pose.pose.orientation
            out.write(
                f"{stamp:.9f} {p.x:.12g} {p.y:.12g} {p.z:.12g} "
                f"{q.x:.12g} {q.y:.12g} {q.z:.12g} {q.w:.12g}\n"
            )
            last = stamp
            count += 1
    if count == 0:
        raise RuntimeError(f"no messages on {args.topic}")
    print(f"rows={count}")


if __name__ == "__main__":
    main()
