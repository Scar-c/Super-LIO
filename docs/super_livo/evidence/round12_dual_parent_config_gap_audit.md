# Round12 dual-parent configuration gap audit

Audit date: 2026-08-27. `original/` files and machine manifests live under
`.scratch/super-livo-v1/reference/upstream_configs/`.

| Parent × dataset | Specific config | Authority | YAML/launch/camera | Revision/branch/hash | Effective params | Hard-coded audit | Local runnable | Classification |
|---|---|---|---|---|---|---|---|---|
| Super-LIO × NTU | yes | METHOD_AUTHOR | yes/yes/N/A | complete | captured for eee/nya | complete | pristine build and runs complete | COMPLETE_REPRODUCIBLE |
| Super-LIO × MCD | yes | METHOD_AUTHOR | yes/yes/N/A; dataset calibration separate | complete | captured for day10/night08 at current and paper-era revisions | complete | pristine build and runs complete | COMPLETE_REPRODUCIBLE_REVISION_SENSITIVE |
| Super-LIO × Oxford | no | NOT_PUBLISHED | none | search pinned at `60b57aa` | N/A | method-global complete | no authoritative config | NOT_PUBLISHED |
| Super-LIO × M3DGR | no | NOT_PUBLISHED | none; M2DGR is not substituted | search pinned at `60b57aa` | N/A | method-global complete | no authoritative config | NOT_PUBLISHED |
| FAST-LIVO2 × NTU | yes | METHOD_AUTHOR | yes/yes/yes | complete | pending run | complete | pristine build verified; run stopped by Owner gate | PARTIAL_CONFIG |
| FAST-LIVO2 × MCD | no | NOT_FOUND | method defaults preserved only | search pinned at `0d2c034` | N/A | method-global complete | fallback needs explicit adaptation | NOT_PUBLISHED |
| FAST-LIVO2 × Oxford | yes | DATASET_AUTHOR_ADAPTED | yes/yes/yes | `config-used-OSD@f2c9abb`, complete | pending run | inherited method audit | run stopped by Owner gate | PARTIAL_CONFIG |
| FAST-LIVO2 × M3DGR | yes | DATASET_AUTHOR_ADAPTED | yes/yes/yes | `M3DGR@e0cf7d5`, complete | pending run | adapted-source comparison pending | run stopped by Owner gate | PARTIAL_CONFIG |

## Round11AB claim correction

The earlier registry wording “configs captured” was too broad. Exact raw files
were not yet stored for every parent/dataset pair, and Super-LIO's MCD/NTU
dataset configs were not given equal status with FAST-LIVO2. Round12 corrects
that record. A downloaded YAML is still only `PARTIAL_CONFIG` until pristine
source, runtime parameters, input identity, evaluator and local output close.

## MCD finding

`MCD_ATH.yaml` exists and is pinned; therefore Super-LIO × MCD is not
`NOT_FOUND`. Conversely, no credible FAST-LIVO2 MCD dataset config was found.
Its saved Avia/pinhole defaults are labeled
`FAST_LIVO2_METHOD_DEFAULT_FALLBACK`, never official MCD configuration.

## Revision provenance

- `PAPER_ERA_REVISION`: Super-LIO `2c09212` (initial ros1 release).
- `SUPER_LIVO_PARENT_REVISION`: `60b57aa`; it is also the merge-base of the
  current Super-LIVO branch with upstream ros1.
- `CURRENT_UPSTREAM_REVISION`: Super-LIO `60b57aa`; FAST-LIVO2 `0d2c034`.

Super-LIO config accuracy fields are stable from paper-era release to current,
but estimator/distortion source has later accuracy-related changes. Published
and current-source results remain separate provenance tiers.
