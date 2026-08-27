# Round12 final gap audit and closure

Audited 2026-08-28 after the two canonical M3DGR Outdoor controls.

| Item | Local data | Authoritative config | Decision | Reason |
|---|---|---|---|---|
| M3 Corridor01 | yes | dataset-author adapted FAST-LIVO2 | COMPLETE | Existing snapshot/parity-complete canonical run is 3.03 m; no rerun permitted. |
| M3 Corridor02 | yes | adapted config, but no authoritative score | JUSTIFIED_NOT_RUN_NON_BLOCKING | Two longer Outdoor controls and Corridor01 already close M3 health; another 293 s run has no paper-reproduction target and is optional under Prompt52 resource policy. |
| NTU sbs_01 Super-LIO | yes | method-author `NTU.yaml`, pinned `60b57aa` | JUSTIFIED_NOT_RUN_TRANSACTION_ADAPTER_MISSING | The available legacy runner uses PID-only cleanup and lacks the immutable transaction/PGID/exclusivity contract now mandatory for every new full-bag run. |
| NTU sbs_01 FAST-LIVO2 | yes | method-author NTU config, pinned `0d2c034` | JUSTIFIED_NOT_RUN_TRANSACTION_ADAPTER_MISSING | Same gate. A quick legacy run would not be canonical evidence. |

The sbs bag is not missing and the algorithms are not classified by the
absence of these runs. Existing Super-LIVO B0/C0/A0/A1 values remain historical
project results only. A future sbs parent run first requires a tested,
dual-parent NTU transaction adapter with isolated master, PID/PGID ownership,
snapshot/config parity, output delivery, official VIRAL evaluation, and
cleanup verification.

M3DGR policy is `KEEP`: Outdoor01 (`0.239674 m`, 99.887% coverage) and
Outdoor04 (`0.623458 m`, 99.956% coverage) are both `CANONICAL_VALID` and
`SANITY_HEALTHY`. Corridor02 is therefore non-blocking. M2DGR remains
`NOT_RUN_LOCAL_DATA_MISSING`, not experimentally unhealthy.

All canonical trajectory, snapshot, parity and evaluation artifacts are under
`/home/lc/super_livo/results`; no canonical result consumes `/tmp`. The
previous pristine Super-LIO source checkout was temporary, but its durable
artifacts record the pinned revision and hashes, so the result does not depend
on the continued existence of that checkout.

Round12 classification: `ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED`.
