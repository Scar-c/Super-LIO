# Raw-index set overlap

Native selection is `{0,3,6,...} ∩ finite`; GEODE selection is
`finite_original_indices[0,3,6,...]`. Equal counts are not treated as set
equality.

| scene | before blind | post blind=2.0 | post blind=1.5 |
|---|---|---|---|
| Tunnel2 | median=0.215061, P5=0.0767173, P95=0.450266, min=0.0316554, exact=0, >=0.99=0, >=0.95=0 | median=0.206436, P5=0.0760059, P95=0.432138, min=0.0331029, exact=0, >=0.99=0, >=0.95=0 | median=0.214711, P5=0.0744986, P95=0.451209, min=0.0316554, exact=0, >=0.99=0, >=0.95=0 |
| Stairs | median=0.201203, P5=0.0810077, P95=0.425112, min=0.0226173, exact=0, >=0.99=0, >=0.95=0 | median=0.197456, P5=0.0840306, P95=0.418901, min=0.0302491, exact=0, >=0.99=0, >=0.95=0 | median=0.20096, P5=0.0857147, P95=0.426823, min=0.0202752, exact=0, >=0.99=0, >=0.95=0 |

Every frame in both datasets has `exact_set_equal=false`; count equality is
MUST BE NO as an authority criterion. The full per-frame intersection, union,
symmetric difference and Jaccard records are in the runtime JSON.
