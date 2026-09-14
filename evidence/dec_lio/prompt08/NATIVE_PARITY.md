# Native compatibility-OFF parity

The `geode_finite_then_stride=false` path was preserved. Canonical native
parity uses the required trajectory SHA and `cmp = 0` against the Prompt07R
native reference.

| scene | expected SHA | actual SHA | stage SHA | cmp |
|---|---|---|---|---|
| stairs | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 | 2f49bc3b78d0f83c4ace0cd8efe3435977f18eb543303aa5b650dd0d761ca83f | PASS |
| tunnel2 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 | 461e32c464ba2de5cf198ebbf2825d1809e771a7f675ca5540a6e503bf83cf95 | PASS |
