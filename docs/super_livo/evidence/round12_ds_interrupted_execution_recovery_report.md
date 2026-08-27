# Round12 DS interrupted execution recovery report

Previous executor: `agent-ds`  
Takeover executor: `agent-codex`

Previous reported HEAD: `ce22140fa9dae6529a3e7a227e6c6f3b652729d6`  
Actual recovered frontier: `4b678ffdb17f2c83ccf28f3e63322894f12d7c53`

`HEAD_MATCH = NO`. The difference is expected and evidence-bearing: DS/Codex
work after the earlier report committed M3 transaction infrastructure,
official/runtime parity, canonical Corridor01, Outdoor corrective evidence and
the Dynamic01-data stop. At takeover the tracked worktree was clean. The only
untracked repository file was the Owner's Prompt52, now registered canonically.

## DS work recovered

- `47ceb86`: M3 official/runtime parity checker and TDD.
- `ff8f822`: experiment transaction contract evidence.
- `2ef3835`: snapshot-complete dataset-author adapted FAST-LIVO2 Corridor01,
  ArUco final relative translation `3.03 m` versus `3.35 m`, canonical GREEN.
- `4437556`: transaction lifecycle/exclusivity corrective after the User found
  concurrent Outdoor01 playback.
- `6f5f2ea`: clean Outdoor01 sanity evidence and tested FAST-LIVO2 debug-to-TUM
  conversion.
- `4b678ff`: prior stop pending nonexistent Dynamic01 data. Prompt52 supersedes
  that scheduling assumption.

External, intentionally untracked state exists under `base_ws/` (pinned M3DGR
upstream/build/runtime outputs and workspace entrypoints) and
`results/upstream_reproduction/` (immutable run directories). No external
result is canonized merely because it exists.

## Attempts

| Attempt/run directory | Snapshot/parity/playback/output | Engineering status | Scientific validity |
|---|---|---|---|
| legacy F4, prior `5.83 m` | effective snapshot missing/wrong legacy fork provenance | `DIAGNOSTIC_VALID_NOT_CANONICAL` | legacy diagnostic only; no color |
| known relative official-config path attempt | parity tool raised `FileNotFoundError`; playback blocked | `CONFIG_PATH_FAIL` | invalid |
| known stale master/mapping/duplicate `/republish` attempts | old process or duplicate-node replacement prevented a clean transaction | `PROCESS_LIFECYCLE_FAIL` / `ROS_MASTER_FAIL` | invalid |
| known parity-PASS but zero-row `mat_out.txt` attempt | playback completed; output empty | `OUTPUT_FAIL` | invalid |
| `corridor01/20260827T154937Z_cee38330` | snapshot PASS; parity PASS; full playback; 16,023 rows; ArUco evaluated; cleanup logged | `CANONICAL_VALID` | GREEN, `3.03 m` |
| `dynamic01/20260827T160930Z_b0a73ef0` | local bag missing before ROS | `STATIC_PREFLIGHT_FAIL` | invalid; sequence not locally schedulable |
| `outdoor01/20260827T155950Z_cdf80723` | originally completed, later invalidated by overlapping shared-output incident | `PROCESS_LIFECYCLE_FAIL` | contaminated, withdrawn |
| `outdoor01/20260827T161453Z_56740a53` | concurrent playback; output non-monotonic | `OUTPUT_FAIL` plus lifecycle contamination | invalid |
| `outdoor01/20260827T161702Z_d8d41d0f` | concurrent playback; state stopped at playback | `PROCESS_LIFECYCLE_FAIL` | invalid |
| `outdoor01/20260828T010500Z_corrective02` | execution wrapper reaped before supervisor initialization | `PROCESS_LIFECYCLE_FAIL` | invalid; no playback |
| `outdoor01/20260828T010500Z_corrective03` | sandbox denied local socket before ROS master | `ROS_MASTER_FAIL` | invalid; no playback |
| `outdoor01/20260828T011000Z_corrective04` | snapshot/parity/playback/output/cleanup pass; raw 20-column output was incorrectly passed directly to evo | `EVALUATOR_FAIL` | recovered evaluation is diagnostic, transaction remains non-canonical |

The last Outdoor attempt's independently recovered evo translation APE RMSE is
`0.240323 m`, but its immutable transaction terminal remains
`FAILED/EVALUATOR_FAIL`. It is pipeline-health evidence, not a canonical parent
result.

## Last scientifically valid results before takeover

- NTU FAST-LIVO2 eee_01: approximately `0.0303 m`, existing durable evidence.
- NTU FAST-LIVO2 nya_01: approximately `0.0398 m`, existing durable evidence.
- Oxford FAST-LIVO2 Quarter01: `0.0397 m`, existing snapshot/config/evaluator
  evidence.
- M3 Corridor01 canonical: `3.03 m`, snapshot-complete dataset-author adapted
  Avia path, `CANONICAL_VALID/GREEN`.
- M3 legacy `5.83 m`: `DIAGNOSTIC_VALID_NOT_CANONICAL`.

## Unfinished work

- build the authoritative local dataset inventory from disk/rosbag metadata;
- merge historical tables into the canonical benchmark ledger;
- make the transaction runner produce evaluator-ready trajectory and complete
  delivery/coverage gates inside the transaction;
- canonical Outdoor01 and Outdoor04;
- decide M3/M2 benchmark policy from both outdoor controls;
- Corridor02 if still useful; NTU sbs_01 parent gap audit/reproduction.

## WIP recovered

Repository tracked modifications: none.  
Repository untracked DS work: none beyond the newly supplied Prompt52.  
External `base_ws` and immutable results: preserved, not deleted or silently
promoted.

## Corrected previous final classification

`ROUND12_PARENT_REPRO_PARTIAL`.

Phase A recovery is complete only when this report and Prompt52 registration
are committed. No Phase B experiment was started during reconstruction.
