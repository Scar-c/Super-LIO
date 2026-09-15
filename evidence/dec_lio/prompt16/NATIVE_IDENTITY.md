# Native identity gate

The native gate is run with `estimator_mode=native` from sequence start. It
requires clean worktree execution, the same bag/config/GT contracts used by
Prompt14/15, and records trajectory rows, bytes and SHA256 for each sequence.

Expected historical primary anchors retained from Prompt15 are:

```text
stairs_alpha: 557579 bytes,
  sha256 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
tunnel1_gamma: 341978 bytes,
  sha256 ab93cbaa4a1e823d2ab1c983a5bef478345cf4588eb47a2f4ac632e906378a6c
```

Final clean Prompt16 reruns:

| Sequence | Rows | Bytes | SHA256 | Fatal marker |
|---|---:|---:|---|---|
| bridge01 | 3814 | 622301 | `6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203` | NONE |
| stairs_alpha | 3446 | 557579 | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` | NONE |
| tunnel1_gamma | 2078 | 341978 | `ab93cbaa4a1e823d2ab1c983a5bef478345cf4588eb47a2f4ac632e906378a6c` | NONE |
| tunnel2_alpha | 2739 | 449428 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | NONE |
| tunnel2_gamma | 2605 | 426058 | `6eb07eed27c4e22b19ec83bbac63e8d797a02032e46b8cff4235051a46a388f6` | NONE |

The row counts above are TUM rows (`wc -l`); runner meta records the same
outputs' byte/hash values. A native mismatch is a hard stop for
scientific A/B interpretation. Stairs and tunnel1 exactly match the Prompt15
historical authority bytes/SHA.
