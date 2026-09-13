#!/usr/bin/env python3
"""Convert native /lio/odom ROS1 messages to the baseline TUM byte format."""

import argparse
import pathlib
import struct

import rosbag


def next_up(value):
    bits = struct.unpack(">Q", struct.pack(">d", value))[0]
    bits += 1 if value >= 0.0 else -1
    return struct.unpack(">d", struct.pack(">Q", bits))[0]


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--bag", required=True)
    parser.add_argument("--topic", default="/lio/odom")
    parser.add_argument("--output", required=True)
    args = parser.parse_args(argv)

    count = 0
    last_serialized_stamp = None
    output = pathlib.Path(args.output)
    with rosbag.Bag(args.bag) as bag, output.open("w", encoding="utf-8") as stream:
        for _, message, _ in bag.read_messages(topics=[args.topic]):
            raw_stamp = message.header.stamp.to_sec()
            # Several native odom headers can be distinct at sub-format
            # precision but collapse to the same TUM decimal when written at
            # .17g. Keep every native row and make only the serialized output
            # strictly increasing with the next representable double.
            stamp = float(format(raw_stamp, ".17g"))
            if last_serialized_stamp is not None and stamp <= last_serialized_stamp:
                stamp = next_up(last_serialized_stamp)
            p = message.pose.pose.position
            q = message.pose.pose.orientation
            values = (stamp, p.x, p.y, p.z, q.x, q.y, q.z, q.w)
            stream.write(" ".join(format(value, ".17g") for value in values) + "\n")
            last_serialized_stamp = stamp
            count += 1
    if count == 0:
        raise RuntimeError(f"no messages on {args.topic}")
    print(f"rows={count} output={output}")


if __name__ == "__main__":
    main()
