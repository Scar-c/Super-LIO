# Prompt05 full-shadow runtime and parity

All runs were offline whole-bag, `requested_threads=32`, with D1, D2, and
consistency shadows enabled.  The runtime metadata recorded a clean worktree,
RC=0, and no fatal marker for every scene.

| scene | bag SHA256 | config SHA256 | consistency rows | trajectory SHA256 |
|---|---|---|---:|---|
| Bridge | `1fb14937289172c1fa694a817430c142568b205a1535e16a2d0406f7c471b7b6` | `1f039b0c70b4d7a2a63d420587b8e05ff4653f0385d2aa95db1eb7ccbfaa3558` | 3814 | `6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203` |
| Stairs | `71cea6a30573ac6144776d873a6232707c04ee977a6a59e63bfdd2bce42fedb1` | `12d22a80abd21050d6e7b4984edd51f09f2e65d46bcaadb853a4c75a04a97d3b` | 3446 | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` |
| Tunnel2 | `08a4a32f660b3d2df3d2adb053fe86d310505a1689e9d37b3f87a6a20768498e` | `1e081c1e414e251a1284dbef9688bc74955d86a7c029687f779ba5b9f0540c04` | 2739 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` |

Every consistency row has schema version 4 and `valid=1`; each trajectory SHA
matches the Prompt05 canonical expectation.

Direct byte comparisons against the existing Prompt04/native baseline are
also zero-difference:

```text
Bridge   Prompt05 trajectory == Prompt04 bridge_shadow_zeta canonical_retry: PASS
Stairs   Prompt05 trajectory == Prompt04 stairs_shadow_zeta canonical_retry: PASS
Tunnel2  Prompt05 trajectory == Prompt04 tunnel2_native_strict native_01: PASS
```

Runtime evidence paths:

```text
/home/lc/dec_lio/runtime/prompt05/bridge/canonical/
/home/lc/dec_lio/runtime/prompt05/stairs/canonical/
/home/lc/dec_lio/runtime/prompt05/tunnel2/canonical/
```
