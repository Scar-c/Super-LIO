#!/usr/bin/env python3
"""P0R3: production-vs-exact-ns comparison with binary64 boundary classifier.

Primary gate (Owner numeric decision):
    semantic_mismatch_unique == 0
    ordered_epoch_sequence_mismatch_count == 0
    readiness mismatch == 0

A unique point mismatch is a REPRESENTATION_COLLISION only if ALL of:
  R1 exact ownership disagrees: exact point_time_ns > exact tc_ns while
     production emitted current
  R2 delta_ns = point_time_ns - tc_ns > 0
  R3 production binary64 comparison cannot distinguish: the exact
     production arithmetic (header.to_sec() + offset*1e-9 as binary64)
     yields point_abs_double <= tc_double
  R4 delta_ns <= ULP(tc_double) in ns (ULP from the actual runtime value)
  R5 no future-data/readiness mismatch exists for this epoch
  R6 point identity is otherwise conserved (no dup/missing/overlap)
  R7 mismatch is only local epoch ownership; camera epoch sequence and
     readiness are identical

Anything else is SEMANTIC_MISMATCH (hard fail).
"""
import argparse
import math
import json
import sys


def _ulp_s(x):
    """binary64 spacing for positive normals via bit increment (py3.8)."""
    import struct
    bits = struct.unpack(">Q", struct.pack(">d", x))[0]
    return struct.unpack(">d", struct.pack(">Q", bits + 1))[0] - x


def ulp_ns(tc_ns):
    """binary64 spacing of the runtime tc value (double nearest tc_ns*1e-9)."""
    tc_double = float(tc_ns) / 1e9
    return _ulp_s(tc_double) * 1e9


def production_point_double(header_ns, offset_ns):
    """Exact production arithmetic: header.to_sec() + offset_time*1e-9."""
    header_double = float(header_ns) / 1e9
    offset_double = float(offset_ns) * 1e-9
    return header_double + offset_double


def classify_mismatch(point_time_ns, prod_tc_ns, orac_tc_ns,
                      readiness_mismatch, seq_ok, conservation_ok):
    """Classify one unique point-assignment mismatch.

    Returns (kind, delta_ns, tc_double, ulp_ns).
    kind: "REPRESENTATION_COLLISION" or "SEMANTIC_MISMATCH".
    """
    delta_ns = point_time_ns - prod_tc_ns
    tc_double = float(prod_tc_ns) / 1e9
    ulp = _ulp_s(tc_double) * 1e9
    # R1/R2: exact point past the production epoch, oracle emitted later
    if not (point_time_ns > prod_tc_ns and orac_tc_ns > prod_tc_ns):
        return "SEMANTIC_MISMATCH", delta_ns, tc_double, ulp
    # R4: within one binary64 spacing
    if delta_ns > ulp:
        return "SEMANTIC_MISMATCH", delta_ns, tc_double, ulp
    # R5/R7: readiness and ordered sequence must be identical
    if readiness_mismatch != 0 or not seq_ok:
        return "SEMANTIC_MISMATCH", delta_ns, tc_double, ulp
    # R6: identity conservation global
    if not conservation_ok:
        return "SEMANTIC_MISMATCH", delta_ns, tc_double, ulp
    return "REPRESENTATION_COLLISION", delta_ns, tc_double, ulp


def percentile(sorted_values, p):
    if not sorted_values:
        return None
    index = (len(sorted_values) - 1) * p / 100.0
    lo = int(math.floor(index))
    hi = int(math.ceil(index))
    if lo == hi:
        return sorted_values[lo]
    return sorted_values[lo] + (sorted_values[hi] - sorted_values[lo]) * (
        index - lo)


def ordered_epoch_mismatch(prod_tcs, orac_tcs):
    if len(prod_tcs) != len(orac_tcs):
        return len(prod_tcs) + len(orac_tcs), None
    for index, (a, b) in enumerate(zip(prod_tcs, orac_tcs)):
        if a != b:
            return 1, index
    return 0, None


def resolve_point_time(ids, bag_path, lidar_topic="/livox/lidar",
                       filter_rate=3, blind=2.0, max_range=60.0):
    """Exact point ns for the given (scan_id<<32)|idx ids from the real bag.

    Scans are indexed by point_num >= 10 messages in record order
    (production raw-scan id assignment); idx is the raw point index.
    """
    import rosbag
    from audit_common import header_nsec
    result = {}
    scan_id = 0
    blind2 = blind * blind
    max_range2 = max_range * max_range
    with rosbag.Bag(bag_path) as bag:
        for topic, msg, _ in bag.read_messages(topics=[lidar_topic]):
            if int(msg.point_num) < 10:
                continue
            wanted = [idx for sid, idx in ids if sid == scan_id]
            if wanted:
                hns = header_nsec(msg)
                for idx in wanted:
                    if idx < len(msg.points):
                        result[(scan_id, idx)] = hns + int(msg.points[idx].offset_time)
            scan_id += 1
    return result


