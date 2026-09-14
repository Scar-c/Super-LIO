# Weak versus complement update

The estimator preserves the raw DCReg weak direction source and computes the
counterfactual delta in the full fused state. On finite, bounded diagnostic
rows, final shadow median `(weak, complement)` update-difference norms were:

| scene | weak median / P95 | complement median / P95 |
|---|---:|---:|
| Bridge01 | 0 / 535.08 | 0 / 984.37 |
| Stairs | 0 / 4.4515e9 | 0 / 5.8074e9 |
| Tunnel2 | 1.0895e7 / 3.8956e9 | 1.7154e7 / 4.8283e9 |

These values are diagnostic exposure only: rows above the fixed
counterfactual trust bound are fail-open and set to zero, and no diagnostic
quantity is recomputed on H-tilde. The update delta is therefore not used as
a causal claim or as a tuning signal.
