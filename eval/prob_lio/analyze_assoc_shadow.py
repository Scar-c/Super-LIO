#!/usr/bin/env python3
"""Prob-LIO P5 shadow association diagnosis report (P9: iteration-resolved).

Consumes the per-(frame,iteration) summary CSV produced by the shadow run
(assoc_shadow_frames.csv under the run directory) and emits a compact report:

  - input integrity validation (G-P9.T5): rejects corrupt lifecycle input
  - total four-way quadrant matrix (sum == attempted)
  - exact iteration accounting (G-P9.T2): per-frame iterations_executed,
    contiguous legal obs_iter, sum(iterations_executed) == records
  - per-iteration quadrant matrix and lifecycle counters
  - top N frames by LA_PR count / rate (RANKING view)
  - first spike / consecutive bursts (CHRONOLOGY view: sorted strictly by
    frame_id; bursts are temporal adjacencies, never score-derived)
  - LA_PR component summaries (weighted by frame LA_PR)
  - count-bin table

CLI-parameterized; no dataset hard-coding.

Usage:
  analyze_assoc_shadow.py <frames.csv> [--out report.txt] [--top 10]
                          [--spike 100] [--burst-gap 5]
"""
import argparse
import csv
import sys


def validate(rows):
    """G-P9.T5: reject corrupt lifecycle input before any conclusion.

    Checks (each violation -> nonzero exit, no report):
      1. frame_id / obs_iter parse as integers
      2. no duplicate (frame_id, obs_iter)
      3. legal contiguous iteration progression per frame (1..max)
      4. stable timestamp inside one frame
      5. frame identity does not reset to default mid-run (frame_id
         non-decreasing, no backwards jumps)
      6. timestamp does not reset unexpectedly (non-decreasing within frame)
    """
    errs = []
    seen = set()
    last_fid = -1
    last_ts = None
    prev_iter = 0
    i = 0
    while i < len(rows):
        r = rows[i]
        try:
            fid = int(r["frame_id"])
        except (ValueError, KeyError) as e:
            errs.append(f"row {i}: frame_id not an integer ({e})")
            i += 1
            continue
        try:
            it = int(r["obs_iter"])
        except (ValueError, KeyError) as e:
            errs.append(f"row {i}: obs_iter not an integer ({e})")
            i += 1
            continue
        try:
            ts = float(r["timestamp"])
        except (ValueError, KeyError) as e:
            errs.append(f"row {i}: timestamp not a number ({e})")
            i += 1
            continue
        if (fid, it) in seen:
            errs.append(f"duplicate (frame_id, obs_iter) = ({fid}, {it})")
        seen.add((fid, it))
        if fid < last_fid:
            errs.append(
                f"frame identity went backwards: {last_fid} -> {fid} at row {i}")
        if fid == last_fid and it != prev_iter + 1:
            errs.append(
                f"non-contiguous iteration for frame {fid}: expected "
                f"{prev_iter + 1} got {it}")
        if fid == last_fid and abs(ts - last_ts) > 1e-9:
            errs.append(
                f"timestamp changed inside frame {fid}: {last_ts} -> {ts}")
        if fid == 0 and last_fid > 0:
            # frame_id reset to the default mid-run (frame 0 is legitimate
            # only as the first frame of the run)
            errs.append(f"frame identity reset to default 0 at row {i}")
        last_fid = fid
        last_ts = ts
        prev_iter = it
        r["_frame_id"] = fid
        r["_obs_iter"] = it
        i += 1
    return errs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--out", default=None)
    ap.add_argument("--top", type=int, default=10)
    ap.add_argument("--spike", type=int, default=100,
                    help="LA_PR count at which a frame is a 'spike'")
    ap.add_argument("--burst-gap", type=int, default=5,
                    help="frame-id gap that separates bursts")
    args = ap.parse_args()

    raw = []
    with open(args.csv) as f:
        for r in csv.DictReader(f):
            raw.append({k: v for k, v in r.items()})
    if not raw:
        raise SystemExit("no rows")

    errs = validate(raw)
    if errs:
        print("INTEGRITY FAILURE (G-P9.T5): refusing to publish lifecycle "
              "conclusions", file=sys.stderr)
        for e in errs[:20]:
            print(f"  - {e}", file=sys.stderr)
        if len(errs) > 20:
            print(f"  ... ({len(errs)} violations total)", file=sys.stderr)
        raise SystemExit(2)

    # Rows passed validation; convert to numeric where the report needs it.
    rows = []
    for r in raw:
        num = {}
        for k, v in r.items():
            if k in ("_frame_id", "_obs_iter"):
                continue
            try:
                num[k] = float(v)
            except ValueError:
                num[k] = v
        rows.append(num)

    # Chronology: rows are already in (frame_id, obs_iter) order in the CSV;
    # enforce it defensively.
    rows.sort(key=lambda r: (r["frame_id"], r["obs_iter"]))

    def S(k):
        return sum(r[k] for r in rows)

    report = []
    add = report.append
    nlapr = S("la_pr") if S("la_pr") > 0 else 1.0

    add("=== P5 shadow association diagnosis (iteration-resolved) ===")
    add(f"records={(len(rows))} (frame,iteration) rows; "
        f"frames={len(set(int(r['frame_id']) for r in rows))}; "
        f"iterations={sorted(set(int(r['obs_iter']) for r in rows))}")

    # G-P9.T2 exact iteration accounting.
    frames_iter = {}
    for r in rows:
        fid = int(r["frame_id"])
        frames_iter[fid] = max(frames_iter.get(fid, 0), int(r["obs_iter"]))
    sum_exec = sum(frames_iter.values())
    add(f"iteration accounting: sum(iterations_executed)="
        f"{sum_exec} == records={len(rows)} "
        f"({'OK' if sum_exec == len(rows) else 'MISMATCH'})")
    need_cv = sum(1 for fid, n in frames_iter.items() if n > 2)
    add(f"frames reaching need_converge (iter>2): {need_cv}")

    add(f"attempted={S('attempted'):.0f}")
    add(f"quadrant matrix: LA_PA={S('la_pa'):.0f} "
        f"LA_PR={S('la_pr'):.0f} LR_PA={S('lr_pa'):.0f} "
        f"LR_PR={S('lr_pr'):.0f}")
    add(f"matrix sum={S('la_pa')+S('la_pr')+S('lr_pa')+S('lr_pr'):.0f} "
        f"(== attempted: "
        f"{'OK' if abs(S('la_pa')+S('la_pr')+S('lr_pa')+S('lr_pr')-S('attempted'))<1 else 'MISMATCH'})")
    add(f"invalid: nonfinite={S('inv_nf'):.0f} negative={S('inv_neg'):.0f}")
    add(f"lifecycle: reject_from_active={S('rej_active'):.0f} "
        f"late(need_converge)={S('rej_late'):.0f} "
        f"sticky(need_converge)={S('sticky'):.0f} "
        f"flip={S('flip'):.0f} "
        f"accept_to_reject={S('acc2rej'):.0f} "
        f"reject_to_accept={S('rej2acc'):.0f} "
        f"sticky_skip(prior prob reject)={S('sticky_skip'):.0f} "
        f"counterfactual_reaccept={S('counterfactual'):.0f}")
    add("  NOTE: the ESKF breaks on convergence (quit_eps), so the "
        "need_converge iteration rarely executes;")
    add("  'final-iteration' decisions (the ones that determine the state) "
        "are analyzed per frame below.")

    # Final-phase analysis: for each frame, the LAST executed iteration's
    # decisions are final (loop-break semantics). The final iteration can be
    # iter<4 with need_converge=0.
    frames_final = {}
    for r in rows:
        fid = int(r["frame_id"])
        cur = frames_final.get(fid)
        if cur is None or int(r["obs_iter"]) > int(cur["obs_iter"]):
            frames_final[fid] = r
    fr = list(frames_final.values())
    add("\nfinal-iteration-per-frame analysis "
        f"(frames={len(fr)}):")
    add(f"  attempted={sum(r['attempted'] for r in fr):.0f} "
        f"LA_PR={sum(r['la_pr'] for r in fr):.0f} "
        f"rej_active={sum(r['rej_active'] for r in fr):.0f} "
        f"flip={sum(r['flip'] for r in fr):.0f}")
    add("  iteration histogram of final decisions:")
    hist = {}
    for r in fr:
        it = int(r["obs_iter"])
        hist[it] = hist.get(it, 0) + 1
    for it in sorted(hist):
        add(f"    final iter {it}: {hist[it]} frames")

    add("\nper-iteration summary:")
    add("  iter need_converge attempted  LA_PA   LA_PR   LR_PA   LR_PR "
        " sticky  flip  acc2rej  rej2acc  sticky_skip  counterfactual")
    for it in sorted(set(int(r["obs_iter"]) for r in rows)):
        itr = [r for r in rows if int(r["obs_iter"]) == it]
        nc = itr[0]["need_converge"]
        add(f"  {it:3d}  {int(nc):11d} {sum(r['attempted'] for r in itr):9.0f}"
            f" {sum(r['la_pa'] for r in itr):7.0f}"
            f" {sum(r['la_pr'] for r in itr):7.0f}"
            f" {sum(r['lr_pa'] for r in itr):7.0f}"
            f" {sum(r['lr_pr'] for r in itr):7.0f}"
            f" {sum(r['sticky'] for r in itr):7.0f}"
            f" {sum(r['flip'] for r in itr):6.0f}"
            f" {sum(r['acc2rej'] for r in itr):8.0f}"
            f" {sum(r['rej2acc'] for r in itr):8.0f}"
            f" {sum(r['sticky_skip'] for r in itr):11.0f}"
            f" {sum(r['counterfactual'] for r in itr):14.0f}")

    def wmean(col):
        return sum(r[col] * r["la_pr"] for r in rows) / nlapr

    add("\nLA_PR component summaries (weighted by frame LA_PR):")
    add(f"  |r| mean={wmean('r_mean'):.4f}")
    add(f"  sigma_assoc mean={wmean('s_mean'):.4e}")
    add(f"  z=|r|/sqrt(var) mean={wmean('z_mean'):.3f}")
    add(f"  plane_var mean={wmean('pv_mean'):.4e}")
    add(f"  query_sensor_var mean={wmean('sv_mean'):.4e}")
    add(f"  query_pose_rot_var mean={wmean('rv_mean'):.4e}")
    add(f"  query_pose_pos_var mean={wmean('tv_mean'):.4e}")
    wt_mean = sum(r["cnt_mean_mean"] * r["la_pr"] for r in rows) / nlapr
    wt_max = sum(r["cnt_max_mean"] * r["la_pr"] for r in rows) / nlapr
    add(f"  neighbor_count mean={wt_mean:.2f} max={wt_max:.2f} "
        f"(weighted)")
    add(f"  probe_rescued={S('probe_rescued'):.0f} "
        f"({100.0*S('probe_rescued')/nlapr if nlapr else 0:.1f}% of LA_PR)")

    # Count-bin table (per frame, iter rows).
    add("\ncount-bin table (LA_PR candidates):")
    add("  bin        n      LA_PR  LA_PR_rate  plane_var_mean  z_mean")
    bin_labels = ["1", "2-4", "5-9", "10-14", "15-20"]
    for i, lab in enumerate(bin_labels):
        bn = S(f"bin{i+1}_n")
        bl = S(f"bin{i+1}_lapr")
        bz = sum(r[f"bin{i+1}_z"] * r[f"bin{i+1}_lapr"] for r in rows)
        bpv = sum(r[f"bin{i+1}_pv"] * r[f"bin{i+1}_lapr"] for r in rows)
        denom = bl if bl > 0 else 1.0
        add(f"  {lab:>5}  {bn:7.0f}  {bl:7.0f}  "
            f"{100.0*bl/bn if bn else 0:9.2f}%  {bpv/denom:13.4e}  {bz/denom:6.3f}")

    # RANKING view: top frames by LA_PR count and by rate (frame-level).
    frames = {}
    for r in rows:
        fid = int(r["frame_id"])
        f = frames.setdefault(fid, {"attempted": 0.0, "la_pr": 0.0,
                                    "lr_pr": 0.0, "lr_pa": 0.0,
                                    "timestamp": r["timestamp"]})
        f["attempted"] += r["attempted"]
        f["la_pr"] += r["la_pr"]
        f["lr_pr"] += r["lr_pr"]
        f["lr_pa"] += r["lr_pa"]
    flist = sorted(frames.items())
    ranked = []
    for fid, f in flist:
        legacy = f["attempted"] - f["lr_pr"] - f["lr_pa"]
        frac = f["la_pr"] / legacy if legacy > 0 else 0.0
        ranked.append((fid, f, frac))
    ranked_count = sorted(ranked, key=lambda x: (-x[1]["la_pr"], -x[2]))
    ranked_rate = sorted(ranked, key=lambda x: -x[2])

    add(f"\ntop {args.top} frames by LA_PR count (ranking):")
    add("  frame  timestamp      LA_PR  LA_PR/legacy_accept")
    for fid, f, frac in ranked_count[: args.top]:
        add(f"  {fid:5d}  {f['timestamp']:14.6f}  {f['la_pr']:6.0f}  {frac:9.3f}")

    add(f"\ntop {args.top} frames by LA_PR/legacy fraction (ranking):")
    add("  frame  timestamp      LA_PR  LA_PR/legacy_accept")
    for fid, f, frac in ranked_rate[: args.top]:
        add(f"  {fid:5d}  {f['timestamp']:14.6f}  {f['la_pr']:6.0f}  {frac:9.3f}")

    # CHRONOLOGY view: spikes and bursts strictly by frame_id order.
    spike_frames = [fid for fid, f, _ in ranked if f["la_pr"] >= args.spike]
    if spike_frames:
        add(f"\nfirst spike frame (LA_PR>={args.spike}): {spike_frames[0]} "
            f"(LA_PR={frames[spike_frames[0]]['la_pr']:.0f})")
        bursts = []
        cur = [spike_frames[0]]
        for s in spike_frames[1:]:
            if s - cur[-1] <= args.burst_gap:
                cur.append(s)
            else:
                bursts.append(cur)
                cur = [s]
        bursts.append(cur)
        add(f"disagreement bursts ({args.burst_gap}-gap, chronology): "
            f"{len(bursts)}")
        for b in bursts[:10]:
            add(f"  frames {b[0]}..{b[-1]} (n={len(b)}, "
                f"total LA_PR={sum(frames[i]['la_pr'] for i in b):.0f})")
        for b in bursts:
            if b[0] > b[-1]:
                add(f"  WARNING: impossible burst range {b[0]}..{b[-1]}")
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
