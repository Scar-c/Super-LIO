#!/usr/bin/env python3
"""Inspect a rosbag: duration, topics, types, counts, fields of first message."""
import argparse, sys
import rosbag

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('bag')
    ap.add_argument('--topic', default=None)
    args = ap.parse_args()
    b = rosbag.Bag(args.bag)
    info = b.get_type_and_topic_info()
    print('duration: %.1f s  size: %d bytes' % (b.get_end_time() - b.get_start_time(), b.get_size()))
    for t, i in info[1].items():
        print('%-45s %-35s n=%d' % (t, i.msg_type, i.message_count))
    if args.topic:
        for i, (t, m, ts) in enumerate(b.read_messages(topics=[args.topic])):
            if hasattr(m, 'fields'):
                print('fields:', [(f.name, f.datatype) for f in m.fields])
            elif hasattr(m, 'encoding'):
                print('encoding:', m.encoding, 'size:', getattr(m, 'width', 0), 'x', getattr(m, 'height', 0),
                      'step:', m.step, 'data:', len(m.data))
            else:
                print('first msg fields:', [a for a in dir(m) if not a.startswith('_')][:12])
            break
    b.close()

if __name__ == '__main__':
    main()
