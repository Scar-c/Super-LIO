#!/usr/bin/env python3
"""Prob-LIO P5 shadow association diagnosis report.

Consumes the per-frame summary CSV produced by the shadow run
(super_lio/assoc_shadow_frames.csv, g_root_dir of the package) and emits a
compact report:

  - total four-way quadrant matrix (LA_PA/LA_PR/LR_PA/LR_PR; sum == attempted)
  - top N frames by LA_PR count
  - top N frames by LA_PR / legacy_accept fraction
  - first frame where disagreement spikes (LA_PR >= threshold)
  - consecutive disagreement bursts
  - component summaries (residual/sigma/z/plane/query-sensor/pose-rot/pose-pos)
  - count-bin table (bins 1 / 2-4 / 5-9 / 10-14 / 15-20)

CLI-parameterized; no dataset hard-coding.

Usage:
  analyze_assoc_shadow.py <frames.csv> [--out report.txt] [--top 10]
                          [--spike 100] [--burst-gap 5]
"""
import argparse
import csv
import statistics


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--out", default=None)
    ap.add_argument("--top", type=int, default=10)
    ap.add_argument("--spike", type=int, default=100,
                    help="LA_PR count at which a frame is a 'spike'")
    ap.add_argument("--burst-gap", type=int, default=5,
                    help="frame-index gap that separates bursts")
    args = ap.parse_args()

    rows = []
    with open(args.csv) as f:
        for r in csv.DictReader(f):
            rows.append({k: float(v) for k, v in r.items()})

    if not rows:
        raise SystemExit("no rows")

    def S(k):
        return sum(r[k] for r in rows)

    lapr_mean = S("la_pr")
    report = []
    add = report.append
    add("=== P5 shadow association diagnosis ===")
    add(f"frames={len(rows)} attempted={S('attempted'):.0f}")
    add(f"quadrant matrix: LA_PA={S('la_pa'):.0f} "
        f"LA_PR={S('la_pr'):.0f} LR_PA={S('lr_pa'):.0f} "
        f"LR_PR={S('lr_pr'):.0f}")
    add(f"matrix sum={S('la_pa')+S('la_pr')+S('lr_pa')+S('lr_pr'):.0f} "
        f"(== attempted: {'OK' if abs(S('la_pa')+S('la_pr')+S('lr_pa')+S('lr_pr')-S('attempted'))<1 else 'MISMATCH'})")
    add(f"invalid: nonfinite={S('inv_nf'):.0f} negative={S('inv_neg'):.0f}")
    nlapr = lapr_mean if lapr_mean > 0 else 1.0
    add("LA_PR component summaries (mean over LA_PR):")
    add(f"  |r| mean={S('r_mean')/nlapr:.4f} min={min(r['r_min'] for r in rows if r['la_pr']>0) if any(r['la_pr']>0 for r in rows) else float('nan'):.4f} max={max(r['r_max'] for r in rows if r['la_pr']>0) if any(r['la_pr']>0 for r in rows) else float('nan'):.4f}")
    add(f"  sigma_assoc mean={S('s_mean')/nlapr:.4e}")
    add(f"  z=|r|/sqrt(var) mean={S('z_mean')/nlapr:.3f}")
    add(f"  plane_var mean={S('pv_mean')/nlapr:.4e}")
    add(f"  query_sensor_var mean={S('sv_mean')/nlapr:.4e}")
    add(f"  query_pose_rot_var mean={S('rv_mean')/nlapr:.4e}")
    add(f"  query_pose_pos_var mean={S('tv_mean')/nlapr:.4e}")
    wt_mean = (sum(r["cnt_mean_mean"] * r["la_pr"] for r in rows) / nlapr
               if lapr_sum else 0.0)
    wt_max = (sum(r["cnt_max_mean"] * r["la_pr"] for r in rows) / nlapr
              if lapr_sum else 0.0)
    add(f"  neighbor_count mean={wt_mean:.2f} max={wt_max:.2f} "
        f"(weighted by frame LA_PR)")
    add(f"  probe_rescued={S('probe_rescued'):.0f} "
        f"({100.0*S('probe_rescued')/nlapr if lapr_mean else 0:.1f}% of LA_PR)")

    add("\ncount-bin table (LA_PR candidates):")
    add("  bin        n      LA_PR  LA_PR_rate  plane_var_mean  z_mean")
    bin_labels = ["1", "2-4", "5-9", "10-14", "15-20"]
    for i, lab in enumerate(bin_labels):
        bn = S(f"bin{i+1}_n")
        bl = S(f"bin{i+1}_lapr")
        bz = S(f"bin{i+1}_z")
        bpv = S(f"bin{i+1}_pv")
        denom = bl if bl > 0 else 1.0
        add(f"  {lab:>5}  {bn:7.0f}  {bl:7.0f}  "
            f"{100.0*bl/bn if bn else 0:9.2f}%  {bpv/denom:13.4e}  {bz/denom:6.3f}")

    scored = []
    for idx, r in enumerate(rows):
        legacy = r["attempted"] - r["lr_pr"] - r["lr_pa"]
        frac = r["la_pr"] / legacy if legacy > 0 else 0.0
        scored.append((idx, r, r["la_pr"], frac))
    scored.sort(key=lambda x: (-x[2], -x[3]))

    add(f"\ntop {args.top} frames by LA_PR count:")
    add("  frame  timestamp      LA_PR  LA_PR/legacy_accept")
    for idx, r, lapr, frac in scored[: args.top]:
        add(f"  {idx:5d}  {r['timestamp']:14.6f}  {lapr:6.0f}  {frac:9.3f}")

    add(f"\ntop {args.top} frames by LA_PR/legacy fraction:")
    add("  frame  timestamp      LA_PR  LA_PR/legacy_accept")
    for idx, r, lapr, frac in sorted(scored, key=lambda x: -x[3])[: args.top]:
        add(f"  {idx:5d}  {r['timestamp']:14.6f}  {lapr:6.0f}  {frac:9.3f}")

    spikes = [idx for idx, r, lapr, _ in scored if lapr >= args.spike]
    if spikes:
        first = min(spikes)
        add(f"\nfirst spike frame (LA_PR>={args.spike}): {first} "
            f"(t={rows[first]['timestamp']:.6f}, LA_PR={rows[first]['la_pr']:.0f})")
        bursts = []
        cur = [spikes[0]]
        for s in spikes[1:]:
            if s - cur[-1] <= args.burst_gap:
                cur.append(s)
            else:
                bursts.append(cur)
                cur = [s]
        bursts.append(cur)
        add(f"disagreement bursts ({args.burst_gap}-gap): {len(bursts)}")
        for b in bursts[:10]:
            add(f"  frames {b[0]}..{b[-1]} (n={len(b)}, "
                f"total LA_PR={sum(rows[i]['la_pr'] for i in b):.0f})")
    else:
        add(f"\nno spike frames (LA_PR >= {args.spike})")

    text = "\n".join(report)
    print(text)
    if args.out:
        with open(args.out, "w") as f:
            f.write(text + "\n")
        print(f"report written: {args.out}")


if __name__ == "__main__":
    main()
