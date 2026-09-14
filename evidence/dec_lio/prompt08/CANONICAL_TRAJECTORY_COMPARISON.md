# Canonical trajectory comparison

All rows below use association max difference 0.10 s, one global SE(3), no
scale, no crop, and no per-window realignment. Stairs uses the official full-
pose reference. Tunnel2 is position-only and its GT attitude claim is
`MUST BE NO`.

| scene | arm | matched | RMSE m | median m | P95 m | max m | trajectory SHA |
|---|---|---:|---:|---:|---:|---:|---|
| stairs | N | 3442 | 0.1977243472 | 0.183732441 | 0.3044888497 | 0.3757390501 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 |
| stairs | S_correct | 3442 | 0.5289707552 | 0.4189464002 | 0.8701305616 | 0.9399272056 | 9a4abb5cf192945ab7f0e453cfa7ca4356b88a25ad74cc90f3f0e1f3eaac8f59 |
| stairs | A_old | 3445 | 0.2514963558 | 0.2168598133 | 0.4235576324 | 0.5690649103 | bcf216d30c40e6820f30f601f3939fec67896db11bbb22beea35989e56b84969 |
| stairs | A_correct | 3445 | 0.2945521385 | 0.2320233414 | 0.608020613 | 0.7002786392 | 5b882021145e70118abd308b63bc66aadb15d6a76ca46f22442d8595d528ee2b |
| tunnel2 | N | 591 | 6.450671655 | 2.783266468 | 17.20008746 | 17.40506453 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 |
| tunnel2 | S_correct | 591 | 6.634343285 | 2.912983833 | 17.56170522 | 17.76255564 | b0211447445db066b8bea064a3f1eaaf8ecabe888ec430518dff9bc52965a312 |
| tunnel2 | A_old | 591 | 2.776658138 | 1.096040551 | 7.709932239 | 7.965588957 | 2aee0260c58daf95539ba3c3d05c409b8e24e41707cd2f56e32bb9822b583024 |
| tunnel2 | A_correct | 591 | 2.712877573 | 1.072252379 | 7.530378083 | 7.78830089 | c73e9a42416d803ed53947caf81eedc0b28395afd0ec2b33279e992c84e9cead |

Historical A_old is valid and reused from Prompt07R. Old Prompt07R S and A*
trajectory results are not in this table; they are `INVALID_PRE_FIX` and are
not scientific controls for Prompt08.
