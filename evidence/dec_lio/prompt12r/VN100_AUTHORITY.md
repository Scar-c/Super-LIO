# VN100 authority

The estimator uses only `/vectornav_driver_node/imu/data`.

```text
Fyllingsdalen: frame=vn100, count=56191, effective=199.9954 Hz,
 first=1719890310.9108202, last=1719890591.861788,
 dt median=0.00499892 s, P5=0.00499797, P95=0.00500107,
 duplicates=0, backwards=0, gaps>20ms=0, NaN/Inf=0

Runehamar: frame=vn100, count=54540, effective=200.00496 Hz,
 first=1763487661.3960989, last=1763487934.0889292,
 dt median=0.00499892 s, P5=0.00499773, P95=0.00500107,
 duplicates=0, backwards=0, gaps>20ms=0, NaN/Inf=0
```

Orientation fields are zero and all orientation/angular/linear covariance
arrays are zero in both released VN100 topics. Super consumes only angular
velocity and linear acceleration.
