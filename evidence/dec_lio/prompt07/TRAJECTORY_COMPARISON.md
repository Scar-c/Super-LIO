# Trajectory comparison

Contract: association max-diff .05 s, one global SE(3) alignment, no scale, no crop, no per-window realignment. Tunnel2 is position-only; Stairs uses the official full-pose reference.

| scene | arm | matched | RMSE m | median m | P95 m | max m | trajectory SHA256 |
|---|---|---:|---:|---:|---:|---:|---|
| Stairs | N | 3439 | 0.19775 | 0.183736 | 0.304373 | 0.375656 | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` |
| Stairs | V | 3439 | 0.195704 | 0.180376 | 0.309773 | 0.389892 | `06d9cb222f1756000575a9389cce7b4dcd4e77a43c86702193b98f7bf4070228` |
| Stairs | B | 3445 | 0.217605 | 0.201607 | 0.338446 | 0.394435 | `fe49cbf2d6201acf8ca9f63944d8642720831742a2fbf3db647329b6bfbf0460` |
| Stairs | R | 3439 | 0.19775 | 0.183736 | 0.304373 | 0.375656 | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` |
| Stairs | A1 | 3445 | 0.251496 | 0.21686 | 0.423558 | 0.569065 | `bcf216d30c40e6820f30f601f3939fec67896db11bbb22beea35989e56b84969` |
| Stairs | A2 | 3445 | 0.251496 | 0.21686 | 0.423558 | 0.569065 | `bcf216d30c40e6820f30f601f3939fec67896db11bbb22beea35989e56b84969` |
| Tunnel2 | N | 580 | 6.30079 | 2.6157 | 17.3687 | 17.5747 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` |
| Tunnel2 | V | 580 | 2.81447 | 1.08818 | 8.01247 | 8.27405 | `0fe508cfd8b6e7afda57846324ac1614cef25f7d7a43f6c2ad6fe6b32e7fc209` |
| Tunnel2 | B | 580 | 5.48427 | 2.18709 | 15.3704 | 15.5762 | `242a3d9a266f5a96b4e1c2f6e7c572eb4087e5604c5b55cdee05b2aca6438380` |
| Tunnel2 | R | 580 | 6.30079 | 2.6157 | 17.3687 | 17.5747 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` |
| Tunnel2 | A1 | 580 | 2.72695 | 1.0434 | 7.76514 | 8.02517 | `2aee0260c58daf95539ba3c3d05c409b8e24e41707cd2f56e32bb9822b583024` |
| Tunnel2 | A2 | 580 | 2.72695 | 1.0434 | 7.76514 | 8.02517 | `2aee0260c58daf95539ba3c3d05c409b8e24e41707cd2f56e32bb9822b583024` |
