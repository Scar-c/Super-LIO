# Future full-pose tunnel dataset handoff

Prompt06 does not run NTNU or any new dataset.  The held-out validation
candidate handoff is:

1. NTNU Unified Autonomy Stack: `fyllingsdalen_tunnel`.
2. NTNU `runehamar_tunnel/hornbill`.

Required acquisition/reference contract for either candidate:

* AR-1 platform;
* Ouster OS0-128;
* VectorNav VN-100;
* ROS1 bag;
* full TUM pose reference with quaternion orientation;
* tunnel ground truth from Leica prism plus onboard-IMU offline optimization;
* raw Ouster packets converted to `sensor_msgs/PointCloud2` before
  Super-LIO integration.

The full TUM orientation reference is **not confirmed for the present GEODE
Tunnel2 input**; its supplied quaternion fields are invalid/zero and Prompt06
claims `GT attitude: MUST BE NO`.  It must be confirmed and checksum-recorded
for the future held-out dataset before any physical directional mechanism is
tested.
