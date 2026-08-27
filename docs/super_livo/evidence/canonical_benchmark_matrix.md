# Canonical benchmark matrix

This is the project-level human-facing ledger. Detailed evaluator/config
provenance remains in `official_evaluator_and_baseline_registry`; historical
Round11/Round12 tables remain evidence and are not deleted.

| Dataset / sequence | Local / GT / evaluator | Super-LIO reference → pristine | FAST-LIVO2 reference/config → pristine | Super-LIVO historical | Metric / validity / notes |
|---|---|---|---|---|---|
| NTU eee_01 | AVAILABLE; Leica; dataset-author prism | paper 0.119 (`2c09212`) → 0.118876 current GREEN | paper 0.068/current 0.0271; method config → 0.0303 | B0/C0/A0 historical 0.1057/0.1024/0.0900; later B0 0.0271; D not reevaluated | translation ATE RMSE; parent results durable |
| NTU nya_01 | AVAILABLE; Leica; same | 0.069 → 0.062927 GREEN | 0.073/current 0.0356 → 0.0398 | 0.0642/0.0626/0.1468; later B0 0.0356 | same |
| NTU sbs_01 | AVAILABLE; Leica; same | paper 0.086 → NOT_RUN | authoritative NTU config/reference exact row pending → NOT_RUN | B0/C0/A0/A1 0.1040/0.1034/0.1101/0.1083 | parent reproduction gap |
| MCD day10/mcd2 | AVAILABLE_MULTI_BAG; discrete GT; OUR_REPRODUCTION | 0.721; current 0.7163 GREEN; paper-era 0.9594 AMBER | NO_AUTHORITATIVE_CONFIG / NOT_RUN | B0 1.2181; D 0.9044 | SE3 translation APE; revision-sensitive |
| MCD night08/mcd4 | AVAILABLE_MULTI_BAG; same | 0.604; current 1.0210 RED N=3; paper-era 0.6978 GREEN | NO_AUTHORITATIVE_CONFIG / NOT_RUN | B0 1.7416; D 1.9964 OWNER_ACCEPTED_AMBER | no retuning |
| Oxford Quarter01 | AVAILABLE; trajectory; evo max-diff .01 | NOT_PUBLISHED / NOT_RUN | dataset benchmark 0.04; adapted `f2c9abb` → 0.0397 GREEN | historical B0/D 0.0630/0.0629 (different .05 association) | snapshot/config/evaluator valid parent result |
| M3DGR Corridor01 | AVAILABLE; ArUco; dataset Python | NOT_PUBLISHED / NOT_RUN | 3.35; adapted `e0cf7d5` Avia → 3.03 GREEN | B0 15.4077; D 7.1549 | canonical snapshot/parity complete; legacy 5.83 diagnostic only; published sensor UNRESOLVED |
| M3DGR Corridor02 | AVAILABLE; ArUco; dataset Python | NOT_PUBLISHED / NOT_RUN | NO_AUTHORITATIVE_REFERENCE; adapted config → NOT_RUN | NOT_RUN | locally runnable optional gap |
| M3DGR Outdoor01 | AVAILABLE; RTK-position trajectory; evo | NOT_PUBLISHED / NOT_RUN | NO_AUTHORITATIVE_REFERENCE; adapted config → evaluator-recovered 0.240323 diagnostic, canonical NOT_RUN | historical result NOT_FOUND | contaminated attempts invalid; canonical rerun required |
| M3DGR Outdoor04 | AVAILABLE; RTK-position trajectory; evo | NOT_PUBLISHED / NOT_RUN | NO_AUTHORITATIVE_REFERENCE; adapted config → NOT_RUN | historical result NOT_FOUND | canonical sanity required |
| M2DGR street_03/m2s3 | NOT_AVAILABLE; published trajectory route | paper ≈0.139; local NOT_RUN | references retained; local NOT_RUN | NOT_RUN | mapping pending exact source recheck; no bag |
| M2DGR hall_01/m2h1 | NOT_AVAILABLE | paper ≈0.291; local NOT_RUN | references retained; local NOT_RUN | NOT_RUN | no bag |
| M2DGR door_02/m2d2 | NOT_AVAILABLE; txt is not bag | paper ≈0.321; local NOT_RUN | references retained; local NOT_RUN | NOT_RUN | `SUPER_LIO_M2_EXTRINSIC_CONFLICT_WITH_DATASET_CALIBRATION` |
| FAST_LIVO2 Bright_Screen_Wall | AVAILABLE; no accuracy GT | N/A | method special qualitative asset; NOT_RUN | N/A | LOCAL_SPECIAL_TEST_ASSET; non-blocking |

Provenance tiers are never collapsed: DATASET_AUTHOR_BENCHMARK,
DATASET_AUTHOR_ADAPTED_METHOD, METHOD_AUTHOR_PAPER,
METHOD_AUTHOR_CURRENT_OPEN_SOURCE, OUR_UPSTREAM_REPRODUCTION and
OUR_SUPER_LIVO_RUN remain explicit in the machine ledger.