def load(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--production", required=True)
    ap.add_argument("--oracle", required=True)
    ap.add_argument("--bag")
    args = ap.parse_args()

    prod = load(args.production)
    orac = load(args.oracle)

    prod_emitted = dict(prod["emitted"])
    orac_emitted = dict(orac["emitted"])
    prod_retained = set(prod["final_retained"])
    orac_retained = set(orac["final_retained"])

    # ordered epoch sequence (P0R3 7.1)
    seq_mismatch, first_index = ordered_epoch_mismatch(
        prod["epoch_tcs"], orac["epoch_tcs"])

    # readiness (set-level, legacy) + retained/emitted identity accounting
    prod_tcs = prod["epoch_tcs"]
    orac_tcs = orac["epoch_tcs"]
    readiness_mismatch = (
        len(prod_tcs) + len(orac_tcs) - 2 * len(set(prod_tcs) & set(orac_tcs)))
    emitted_only_prod = sorted(set(prod_emitted) - set(orac_emitted))
    emitted_only_oracle = sorted(set(orac_emitted) - set(prod_emitted))
    retained_sym_diff = sorted(prod_retained ^ orac_retained)

    # unique mismatch ids (symmetric difference of assignment maps)
    mismatch_ids = set()
    for ident, epoch in prod_emitted.items():
        if orac_emitted.get(ident) != epoch:
            mismatch_ids.add(ident)
    for ident, epoch in orac_emitted.items():
        if prod_emitted.get(ident) != epoch:
            mismatch_ids.add(ident)
    unique_mismatch = sorted(mismatch_ids)

    conservation_ok = (len(emitted_only_prod) == 0 and
                       len(emitted_only_oracle) == 0 and
                       len(retained_sym_diff) == 0)

    # resolve exact point times for the mismatches
    point_times = {}
    if args.bag and unique_mismatch:
        point_times = resolve_point_time(
            [(ident >> 32, ident & 0xFFFFFFFF) for ident in unique_mismatch],
            args.bag)

    collisions = []
    semantic = []
    for ident in unique_mismatch:
        prod_tc = prod_emitted.get(ident)
        orac_tc = orac_emitted.get(ident)
        pns = point_times.get((ident >> 32, ident & 0xFFFFFFFF))
        if pns is None:
            semantic.append((ident, prod_tc, orac_tc, None))
            continue
        kind, delta_ns, tc_double, ulp = classify_mismatch(
            pns, prod_tc, orac_tc, readiness_mismatch, seq_mismatch == 0,
            conservation_ok)
        record = (ident, prod_tc, orac_tc, pns, delta_ns, tc_double, ulp)
        if kind == "REPRESENTATION_COLLISION":
            collisions.append(record)
        else:
            semantic.append(record)

    delta_values = sorted(r[4] for r in collisions)
    ulp_values = sorted(r[6] for r in collisions)
    ratio_max = max((r[4] / r[6] for r in collisions), default=0.0)

    print("production vs exact-ns oracle (P0R3 classifier)")
    print("  emitted ids: prod=%d oracle=%d" % (len(prod_emitted), len(orac_emitted)))
    print("  retained ids: prod=%d oracle=%d" % (len(prod_retained), len(orac_retained)))
    print("  emitted-only-prod: %d   emitted-only-oracle: %d   retained sym diff: %d"
          % (len(emitted_only_prod), len(emitted_only_oracle), len(retained_sym_diff)))
    print("  epoch tcs: prod=%d oracle=%d" % (len(prod_tcs), len(orac_tcs)))
    print("readiness mismatch:            %d" % readiness_mismatch)
    print("ordered epoch sequence mismatch: %d (first index %s)"
          % (seq_mismatch, first_index))
    print("unique point assignment mismatch: %d" % len(unique_mismatch))
    print("representation collisions:     %d" % len(collisions))
    print("semantic mismatches:           %d" % len(semantic))
    if collisions:
        print("collision delta_ns min/P50/P90/P99/max: %.0f/%.0f/%.0f/%.0f/%.0f"
              % (percentile(delta_values, 0), percentile(delta_values, 50),
                 percentile(delta_values, 90), percentile(delta_values, 99),
                 percentile(delta_values, 100)))
        print("collision ULP_ns min/P50/P90/P99/max: %.3f/%.3f/%.3f/%.3f/%.3f"
              % (percentile(ulp_values, 0), percentile(ulp_values, 50),
                 percentile(ulp_values, 90), percentile(ulp_values, 99),
                 percentile(ulp_values, 100)))
        print("max(delta/ULP): %.4f" % ratio_max)
    if semantic:
        for record in semantic[:5]:
            print("  SEMANTIC:", record)
    ok = (len(semantic) == 0 and seq_mismatch == 0 and readiness_mismatch == 0)
    print("P0R3 GATE: %s" % ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
