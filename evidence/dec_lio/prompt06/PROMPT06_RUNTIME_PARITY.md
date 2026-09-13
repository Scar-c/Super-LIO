# Prompt06 canonical runtime parity

Both runs used the native offline transport and canonical Alpha configs; no
canonical YAML was modified.

| scene | RC | fatal marker | schema5 rows | expected trajectory SHA256 | actual SHA256 | cmp vs Prompt05 |
|---|---:|---|---:|---|---|---:|
| Stairs Alpha | 0 | NONE | 3446 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 | 0 |
| Tunnel2 Alpha | 0 | NONE | 2739 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 | 0 |

The final evidence uses the non-destructive `canonical_rerun` runtime
directories because the runner refuses to overwrite the first canonical
directories.
