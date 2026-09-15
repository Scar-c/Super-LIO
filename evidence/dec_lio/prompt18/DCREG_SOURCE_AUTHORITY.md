# DCReg source authority

Authority clone: `https://github.com/JokerJohn/DCReg`

Pinned commit: `8ce8451b15491a4bbe17cf85ab02a8bed6696861`

The implementation was audited from `/tmp/dcreg_prompt18/DCReg/include/dcreg.hpp`
at that exact commit. The Prompt18 port follows the public core semantics but
keeps Super-LIO's existing correspondence construction, mixed tangent basis,
outer ICP, backtracking, and pose update.

## Source mapping

| Authority module | Pinned source | Dec-LIO mapping |
|---|---|---|
| full SVD and two Schur complements | `dcreg.hpp:198-248` | `DetectDegeneracy` in `DCRegCoreSolver` |
| eigenbasis-to-axis matching, weak masks, selective eigenvalue clamp, block preconditioner | `dcreg.hpp:251-351` | `CharacterizeDegeneracy` in `DCRegCoreSolver` |
| preconditioned conjugate-gradient solve of original `H delta=b` | `dcreg.hpp:363-471` | `SolvePreconditionedUpdate` in `DCRegCoreSolver` |
| dense raw QR solve and fallback | `dcreg.hpp:354-360`, `:458-470` | explicit `DCREG_*_QR` statuses |

The official defaults frozen for Prompt18 are:

```text
degeneracy_condition_threshold = 10.0
kappa_target                   = 10.0
pcg_tolerance                  = 1e-6
pcg_max_iterations             = 10
```

DCReg is used as a numerical solve strategy. The original Super normal
equation and objective remain unchanged; no P1/SA spectral attenuation or
information-matrix mutation is imported.
