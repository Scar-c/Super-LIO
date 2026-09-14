# Tunnel2 P1

Evaluation: supplied `Tunneling_tunnel2.txt`, fixed 0.10 s association, one
global SE(3) translation alignment. The supplied quaternion fields are zero,
so this report makes no attitude claim.

| metric | N | P1 |
|---|---:|---:|
| translation RMSE m | 6.450672 | 4.691793 |
| translation median m | 2.783266 | 1.903054 |
| translation P95 m | 17.200087 | 12.862756 |

The predeclared onset window `1706584541.828..1706584579.030` has absolute
translation-error median 16.884 m (N) versus 12.503 m (P1); post-onset
medians are 2.762 m versus 1.895 m. Five-second local onset median is
1.759 m versus 0.976 m, and post-onset median is 0.250 m versus 0.250 m.

P1 exposure: 8851/8851 = 1.000000 active; active gamma median 0.398241;
trace-ratio median 0.996191. Improvement is 27.267%, passing the >=10%
benefit gate. GT attitude claim: NO.
