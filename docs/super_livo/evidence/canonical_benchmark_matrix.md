# Canonical benchmark matrix

This is the human-facing projection of the provenance-normalized v2 machine
ledger. A number is not an “official value” by itself: method-author paper,
method-author current source, dataset-author benchmark, config, evaluator, and
our reproduction remain separate records in the YAML catalog.

| Dataset | Sequence | Local bag | GT/Eval | Super-LIO method-author ref | Super-LIO dataset-author ref | pristine Super-LIO | FAST-LIVO2 method-author ref | FAST-LIVO2 dataset-author ref | pristine FAST-LIVO2 | Super-LIVO B0 | C0 | A0 | A1 | D | Visual |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| NTU | eee_01 | AVAILABLE | Leica prism; dataset-author interpolation; SE3; translation ATE RMSE | paper Table I 0.119 m (A) | NOT_FOUND | 0.118876 m, canonical | paper 0.068 m + current-source 0.0271 m (both B) | NOT_FOUND | 0.0303 m, canonical | 0.1057 legacy | 0.1024 legacy | 0.0900 legacy | NOT_RUN | NOT_REEVALUATED | NOT_RUN |
| NTU | nya_01 | AVAILABLE | same | paper 0.069 m (A) | NOT_FOUND | 0.062927 m, canonical | paper 0.073 m + current-source 0.0356 m (both B) | NOT_FOUND | 0.0398 m, canonical | 0.0642 legacy | 0.0626 legacy | 0.1468 legacy | NOT_RUN | NOT_REEVALUATED | NOT_RUN |
| NTU | sbs_01 | AVAILABLE | same | paper 0.086 m (A) | NOT_FOUND | 0.084422872 m, canonical GREEN | paper Table II 0.062 m + current-source 0.0234 m (both B) | NOT_FOUND | 0.028766780 m, canonical GREEN against current source | 0.1040 legacy | 0.1034 legacy | 0.1101 legacy | 0.1083 legacy | NOT_RUN | NOT_RUN |
| MCD | ntu_day_10 / mcd2 | AVAILABLE_MULTI_BAG | discrete body GT; project evaluator; SE3 APE | paper 0.721 m (B) | NOT_FOUND | current 0.7163 m canonical; paper-era 0.9594 m | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | 1.2181 legacy | NOT_RUN | NOT_RUN | NOT_RUN | 0.9044 legacy | NOT_RUN |
| MCD | ntu_night_08 / mcd4 | AVAILABLE_MULTI_BAG | same | paper 0.604 m (B) | NOT_FOUND | current 1.0210 m canonical; paper-era 0.6978 m | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | 1.7416 legacy | NOT_RUN | NOT_RUN | NOT_RUN | 1.9964 legacy | NOT_RUN |
| Oxford | Quarter01 | AVAILABLE | dataset-author evo SE3, max diff 0.01 s | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | NOT_FOUND | dataset benchmark 0.04 m (A) | 0.0397 m canonical | 0.0630 legacy, noncanonical 0.05 s association | NOT_RUN | NOT_RUN | NOT_RUN | 0.0629 legacy, noncanonical association | NOT_RUN |
| M3DGR | Corridor01 | AVAILABLE | ArUco first-to-last relative translation | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | NOT_FOUND | 3.35 m (A; sensor attribution unresolved) | 3.03 m canonical | 15.4077 m | NOT_RUN | NOT_RUN | NOT_RUN | 7.1549 m | NOT_RUN |
| M3DGR | Corridor02 | AVAILABLE | same | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | NOT_FOUND | NO_AUTHORITATIVE_REFERENCE | INVALID_ESTIMATOR_DIVERGENCE (22.9 km diagnostic; not consumed) | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| M3DGR | Outdoor01 | AVAILABLE | RTK trajectory; dataset evo SE3 | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | NOT_FOUND | NO_AUTHORITATIVE_REFERENCE | 0.239674 m canonical | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_RUN |
| M3DGR | Outdoor04 | AVAILABLE | RTK trajectory; dataset evo SE3 | NOT_FOUND | NOT_FOUND | NOT_RUN_NO_AUTHORITATIVE_CONFIG | NOT_FOUND | NO_AUTHORITATIVE_REFERENCE | 0.623458 m canonical | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_FOUND | NOT_RUN |
| M2DGR | m2s3 (dataset alias unresolved) | NOT_AVAILABLE | trajectory/evo if obtained | paper Table I 0.139 m (UNRESOLVED) | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_FOUND | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| M2DGR | m2h1 (dataset alias unresolved) | NOT_AVAILABLE | same | paper Table I 0.291 m (UNRESOLVED) | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_FOUND | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| M2DGR | m2d2 (dataset alias unresolved) | NOT_AVAILABLE | same | paper Table I 0.321 m (UNRESOLVED) | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_FOUND | NOT_FOUND | NOT_RUN_LOCAL_DATA_MISSING | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| FAST-LIVO2 | Bright_Screen_Wall | AVAILABLE | no accuracy GT | NOT_AVAILABLE | NOT_AVAILABLE | NOT_AVAILABLE | qualitative method asset | NOT_AVAILABLE | NOT_RUN | NOT_AVAILABLE | NOT_AVAILABLE | NOT_AVAILABLE | NOT_AVAILABLE | NOT_AVAILABLE | NOT_RUN |

“legacy” means `LEGACY_NO_POST_RESOLVE_SNAPSHOT`; it is not silently upgraded
to current canonical evidence. M2 paper aliases are retained exactly as printed,
but no street/hall/door mapping is asserted. The unresolved M2 extrinsic conflict
also remains recorded in the machine ledger.

Current policies: `M3DGR_PRIMARY_BENCHMARK = KEEP`; M2DGR is not blacklisted,
but has no local bag and therefore is not scheduled.
