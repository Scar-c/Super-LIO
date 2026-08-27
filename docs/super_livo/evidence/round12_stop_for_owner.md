# Round12 STOP FOR OWNER

## Outcome

Round12 configuration provenance, hard-coded parameter inventory, semantic
mapping, reference-base lineage, and pristine Super-LIO NTU/MCD evidence are
durable. Round12 is not closed: the mandatory reproduction gate ended in
`ROUND12_STOPPED_FOR_OWNER`.

## Trigger

Pristine current-upstream Super-LIO (`60b57aa`) on canonical MCD night08 / paper
`mcd4` produced APE RMSE `1.0210 m` against the published `0.604 m`. Three
identical runs were bitwise identical (trajectory SHA256
`9a27828eeba69720904d761d9b666a08a805511183ffc4971fa4e9bd9b628582`),
so min/median/max are all `1.0210 m` and range is `0`.

The paper-era revision (`2c09212`) with its immutable authoritative config
produced `0.6978 m` and is GREEN. This establishes revision sensitivity but does
not erase the current-upstream RED result. No parameter was changed or swept.

Prompt49 sections 39 and 50 require `STOP FOR OWNER` when a reproduction remains
RED after provenance diagnosis and identical repeats. Consequently the
FAST-LIVO2 NTU, Oxford and M3DGR runs were not started. The pinned FAST-LIVO2
tree was only brought to a verified clean build state.

## Owner decisions needed to resume

1. Select whether the publication/reference baseline for Super-LIO is the
   paper-era revision, current upstream, or both as explicitly revision-scoped
   baselines.
2. Authorize continuation of the remaining pristine FAST-LIVO2 reproduction
   matrix despite the current-upstream Super-LIO night08 RED result.

No visual functional optimization, accuracy-parameter tuning, or offset sweep
was performed.

## Prompt hygiene requested during execution

The Owner instructed that raw prompt files must be removed after canonical
registration. The raw Round11AB and Round12 prompt files were verified/registered
and removed. `prompts/` now has no top-level raw prompt file other than
`README.md`; Prompt48 and Prompt49 remain in canonical numbered locations.
