# Prompt05 hypotheses and classification

The hypotheses are tested descriptively against the matched Bridge, Stairs,
and Tunnel2 Alpha runs.  No threshold below is promoted to production.

| hypothesis | evidence | disposition |
|---|---|---|
| H0 geometry-only | Stairs has median rotational kappa 14.589 and p95 61.086 but APE RMSE 0.198 m; Tunnel2 has median rotational kappa 59.437 and RMSE 6.451 m. | Rejected as a transferable harmfulness rule. |
| H1 high kappa + low weak chi | Tunnel2 has low full-run median weak chi R 0.565 but its fixed onset reaches 66.355; Stairs remains low-error with high kappa and nonzero weak chi. | Not sufficient; onset candidate only. |
| H2 high kappa + high weak chi | Tunnel2 onset combines rotational kappa up to 110.444 with weak chi R 66.355 and psi R 33.432. | Supported as a Tunnel2 candidate signature, not causal proof. |
| H3 moderate kappa + high G/C_L | Bridge has moderate kappa and large C values, but median G is only 0.00642; its strong C is not a unique failure discriminator. | Not supported as a general explanation. |
| H4 sparse low lambda/N without strong G | Stairs and Tunnel2 have lower rotational lambda/N than Bridge, while Bridge has the Owner's sparse-environment label; G is small in all medians. | Does not explain the three scenes as one mechanism. |

## A-G classification audit

* **A — weak geometry + strong forcing:** not established as a complete
  scene-wide explanation; only the Tunnel2 onset is a candidate instance.
* **B — prior conflict better:** not selected.  `S_prior`, `delta_C`, and `G`
  are descriptive and do not isolate prior conflict from residual scale.
* **C — sparse information explains Bridge but not Tunnel2:** the original
  Prompt04 primary C was too strong and is superseded.  The Bridge owner label
  is sparse geometric support, while Tunnel2 is a distinct rotational
  candidate with no translation weak direction.
* **D — Tunnel2 supports a rotational causal chain:** candidate-level support
  only.  The available reference cannot measure attitude error, so this is not
  a causal classification.
* **E — different failure modes need separate detectors:** supported by the
  sparse Bridge label, low-error high-kappa Stairs control, and Tunnel2 onset
  separation.
* **F — consistency is still not transferable:** **primary classification**.
* **G — evidence contradicts a directional D2 gate:** supported for any gate
  that treats high kappa alone as harmful, because Stairs is a low-error
  counterexample; no production gate is introduced.

Final Prompt05 classification:

```text
F — INCONCLUSIVE / CONSISTENCY NOT TRANSFERABLE
supporting annotation: E supported; D candidate-only; G rejects kappa-only gate
```
