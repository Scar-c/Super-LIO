# Native trajectory parity

The Prompt18 and Prompt20 Tunnel2 Alpha Native files were compared directly.

```text
Prompt18 rows:  2739
Prompt20 rows:  2739
Prompt18 bytes: 449428
Prompt20 bytes: 449428
Prompt18 SHA256: 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
Prompt20 SHA256: 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
cmp --silent:  PASS
timestamp/pose rows: byte-identical
```

Conclusion: the `6.450672` versus `6.30079297` discrepancy is not caused by
trajectory drift, Native source drift, bag drift, config drift, or runtime
preprocessing drift.
