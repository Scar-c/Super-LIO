# BRANCH_DECISION_R12_UPSTREAM_IMAGE_TRANSPORT_PARITY

```
decision source:    branch review after previous Origin instruction
decision identifier: BRANCH_DECISION_R12_UPSTREAM_IMAGE_TRANSPORT_PARITY
status:             APPLIED_PENDING_ORIGIN_REVIEW
```

## 1. Previous proposed solution

The earlier Round12 resume report stopped F3 (Oxford) because the Oxford bag
camera messages are `sensor_msgs/CompressedImage` while the FAST-LIVO2 image
callback consumes raw `sensor_msgs/Image`. A generic custom compressed->raw
adapter was suggested.

## 2. What this branch changed/refined

A generic custom adapter is NOT the default. Instead the priority is:

1. Reproduce the exact Oxford benchmark image-delivery path (search benchmark
   wrappers/launch/scripts/remaps for the raw-topic producer).
2. If none found after a bounded audit: use ROS standard
   `image_transport republish` (compressed -> raw) — the same mechanism
   FAST-LIVO2 method-author uses.
3. A custom JPEG decoder is only a last resort; STOP before implementing one.

## 3. Primary-source evidence (independently re-verified this round)

- Oxford fork `ori-drs/FAST-LIVO2` branch config-used-OSD,
  `launch/mapping_oxford_spires.launch:16` contains a COMMENTED ROS standard
  image_transport node:
  `image_transport republish compressed in:=/left_camera/image raw out:=/left_camera/image`
  => proves the adapted tree RETAINED the standard compressed->raw mechanism,
  but does NOT by itself prove the benchmark actively used this exact line.
- FAST-LIVO2 method-author `hku-mars/FAST-LIVO2`
  `launch/mapping_avia_marslvig.launch:16` ACTIVELY uses
  `image_transport republish compressed ... raw ...`
  => an upstream method-author precedent:
  CompressedImage -> ROS image_transport decoder -> sensor_msgs/Image ->
  FAST-LIVO2 img_cbk. NOT a new photometric algorithm invented by Super-LIVO.

## 4. Runtime behavior adopted

- Standard ROS Noetic `image_transport republish` (compressed -> raw) decodes
  the bag CompressedImage to sensor_msgs/Image feeding FAST-LIVO2 img_cbk.
- Conversion is ONLY encoded payload -> deterministic decoder -> Image.
  Forbidden: resize/crop/rectify/undistort/gamma/exposure/brightness/denoise/
  sharpen/histogram/resampling/frame ops/timestamp or frame_id modification/
  manual time offset.
- Decoded output is described scientifically as: "decoded representation of
  the compressed image stored in the bag" (NOT the original sensor raw image).
- header.stamp / frame_id / sequence order preserved unchanged; no
  ros::Time::now() / wall clock / republish-time timestamp for estimator timing.

## 5. Estimator / config semantic changes

- estimator source modification: NONE
- algorithm parameters modified: NONE
- config: pristine upstream oxford_spires.yaml used unchanged
- only a non-semantic ROS interface adapter (image_transport republish) added
  via an external wrapper launch.

## 6. F3/F4 results under this decision

(filled after runs)
