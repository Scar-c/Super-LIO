# Prompt20 identity

- Dec-LIO repository: `/home/lc/dec_lio/src/Super-LIO`
- branch: `Dec-LIO`
- exact ROS1 ancestry: `merge-base(HEAD, origin/ros1) == origin/ros1 == 60b57aaac8dc397f80c56364e7ccb008c300cc29`
- L0 checkpoint: `e77934a`
- L1 checkpoint: `6b0abd9`
- source boundary: `production_identity.py` passed
- ROS1 build: `catkin_make -C /home/lc/dec_lio -j4` passed
- Prompt20 synthetic and existing regression tests: passed
- runtime CPU policy: canonical runs requested 4 threads
- A0 note: Prompt18 canonical A0 values are retained as the previously
  audited pure-asymmetric authority; three parallel replays in this prompt
  did not produce nonempty trajectory files and are excluded from the CSV.

