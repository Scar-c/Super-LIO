# Prob-LIO Generalization Result Ledger

Do not fabricate results. Each future dataset/sequence run appends one row
below once it has clean committed-source evidence. Rule:
`modify → test → commit → clean → canonical run → evaluate`.

Since Prompt12, full runtime output is written outside the checkout under
`/home/lc/super_livo/results/prob_lio_runtime/`. Git tracks only compact
exports under [`evidence/`](evidence/); trajectories and logs remain local and
are represented by hashes/row counts in each manifest.

## Ledger template

| dataset | sequence | bag/hash | config source | algorithm HEAD | production_code_tree_oid | git_dirty | association mode | map pose covariance mode | evaluator | ATE | completion | runtime | classification | notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|

## Planning order (current Owner plan — prompt10)

```text
MCD second target / NTU night_08
→ NTU VIRAL
→ Oxford
→ M3DGR
```

(If repository evidence later contains a newer explicit Owner order, that
order wins; provenance must be identified.)

## Canonical config for future runs (from SPEC §5A.8)

```text
cov_enable = ON
cov_validation_mode = light
map_pose_cov_model = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable = ON
p2p_weight_mode = prob_livo2
association_mode = super_legacy
prob_assoc_shadow_enable = OFF
```

Heavy/debug diagnostics OFF by default. Config provenance priority:
dataset/algorithm official config → frozen Super-LIO dataset config →
FAST-LIVO2 official config → default. No sweeps by default.

## Canonical `eee_01` entries (committed evidence; see `EVIDENCE_INDEX.md`)

| dataset | sequence | bag/hash | config source | algorithm HEAD | production_code_tree_oid | git_dirty | association mode | map pose covariance mode | evaluator | ATE | completion | runtime | classification | notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| NTU VIRAL | eee_01 | eee_01.bag (in-repo path, see run meta) | NTU.yaml frozen | `734839fb` | see `run_20260830_215616/meta.txt` | no | super_legacy | livo2_compat | official-compatible | 0.088831554 m | 3981/3329 | see meta | P4 canonical | trajectory `259d3fbc...` |
| NTU VIRAL | eee_01 | eee_01.bag | NTU.yaml frozen | `f56c376` | `48abc2c7` | no | super_legacy | livo2_compat | official-compatible | 0.088831554 m | 3981/3329 | see meta | P4 canonical (prompt9 A0) | trajectory `259d3fbc...`; shadow report in run dir |
| NTU VIRAL | eee_01 | eee_01.bag | NTU.yaml frozen | `f56c376` | `48abc2c7` | no | prob_livo2 | livo2_compat | official-compatible | 1.190814611 m | 3981/3329 | see meta | P5 experimental | applied P5 regression |
