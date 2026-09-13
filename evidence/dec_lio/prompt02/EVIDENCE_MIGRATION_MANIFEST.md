# Evidence migration manifest

Prompt02 moved historical evidence into explicit Prompt00R and Prompt01
namespaces. Every move below preserves the Git blob identity; no evidence
content was edited. New Prompt02 evidence is placed under `prompt02/`.

| old path | new path | old/new blob |
|---|---|---|
| `evidence/dec_lio/build_identity.txt` | `evidence/dec_lio/prompt00r/build_identity.txt` | `5d53014635b6373c56b27077f6fd65e379054a53` |
| `evidence/dec_lio/dataset_identity.txt` | `evidence/dec_lio/prompt00r/dataset_identity.txt` | `a18e329b6a6e32f860e951ac7d614d9dd00f91af` |
| `evidence/dec_lio/evaluation.txt` | `evidence/dec_lio/prompt00r/evaluation.txt` | `ca2c7cc4a2c5ae3d381f33d12fb65d4b6bffb61e` |
| `evidence/dec_lio/exact_parity.txt` | `evidence/dec_lio/prompt00r/exact_parity.txt` | `15c88bbb40895de8ddd247457189f7a9058f05fd` |
| `evidence/dec_lio/prompt00r_authority_correction.txt` | `evidence/dec_lio/prompt00r/prompt00r_authority_correction.txt` | `845f9bd47bd0be2b310fe4728b140670b2e6b75d` |
| `evidence/dec_lio/prompt00r_closure.txt` | `evidence/dec_lio/prompt00r/prompt00r_closure.txt` | `9bfa323f5bbe75f4e89e0b0d261693ce918a0c9a` |
| `evidence/dec_lio/repository_identity.txt` | `evidence/dec_lio/prompt00r/repository_identity.txt` | `d8212b568eca6e069ad7d30b0bd7f3f530848e3b` |
| `evidence/dec_lio/run_offline_01.txt` | `evidence/dec_lio/prompt00r/run_offline_01.txt` | `2b5c5711b746ab28af7e9cdeacf0d218d64e7799` |
| `evidence/dec_lio/run_offline_02.txt` | `evidence/dec_lio/prompt00r/run_offline_02.txt` | `cbb711032742ac4123abcb3002aea83a64e31da5` |
| `evidence/dec_lio/run_online.txt` | `evidence/dec_lio/prompt00r/run_online.txt` | `5b3f9481b6132689ffab4406c2837a09d1b7e5bb` |
| `evidence/dec_lio/d1_runtime_bridge.txt` | `evidence/dec_lio/prompt01/d1_runtime_bridge.txt` | `473fd9133fec272e9b25e073bc229650e148ce35` |
| `evidence/dec_lio/d1_runtime_stairs.txt` | `evidence/dec_lio/prompt01/d1_runtime_stairs.txt` | `2f52d2bb15acf9370166711a8eaad25efd6be145` |
| `evidence/dec_lio/d1_synthetic_reference.txt` | `evidence/dec_lio/prompt01/d1_synthetic_reference.txt` | `45f3945ead9cc057bfbbfd17e70961706a062e8e` |
| `evidence/dec_lio/prompt01_build_identity.txt` | `evidence/dec_lio/prompt01/prompt01_build_identity.txt` | `24b6b2e75457e60e339cf37e69618667e7026b68` |
| `evidence/dec_lio/prompt01_closure.txt` | `evidence/dec_lio/prompt01/prompt01_closure_full.txt` | `deb96c792346c870743aa610e978da38b282a777` |
| `evidence/dec_lio/stairs_dataset_identity.txt` | `evidence/dec_lio/prompt01/stairs_dataset_identity_full.txt` | `943989a716fbb8b4efd961286dba48113a2212ad` |
| `evidence/dec_lio/stairs_d0_exact_parity.txt` | `evidence/dec_lio/prompt01/stairs_native_exact_parity_full.txt` | `785ea1da669ade4cf1f9fad17a6abb65bc59c3d9` |

The old paths are absent and the top-level `evidence/dec_lio/` contains only
its README and index after migration.
