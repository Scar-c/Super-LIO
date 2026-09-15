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

The final table below is populated only from the clean Prompt16 reruns. A
native mismatch is a hard stop for scientific A/B interpretation.
