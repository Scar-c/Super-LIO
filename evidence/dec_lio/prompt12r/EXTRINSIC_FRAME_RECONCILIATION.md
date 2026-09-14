# NTNU extrinsic reconciliation

All transforms are written as destination-frame coordinates of the source.

```text
C1 card T_vn100<-os_sensor candidate:
    R=I, t=(0.01660, 0.02158, 0.03375) m

E1 raw acquisition T_vn100<-os_sensor:
    R=I, t=(0.00166, 0.02158, 0.03610) m

card - E1:
    dt=(0.01494, 0, -0.00235) m
    ||dt||=0.015123... m
    dR=I
```

If the card translation is expressed against `os_lidar`, composing the Ouster
internal transform gives:

```text
T_vn100<-os_lidar(card) = Rz(pi), t=(0.01660,0.02158,0.071945) m
T_vn100<-os_lidar(raw)  = Rz(pi), t=(0.00166,0.02158,0.074295) m
```

The residual discrepancy remains the same 14.94 mm x and 2.35 mm z
translation, with no rotational disagreement. It is not the Ouster internal
180-degree term or the 38.195 mm term.

Decision: retain the raw acquisition-specific transform for the two released
bags. ATE was not used to choose it.
