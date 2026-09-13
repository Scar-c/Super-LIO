#!/usr/bin/env python3
"""Convert native /lio/odom ROS1 messages to the baseline TUM byte format."""

import argparse
import pathlib

import rosbag


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--bag", required=True)
    parser.add_argument("--topic", default="/lio/odom")
    parser.add_argument("--output", required=True)
    args = parser.parse_args(argv)

    count = 0
    last = None
    output = pathlib.Path(args.output)
    with rosbag.Bag(args.bag) as bag, output.open("w", encoding="utf-8") as stream:
        for _, message, _ in bag.read_messages(topics=[args.topic]):
            stamp = message.header.stamp.to_sec()
            if last is not None and stamp <= last:
                raise RuntimeError(f"non-increasing pose stamp {stamp} <= {last}")
            p = message.pose.pose.position
            q = message.pose.pose.orientation
            values = (stamp, p.x, p.y, p.z, q.x, q.y, q.z, q.w)
            stream.write(" ".join(format(value, ".17g") for value in values) + "\n")
            last = stamp
            count += 1
    if count == 0:
        raise RuntimeError(f"no messages on {args.topic}")
    print(f"rows={count} output={output}")


if __name__ == "__main__":
    main()
