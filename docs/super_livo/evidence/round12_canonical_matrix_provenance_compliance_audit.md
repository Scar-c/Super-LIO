# Round 12 canonical-matrix provenance compliance audit

Date: 2026-08-28

## Scope and method

The audit covered all 14 known ledger rows and all four external-reference
classes per row (two parents × method-author/dataset-author), for 56 explicit
reference cells. Empty knowledge is represented by an explicit status in the
human table and an empty typed list in the machine ledger; it is never a blank
or an inferred zero.

The v2 ledger separates result provenance, config provenance, and evaluator
provenance. Every numeric external reference has algorithm, dataset, sequence,
value, metric, alignment semantics, source type/owner/title, paper or repository,
revision, exact location, and comparability. Every numeric reproduction has its
revision, config provenance, effective snapshot, evaluator, metric, and validity.
The public fail-closed validator is
`scripts/super_livo/evaluation/validate_canonical_benchmark_matrix.py`.

## Findings

- Rows audited: 14.
- Reference cells audited: 56.
- Numeric method-author reference records found: 14.
- Numeric dataset-author reference records found: 2 (Oxford Quarter01 and
  M3DGR Corridor01).
- Multi-source rows: 3. NTU eee_01, nya_01, and sbs_01 retain both the
  FAST-LIVO2 paper and current-open-source values.
- Prose-only numeric cells replaced: all numeric cells now point to complete
  catalog records; no bare `0.04 m`-style machine entry remains.
- Ambiguities corrected: FAST-LIVO2 `sbs_01` is paper Table II `0.062 m`, while
  the pinned current-source README reports `0.0234 m`; these are distinct B-class
  references. M3 Corridor01 uses the ArUco endpoint translation metric despite
  the over-general paper-table heading. M2 values remain under the paper aliases
  `m2s3`, `m2h1`, and `m2d2`; street/hall/door mappings are not asserted.
- Historical Super-LIVO results without post-resolve evidence remain explicitly
  `LEGACY_NO_POST_RESOLVE_SNAPSHOT`; current evidence quality was not backfilled.

## Remaining unresolved provenance

- No numeric NTU dataset-author benchmark was found for either parent. The
  dataset-author contribution is the evaluator/calibration protocol, not a score.
- No authoritative FAST-LIVO2 Corridor02 number was found in the registered
  primary-source evidence.
- Super-LIO has no authoritative published M3DGR config; M3 parent reproductions
  therefore remain not run.
- M2 dataset aliases are unresolved and local bags are unavailable. The
  `SUPER_LIO_M2_EXTRINSIC_CONFLICT_WITH_DATASET_CALIBRATION` note remains open.
- M3 Corridor01 published sensor attribution remains unresolved; its reference
  is retained with that caveat.

These unresolved items are explicit statuses/caveats, not schema omissions.

`CANONICAL_MATRIX_PROVENANCE_COMPLIANCE = PASS`
