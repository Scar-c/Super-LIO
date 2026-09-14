# Prompt12 closure

Prompt12 reached the mandatory TF authority gate after successful startup, dataset identity, raw bag presence, metadata, and static-TF extraction audits.

Hard stop:

```text
NTNU_EXTRINSIC_AUTHORITY_CONFLICT
```

Reason: both NTNU bags expose an unambiguous `vn100 -> os_sensor -> os_lidar` chain whose composed `T_vn100<-os_lidar` is materially inconsistent with the official Prompt12 `T_imu<-lidar` authority in both rotation and translation.

Therefore the following were intentionally not executed: Ouster packet conversion, PointCloud2 point-time authority closure, Super Ouster/VN100 adapter changes, canonical runtime configuration, native sanity, full 6DoF N/P1 runs, determinism checks, and all held-out metrics. No GT-based configuration tuning occurred.

The Prompt12 archive is present at `prompts/dec_lio/PROMPT12_NTNU_HELDOUT_FULL6DOF.md`. Source bags and GT files were not modified, moved, deleted, or migrated.
