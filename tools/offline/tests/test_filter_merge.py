#!/usr/bin/env python3
"""T1-T7: deterministic merge semantics of filter_mcd (pure logic level)."""
import sys


def merge_streams(sources, selected):
    """Mirror filter_mcd.iter_selected ordering:
    returns list of (rec, si, seq, topic) in deterministic order."""
    items = []
    for si, src in enumerate(sources):
        for seq, (rec, topic) in enumerate(src):
            if topic not in selected:
                continue
            items.append((rec, si, seq, topic))
    items.sort(key=lambda x: (x[0], x[1], x[2]))
    return items


def main():
    ok = True
    def expect(name, cond):
        nonlocal ok
        print('%s: %s' % (name, 'PASS' if cond else 'FAIL'))
        ok &= cond

    # T1: two ordered streams interleave
    A = [(1.0, 'lidar'), (3.0, 'lidar'), (5.0, 'lidar')]
    B = [(2.0, 'imu'), (4.0, 'imu'), (6.0, 'imu')]
    out = merge_streams([A, B], ['lidar', 'imu'])
    expect('T1 interleave', [r for r, *_ in out] == [1, 2, 3, 4, 5, 6])

    # T2: one source EOF early -> no replay of other
    C = [(0.5, 'imu'), (7.0, 'imu')]
    out2 = merge_streams([A, C], ['lidar', 'imu'])
    expect('T2 EOF early no replay', len(out2) == 5 and out2[-1][0] == 7.0)

    # T3: equal timestamps -> stable (source_index, seq) tie-break
    D = [(1.0, 'lidar'), (2.0, 'imu')]
    E = [(1.0, 'imu'), (2.0, 'lidar')]
    out3 = merge_streams([D, E], ['lidar', 'imu'])
    expect('T3 tie-break stable',
           out3[0][1] == 0 and out3[0][3] == 'lidar' and
           out3[1][1] == 1 and out3[1][3] == 'imu')

    # T4: empty source passes others exactly once
    F = []
    out4 = merge_streams([F, D], ['lidar', 'imu'])
    expect('T4 empty source', len(out4) == 2)

    # T5: topic filtering
    G = [(1.0, 'irrelevant'), (2.0, 'lidar')]
    out5 = merge_streams([G], ['lidar'])
    expect('T5 filter', len(out5) == 1 and out5[0][3] == 'lidar')

    # T6: header timestamps preserved (record-time ordering must not alter
    # header stamps) — here header is implicit; assert seq identity kept
    H = [(1.0, 'lidar'), (2.0, 'lidar')]
    out6 = merge_streams([H], ['lidar'])
    expect('T6 header identity (seq kept)',
           out6[0][2] == 0 and out6[1][2] == 1)

    # T7: output count == sum(selected input counts)
    I = [(1.0, 'lidar'), (2.0, 'imu')]
    J = [(1.5, 'imu')]
    out7 = merge_streams([I, J], ['lidar', 'imu'])
    expect('T7 output count', len(out7) == 3)

    print('FILTER TDD:', 'ALL PASS' if ok else 'FAIL')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())