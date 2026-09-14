# Stairs U-trace

Canonical input: `stairs_alpha.bag`, `geode_stairs_alpha.yaml`, official
`stairs_alpha.txt`; fixed 0.10 s association, one global SE(3) alignment, no
crop or scale. Final trajectory SHA:
`543719f6c3aff530c98298fea70a24f8acf3562e340621db67e915432a8dac78`.

| metric | U-trace |
|---|---:|
| translation RMSE m | 0.289370 |
| translation median m | 0.246879 |
| translation P95 m | 0.477001 |
| rotation RMSE deg | 3.811854 |
| rotation median deg | 3.413935 |
| rotation P95 deg | 5.358497 |

Active fraction: `0.671368`; `alpha_trace` median `0.999111686`; gamma_w
median `0.823130`; valid fraction `1.0`. The run RC was 0 and the second
determinism run was byte-identical.
