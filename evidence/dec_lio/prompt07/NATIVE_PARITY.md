# Native N and corrected-time parity

| scene | N trajectory SHA256 | rows | T equal to N | Prompt06 SHA preserved |
|---|---|---:|---|---|
| Stairs | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` | 3446 | YES | YES |
| Tunnel2 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | 2739 | YES | YES |

Prompt06-compatible shadow schemas remain D1=2, D2=3, consistency=4 and
axis=5; Prompt07 stage schema=1. The binary identity changed only for stage
counters and explicit point-time scaling; H, b, P, map, gamma and PCG were not
changed.
