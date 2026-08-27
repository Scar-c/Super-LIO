# Round12 resume — DS takeover + durable base_ws

## Agent takeover

- previous agent: agent-codex
- expected HEAD: 6ddae81a1187fedcc1778a8c03bea70cf0f33fa3
- actual HEAD: 6ddae81a1187fedcc1778a8c03bea70cf0f33fa3 (matches)
- remote lag: origin/super-livo = 742b9c4 (previous agents not authorized to push; OK)
- Codex evidence docs/artifacts independently verified present (Round12 prompt49 +
  evidence docs + reference configs). No mismatch found.

## Owner decisions applied

- O12-1: keep BOTH Super-LIO revision baselines (paper-era 2c09212
  PUBLICATION_REPRODUCTION_BASELINE; current 60b57aa
  CURRENT_METHOD_AUTHOR_IMPLEMENTATION_BASELINE) — distinct in every table.
- O12-2: CONTINUE_FAST_LIVO2_PRISTINE_REPRODUCTION = YES (mcd4 RED not a blocker).
- SUPER_LIO_MCD_REVISION_SENSITIVITY_CONFIRMED (paper-era mcd2 0.9594 AMBER,
  current mcd2 0.7163 GREEN; paper-era mcd4 0.6978 GREEN, current mcd4 1.0210 RED).

## Durable base_ws

root: /home/lc/super_livo/base_ws (outside the Super-LIVO git repo)
- src/super_lio_upstream (60b57aa) / super_lio_paper_era (2c09212)
- src/fast_livo2_upstream (0d2c034) / fast_livo2_oxford (f2c9abb) / m3dgr (e0cf7d5)
- src/rpg_vikit (6c886c8)
- third_party/sophus (a621ff2) + third_party/sophus_build (pinned libSophus 60db4bc2)
- ws_fast_livo2/{src,build,devel} — FAST-LIVO2 catkin build (PASS)
- ws_super_lio/{src,build,devel} — pristine Super-LIO 60b57aa build (PASS)
- manifests/base_ws_manifest.yaml (revision identity for every parent; durable copy
  in docs/super_livo/evidence/base_ws_manifest.yaml)
- logs/

No canonical parent source/build/result lives under /tmp. The /tmp pristine trees
were git-relocated (revision identity preserved and verified).

## Builds

- FAST-LIVO2 0d2c034 + rpg_vikit 6c886c8 from base_ws: catkin_make PASS
  (fastlivo_mapping). Sophus discovery via base_ws sophus_build (compatibility-
  only; pinned a621ff2; no estimator source patch).
- Super-LIO 60b57aa from base_ws: catkin_make PASS (super_lio_node, lio).
