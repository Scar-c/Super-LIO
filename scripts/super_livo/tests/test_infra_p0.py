#!/usr/bin/env python3
"""Round11W-P0 infra TDD: A-K coverage (synthetic, no bags)."""
import json
import os
import subprocess
import sys
import tempfile

ROOT = str(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..")))


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def main():
    ok = True

    def expect(name, cond):
        nonlocal ok
        print('%s: %s' % (name, 'PASS' if cond else 'FAIL'))
        ok &= cond

    # A/B/C: slice oracle + drain + negative fixture are covered by
    # test_lidar_slice_accounting.py and test_round11u_causal_oracles.py.
    r1 = run([sys.executable,
              os.path.join(ROOT, 'scripts/super_livo/tests/test_lidar_slice_accounting.py')])
    expect('A slice oracle TDD', r1.returncode == 0 and 'ALL PASS' in r1.stdout)
    r2 = run([sys.executable,
              os.path.join(ROOT, 'scripts/super_livo/tests/test_round11u_causal_oracles.py')])
    expect('B/C causal oracle + negative fixture', r2.returncode == 0 and 'ALL PASS' in r2.stdout)

    # D: variant matrix — B0 must set /camera/enabled=false (regression E)
    runner = open(os.path.join(ROOT, 'scripts/super_livo/experiments/run_offline_variant.sh')).read()
    expect('D B0 camera=false+camera_epoch=false explicit', 'b0)' in runner and
           runner.split('b0)')[1].split(';;')[0].count('/camera/enabled false') == 1)
    expect('E no /lio/camera/enabled', '/lio/camera/enabled' not in runner)
    expect('D0 explicit identity and fullscan guard', 'd0)' in runner and
           'd0 requires imu_fullscan' in runner and
           'variant: b0|c0|d0|a0|a1' in runner)

    # F: isolated master — runner uses ROS_MASTER_URI + roscore -p
    expect('F isolated master', 'ROS_MASTER_URI' in runner and 'roscore -p' in runner)

    # G: filter exact record-ns tie ordering
    filt = open(os.path.join(ROOT, 'tools/offline/filter_mcd.py')).read()
    expect('G filter to_nsec key', 'rec_ns = ts.to_nsec()' in filt and
           "heapq.heappush(heap, st['active'])" in filt)

    # H: filter required-topic fail-closed
    expect('H required-topic fail-closed',
           'required topic' in filt and 'SystemExit' in filt)

    # I: atomic partial output
    expect('I atomic partial', '.partial' in filt and 'os.rename(' in filt and
           'os.remove(partial_path)' in filt)

    # J: manifest check recomputes counts
    man = open(os.path.join(ROOT, 'scripts/super_livo/datasets/cache_manifest.py')).read()
    expect('J manifest recompute counts', 'bag_info(args.bag)' in man and
           "cur_info['topics'] == e['topics']" in man)

    # K: evaluator matched-duration reporting
    ev = open(os.path.join(ROOT, 'scripts/super_livo/evaluation/eval_tum_translation.py')).read()
    expect('K evaluator matched_duration', 'matched_duration' in ev and
           'assoc_dt_p99' in ev)

    # shell syntax checks
    for sh in ['run_offline_variant.sh', 'run_b0_c0_a0_a1.sh']:
        rs = run(['bash', '-n', os.path.join(ROOT, 'scripts/super_livo/experiments', sh)])
        expect('shell syntax ' + sh, rs.returncode == 0)

    print('INFRA TDD:', 'ALL PASS' if ok else 'FAIL')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
