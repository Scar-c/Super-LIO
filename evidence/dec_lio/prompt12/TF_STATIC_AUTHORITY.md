# TF static authority — Prompt12 hard stop

## Observed static graph

The relevant raw `/tf_static` edges are:

| sequence | parent | child | count | translation (m) | quaternion (x,y,z,w) |
|---|---|---|---:|---|---|
| both | `vn100` | `os_sensor` | 1 | `(0.00166, 0.02158, 0.03610)` | `(0,0,0,1)` |
| both | `os_sensor` | `os_lidar` | 1 (2 records in Runehamar) | `(0,0,0.038195)` | `(0,0,1,0)` |
| both | `os_sensor` | `os_imu` | 1 | `(-0.002441,-0.009725,0.007533)` | `(0,0,0,1)` |

The frame IDs identify `vn100` as the VN100 frame and `os_lidar` as the Ouster LiDAR frame. The graph is therefore a complete cross-sensor chain, not TF-B.

## Composition convention

For a TF record with parent `A` and child `B`, use the ROS transform as `T_A<-B`, matching Prompt12's authority convention `p_A = T_AB p_B`. The composed chain is:

```text
T_vn100<-os_lidar
  = T_vn100<-os_sensor T_os_sensor<-os_lidar
  = (R=diag(-1,-1,1), t=[0.00166, 0.02158, 0.074295] m)
```

## Official external authority

Prompt12 requires:

```text
T_imu<-lidar: R = I
t = [0.0166, 0.02158, 0.03375] m
```

The bag-implied chain differs in rotation and has translation delta `[-0.01494, 0, 0.040545] m`, norm `0.0432099598 m`. This is a material conflict, not numeric noise or an unexposed cross-sensor link.

Classification: **TF-C — CONFLICT**.

Prompt12-required action:

```text
STOP — NTNU_EXTRINSIC_AUTHORITY_CONFLICT
```

No converter build, converted bag, Super adapter, canonical config, N/P1 run, ATE/RPE evaluation, or threshold/tuning decision was performed after this gate. The official value was not averaged with the bag chain and no value was selected by ATE.
