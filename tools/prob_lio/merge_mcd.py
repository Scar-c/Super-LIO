#!/usr/bin/env python3
# Provenance: ported from the audited legacy tool
# /home/lc/super_livo_legacy/src/Super-LIO/tools/offline/filter_mcd.py.
# This is the bounded-memory record-time k-way merge; it preserves message
# header stamps and never invents synchronization or ordering semantics.
"""MCD canonical filtered-bag preprocessing — bounded-memory k-way merge.

Streaming contract:
  - one open Bag + one read_messages iterator per source bag
  - heap holds at most ONE current message per source bag
  - heap key: (record_time_nsec, bag_index, per_bag_sequence)
  - pop earliest -> write immediately -> advance ONLY that bag -> push next
  - EOF source exits heap; no replay, no stall
  - msg.header.stamp preserved verbatim; record time only for order/output
  - header parity: streaming counters + first/last + bounded rolling hash
"""
import argparse
import hashlib
import heapq
import resource
import sys

import rosbag
import rospy


def topic_counts(path):
    b = rosbag.Bag(path)
    try:
        info = b.get_type_and_topic_info()
        return {t: i.message_count for t, i in info[1].items()}
    finally:
        b.close()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--lidar', required=True)
    ap.add_argument('--imu', required=True)
    ap.add_argument('--camera', default=None)
    ap.add_argument('--out', required=True)
    ap.add_argument('--bags', nargs='+', required=True)
    ap.add_argument('--mode', choices=['lio', 'livo'], default='lio')
    ap.add_argument('--duration', type=float, default=-1.0)
    args = ap.parse_args()

    selected = [args.lidar, args.imu]
    if args.mode == 'livo':
        if not args.camera:
            raise SystemExit('livo mode requires --camera')
        selected.append(args.camera)

    # required-topic fail-closed: each required topic must exist in >=1 bag
    avail_all = set()
    for path in args.bags:
        avail_all |= set(topic_counts(path).keys())
    for t in selected:
        if t not in avail_all:
            raise SystemExit('FAIL: required topic %s absent from all source bags' % t)

    # open bags + iterators (one per bag, covering its selected topics)
    # F7: every opened handle owned in one deterministic collection.
    bags = []
    iters = []
    def advance(st):
        if st.get('done'):
            st['active'] = None
            return
        try:
            topic, msg, ts = next(st['it'])
            rec_ns = ts.to_nsec()
            ht = msg.header.stamp.to_sec() if hasattr(msg, 'header') else 0.0
            st['active'] = (rec_ns, st['si'], st['seq'], topic, msg, ts, ht)
            st['seq'] += 1
            heapq.heappush(heap, st['active'])
        except StopIteration:
            st['active'] = None

    final_path = args.out + ('_lio_filtered.bag' if args.mode == 'lio'
                            else '_livo_filtered.bag')
    partial_path = final_path + '.partial'
    try:
        for path in args.bags:
            avail = set(topic_counts(path).keys())
            want = [t for t in selected if t in avail]
            b = rosbag.Bag(path)
            bags.append(b)
            if want:
                it = b.read_messages(topics=want)
                iters.append({'it': it, 'seq': 0, 'active': None, 'done': False, 'si': len(iters)})
            else:
                iters.append({'it': None, 'seq': 0, 'active': None, 'done': True, 'si': len(iters)})

        heap = []
        MOD = 1 << 62

        for si in range(len(iters)):
            advance(iters[si])

        first_rec_ns = None
        prev_rec_ns = -1
        counts = {}
        first_ht = {}
        last_ht = {}
        hash_state = {}

        with rosbag.Bag(partial_path, 'w', compression='lz4') as out:
            while heap:
                rec_ns, si, seq, topic, msg, ts, ht = heapq.heappop(heap)
                if first_rec_ns is None:
                    first_rec_ns = rec_ns
                if args.duration > 0.0 and rec_ns - first_rec_ns > int(args.duration * 1e9):
                    break
                if rec_ns < prev_rec_ns:
                    raise RuntimeError('non-monotonic record time %d < %d'
                                       % (rec_ns, prev_rec_ns))
                prev_rec_ns = rec_ns
                # F6: preserve the ORIGINAL record Time object exactly
                out.write(topic, msg, t=ts)
                counts[topic] = counts.get(topic, 0) + 1
                first_ht.setdefault(topic, ht)
                last_ht[topic] = ht
                h = hash_state.get(topic, 0)
                h = (h * 131 + int(ht * 1e9)) % MOD
                hash_state[topic] = h
                advance(iters[si])
    except Exception as e:
        # partial output must not be confused with a complete canonical bag
        import os
        if os.path.exists(partial_path):
            os.remove(partial_path)
        raise
    finally:
        # F7: all opened handles close deterministically, on any path
        for b in bags:
            try:
                b.close()
            except Exception:
                pass
    # atomic rename to final only after successful close
    import os
    os.rename(partial_path, final_path)

    rss_kb = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    print('output:', final_path)
    print('counts:', counts)
    print('first_ht:', first_ht)
    print('last_ht:', last_ht)
    print('rolling_hash:', hash_state)
    print('peak_rss_kb:', rss_kb)
    print('done')


if __name__ == '__main__':
    main()

