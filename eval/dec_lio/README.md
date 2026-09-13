# Dec-LIO evaluation

`exact_parity.py` is the mandatory raw-byte parity gate. `pose_bag_to_tum.py`
converts online `/lio/odom` messages using the same formatting as the offline
capture. `eval_tum_translation.py` reports rigid SE(3)-aligned translation APE
against the supplied GEODE reference; it does not crop or scale.
