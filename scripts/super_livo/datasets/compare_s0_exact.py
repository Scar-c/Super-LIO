#!/usr/bin/env python3
"""P0R2: production-vs-exact-ns FrozenS0ReferenceOracle comparison.

Loads s0_audit_exact.json from the production offline node and from the
exact-ns oracle, then reports:
  - point epoch-assignment mismatch count       (desired 0)
  - boundary ownership mismatch count           (desired 0)
  - camera readiness mismatch count             (desired 0)
"""
import argparse
import json
import sys


def load(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--production", required=True)
    ap.add_argument("--oracle", required=True)
    args = ap.parse_args()

    prod = load(args.production)
    orac = load(args.oracle)

    prod_emitted = dict(prod["emitted"])
    orac_emitted = dict(orac["emitted"])
    prod_retained = set(prod["final_retained"])
    orac_retained = set(orac["final_retained"])

    # point assignment: symmetric difference of (id -> epoch) maps plus the
    # retained sets; an id must be emitted by the same epoch or retained on
    # both sides.
    point_mismatch = 0
    for ident, epoch in orac_emitted.items():
        if prod_emitted.get(ident) != epoch:
            point_mismatch += 1
    for ident, epoch in prod_emitted.items():
        if orac_emitted.get(ident) != epoch:
            point_mismatch += 1
    point_mismatch += len(prod_retained ^ orac_retained)

    # boundary ownership: the exact boundary ids must be identical
    prod_boundary = {ident for ident, _ in prod["boundary"]}
    orac_boundary = {ident for ident, _ in orac["boundary"]}
    boundary_mismatch = len(prod_boundary ^ orac_boundary)

    # camera readiness: epoch tc lists must be identical
    prod_tcs = prod["epoch_tcs"]
    orac_tcs = orac["epoch_tcs"]
    readiness_mismatch = (
        len(prod_tcs) + len(orac_tcs) - 2 * len(
            set(prod_tcs) & set(orac_tcs)))

    print("production vs exact-ns oracle")
    print("  emitted ids: prod=%d oracle=%d" % (
        len(prod_emitted), len(orac_emitted)))
    print("  retained ids: prod=%d oracle=%d" % (
        len(prod_retained), len(orac_retained)))
    print("  boundary ids: prod=%d oracle=%d" % (
        len(prod_boundary), len(orac_boundary)))
    print("  epoch tcs: prod=%d oracle=%d" % (len(prod_tcs), len(orac_tcs)))
    print("point epoch-assignment mismatch: %d" % point_mismatch)
    print("boundary ownership mismatch:     %d" % boundary_mismatch)
    print("camera readiness mismatch:       %d" % readiness_mismatch)
    ok = point_mismatch == 0 and boundary_mismatch == 0 and \
        readiness_mismatch == 0
    print("PROD-VS-EXACT: %s" % ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
