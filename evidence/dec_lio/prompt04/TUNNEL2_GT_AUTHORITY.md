# Tunnel2 Alpha GT/reference authority

`Tunneling_tunnel2.txt` is frozen as the supplied GEODE Metro_Tunnels
Tunneling reference for the Alpha device:

```text
path: /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt
sha256: b83ca5db4102838cd260fc78b0a20cca90da9db855ab9b762f9d7f2e5c856c5a
rows: 618
first timestamp: 1706584536.660000309
last timestamp: 1706584746.680000005
```

The GEODE dataset README states that Metro_Tunnels includes shield-tunnel and
tunneling-tunnel sequences, that Leica MS60 prism tracking supplies the ground
truth poses, and that Alpha-device trajectories require no additional
coordinate conversion for `rmse.py`. This matches the local Alpha bag and the
provided trajectory reference. Authority source:

<https://github.com/PengYu-Team/GEODE_dataset#localization-evaluation>

Classification:

```text
OFFICIAL_GEODE_DISTRIBUTED_PREPROCESSED_TRAJECTORY_REFERENCE
```

The reference is used for the fixed internal evaluator and GEODE
official-style APE. No FAST-LIO2 score or recollection is used as a gate.
