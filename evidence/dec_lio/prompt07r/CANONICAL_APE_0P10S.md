# Canonical trajectory evaluation

All retained trajectories use association `max_diff=0.10 s`, one global SE(3)
alignment, no scale, no crop and no per-window realignment. Stairs uses the
official full-pose reference for position APE; Tunnel2 is position-only.

| scene | arm | matched | RMSE | mean | median | P90 | P95 | max | trajectory SHA |
|---|---|---|---|---|---|---|---|---|---|
| stairs | N | 3442 | 0.197724 | 0.186934 | 0.183732 | 0.274092 | 0.304489 | 0.375739 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 |
| stairs | V | 3442 | 0.195687 | 0.184045 | 0.180299 | 0.279801 | 0.309865 | 0.390046 | 06d9cb222f1756000575a9389cce7b4dcd4e77a43c86702193b98f7bf4070228 |
| stairs | B | 3445 | 0.217605 | 0.204667 | 0.201607 | 0.30546 | 0.338446 | 0.394435 | fe49cbf2d6201acf8ca9f63944d8642720831742a2fbf3db647329b6bfbf0460 |
| stairs | R | 3442 | 0.197724 | 0.186934 | 0.183732 | 0.274092 | 0.304489 | 0.375739 | 26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11 |
| stairs | A_old | 3445 | 0.251496 | 0.228367 | 0.21686 | 0.377955 | 0.423558 | 0.569065 | bcf216d30c40e6820f30f601f3939fec67896db11bbb22beea35989e56b84969 |
| stairs | S | 3446 | 35813.9 | 30014.9 | 25534 | 61848.8 | 80413.6 | 101597 | e036f3875224d7ea6c1c28ec6454f0e546671c982f904845c101734e23695e98 |
| stairs | A* | 3446 | 0.299703 | 0.255655 | 0.228495 | 0.465326 | 0.623737 | 0.792524 | efa1b641ae73a9e697cd629b478c900367d293ce770e5d587775b24dffe4a778 |
| tunnel2 | N | 591 | 6.45067 | 4.50284 | 2.78327 | 16.9847 | 17.2001 | 17.4051 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 |
| tunnel2 | V | 591 | 2.86619 | 1.88999 | 1.14326 | 7.65499 | 7.95198 | 8.21126 | 0fe508cfd8b6e7afda57846324ac1614cef25f7d7a43f6c2ad6fe6b32e7fc209 |
| tunnel2 | B | 591 | 5.61127 | 3.81382 | 2.3275 | 14.9824 | 15.2275 | 15.4299 | 242a3d9a266f5a96b4e1c2f6e7c572eb4087e5604c5b55cdee05b2aca6438380 |
| tunnel2 | R | 591 | 6.45067 | 4.50284 | 2.78327 | 16.9847 | 17.2001 | 17.4051 | 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30 |
| tunnel2 | A_old | 591 | 2.77666 | 1.82639 | 1.09604 | 7.42442 | 7.70993 | 7.96559 | 2aee0260c58daf95539ba3c3d05c409b8e24e41707cd2f56e32bb9822b583024 |
| tunnel2 | S | 591 | 4.94114 | 3.32175 | 2.02801 | 13.2735 | 13.5101 | 13.7136 | 52795e48e36a1e05eb210d9e355fff9148c0e601f380135b520cc8f9dc174141 |
| tunnel2 | A* | 591 | 2.69626 | 1.77311 | 1.06373 | 7.20388 | 7.48889 | 7.74026 | 9aef2272c88e2d664b3b1e204d1d09b2efcf53431b1636e41ba1f829b938aed0 |

Prompt07's 0.05 s metrics are historical and superseded for canonical
comparison. No old trajectory was rerun for this evaluator correction.
