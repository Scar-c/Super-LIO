#!/usr/bin/env python3
"""Persistent benchmark-cache manifest (bags never enter Git).

add  : register a generated bag into a manifest with provenance + bag MD5
check: verify a bag file MD5/counts against an existing manifest entry
"""
import argparse, csv, hashlib, json, os, subprocess, sys


def bag_info(path):
    b = None
    import rosbag
    b = rosbag.Bag(path)
    info = b.get_type_and_topic_info()
    counts = {t: i.message_count for t, i in info[1].items()}
    dur = b.get_end_time() - b.get_start_time()
    b.close()
    return {'duration': dur, 'topics': counts}


def md5(path, chunk=8 << 20):
    h = hashlib.md5()
    with open(path, 'rb') as f:
        while True:
            d = f.read(chunk)
            if not d:
                break
            h.update(d)
    return h.hexdigest()


def git_head():
    try:
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'],
                                       cwd=os.path.dirname(os.path.dirname(
                                           os.path.dirname(os.path.abspath(__file__)))))
    except Exception:
        return b'unknown'
    return b'unknown'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', choices=['add', 'check'])
    ap.add_argument('--bag', required=True)
    ap.add_argument('--manifest', required=True)
    ap.add_argument('--mode', default='lio')
    ap.add_argument('--sources', nargs='+', default=[])
    ap.add_argument('--topics', nargs='+', default=[])
    ap.add_argument('--convention', default='')
    ap.add_argument('--parity', default='')
    args = ap.parse_args()

    if args.action == 'add':
        info = bag_info(args.bag)
        entry = {
            'bag': os.path.abspath(args.bag),
            'size': os.path.getsize(args.bag),
            'md5': md5(args.bag),
            'duration': info['duration'],
            'topics': info['topics'],
            'mode': args.mode,
            'sources': args.sources,
            'selected_topics': args.topics,
            'convention': args.convention,
            'parity': args.parity,
            'generator_git_head': git_head().decode(),
        }
        m = {}
        if os.path.exists(args.manifest):
            m = json.load(open(args.manifest))
        m[os.path.basename(args.bag)] = entry
        json.dump(m, open(args.manifest, 'w'), indent=2)
        print('registered', os.path.basename(args.bag), entry['md5'])
    else:  # check — recompute current bag metadata, compare against manifest
        if not os.path.exists(args.manifest):
            print('FAIL: manifest missing')
            return 1
        m = json.load(open(args.manifest))
        key = os.path.basename(args.bag)
        if key not in m:
            print('FAIL: no manifest entry for', key)
            return 1
        e = m[key]
        cur_md5 = md5(args.bag)
        cur_size = os.path.getsize(args.bag)
        cur_info = bag_info(args.bag)
        ok = (cur_md5 == e['md5'] and cur_size == e['size'] and
              abs(cur_info['duration'] - e['duration']) < 0.01 and
              cur_info['topics'] == e['topics'])
        if ok and e.get('mode') == 'livo' and 'selected_topics' in e:
            for t in ('/livox/lidar', '/vn100/imu',
                      '/d435i/infra1/image_rect_raw'):
                if t in e['selected_topics'] and t not in cur_info['topics']:
                    ok = False
        print('check %s md5=%s size=%d duration=%.2f topics=%s' %
              ('PASS' if ok else 'FAIL', cur_md5, cur_size,
               cur_info['duration'], cur_info['topics']))
        return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
