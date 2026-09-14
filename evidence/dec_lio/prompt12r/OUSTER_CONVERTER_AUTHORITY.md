# Ouster converter authority

Converter source was isolated under
`/home/lc/dec_lio/runtime/prompt12r/converter_ws`:

```text
repository: https://github.com/ntnu-arl/ouster-ros.git
branch: feature/rosbag_packet_unpacking
commit: 6d6923b65ab4d70916a76362f7563b6d1b66179a
ouster-sdk submodule: d99676858b3b6f90e1618354ff7930cabafa9769
build: catkin_make -C converter_ws -j4, RC=0
command: rosrun ouster_ros bag_converter sensors_only.bag sensors_only_with_clouds.bag ouster
```

This is the converter instructed by the NTNU release documentation, which
explicitly names the packet-unpacking branch and `bag_converter` workflow.
Source: https://huggingface.co/datasets/ntnu-arl/unified_autonomy_stack_datasets/blob/main/lidar_packets_to_pointclouds.md
