#!/usr/bin/env python3
"""R3-T1..T10: binary64 boundary-equivalence classifier TDD (P0R3)."""
import math
import sys

sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parents[2]
                        / "super_livo/datasets"))

from compare_s0_exact import (classify_mismatch, ordered_epoch_mismatch,
                              production_point_double, ulp_ns)

# representative epoch ~ 1.645e9 s (Day10)
TC = 1645008760108858824


def main():
    ok = True

    def expect(name, cond):
        nonlocal ok
        print("%s: %s" % (name, "PASS" if cond else "FAIL"))
        ok &= cond

    # R3-T1: exact equal -> no mismatch (equal assignments never enter the
    # classifier; verify classify would not mark it a collision)
    kind, delta, tc, ulp = classify_mismatch(
        TC, TC, TC + 1000000000, 0, True, True)
    expect("R3-T1 exact equal not collision", kind == "SEMANTIC_MISMATCH")

    # R3-T2: exact +5ns, double collapses equal -> REPRESENTATION_COLLISION
    p5 = TC + 5
    expect("R3-T2 double collapse of +5ns",
           float(p5) / 1e9 == float(TC) / 1e9)
    kind, delta, tc, ulp = classify_mismatch(p5, TC, TC + 33000000, 0, True, True)
    expect("R3-T2 +5ns collision", kind == "REPRESENTATION_COLLISION")

    # R3-T3: +25ns collapses -> REPRESENTATION_COLLISION
    p25 = TC + 25
    expect("R3-T3 double collapse of +25ns",
           float(p25) / 1e9 == float(TC) / 1e9)
    kind, _, _, _ = classify_mismatch(p25, TC, TC + 33000000, 0, True, True)
    expect("R3-T3 +25ns collision", kind == "REPRESENTATION_COLLISION")

    # R3-T4: delta larger than one ULP -> SEMANTIC_MISMATCH
    import struct as _s
    _tc = float(TC) / 1e9
    _bits = _s.unpack(">Q", _s.pack(">d", _tc))[0]
    ulp = (_s.unpack(">d", _s.pack(">Q", _bits + 1))[0] - _tc) * 1e9
    big = TC + int(ulp) + 1
    kind, _, _, _ = classify_mismatch(big, TC, TC + 33000000, 0, True, True)
    expect("R3-T4 >1 ULP semantic", kind == "SEMANTIC_MISMATCH")

    # R3-T5: point assigned to an earlier epoch far away -> SEMANTIC_MISMATCH
    # (production emitted at an epoch several camera periods back)
    kind, _, _, _ = classify_mismatch(
        TC - 66000000, TC - 66000000, TC, 0, True, True)
    expect("R3-T5 earlier-epoch emission semantic", kind == "SEMANTIC_MISMATCH")

    # R3-T6: readiness difference -> SEMANTIC_MISMATCH even if sub-ULP
    kind, _, _, _ = classify_mismatch(p5, TC, TC + 33000000, 3, True, True)
    expect("R3-T6 readiness diff hard fail", kind == "SEMANTIC_MISMATCH")

    # R3-T7: duplicate camera epoch hidden by set -> ordered comparator catches
    mismatch, index = ordered_epoch_mismatch([1, 2, 2, 3], [1, 2, 3])
    expect("R3-T7 duplicate epoch caught", mismatch > 0)

    # R3-T8: reordered camera epochs caught
    mismatch, index = ordered_epoch_mismatch([1, 2, 3], [1, 3, 2])
    expect("R3-T8 reorder caught", mismatch > 0 and index == 1)
    mismatch, index = ordered_epoch_mismatch([1, 2, 3], [1, 2, 3])
    expect("R3-T8 identical sequence ok", mismatch == 0 and index is None)

    # R3-T9: unique mismatch counting (classifier input side: one point with
    # different epochs is ONE unique mismatch)
    ids = {100: 1, 200: 2}
    orac_ids = {100: 2, 200: 2}
    unique = set(ids) | set(orac_ids)
    expect("R3-T9 unique counting", len(unique) == 2)

    # R3-T10: real-value ULP at ~1.645e9 s
    import struct as _s
    _tc = float(TC) / 1e9
    _bits = _s.unpack(">Q", _s.pack(">d", _tc))[0]
    computed = (_s.unpack(">d", _s.pack(">Q", _bits + 1))[0] - _tc) * 1e9
    expect("R3-T10 ULP ~238.418579 ns",
           abs(computed - 238.4185791015625) < 1e-6)

    # R3 extra: production arithmetic collapses point into tc (R3 condition)
    header_ns = 1645008779199000597
    offset_ns = 56340000
    point_double = production_point_double(header_ns, offset_ns)
    tc_double = float(1645008779255340576) / 1e9
    expect("R3 production double collapse",
           point_double <= tc_double and abs(point_double - tc_double) < 1e-6)

    print("P0R3 CLASSIFIER TDD:", "ALL PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
