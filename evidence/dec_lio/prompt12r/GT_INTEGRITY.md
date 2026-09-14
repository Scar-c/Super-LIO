# GT integrity

```text
Fyllingsdalen:
 rows=4384
 first=1719890346.8310049 last=1719890584.1917658 duration=237.3607609272 s
 dt quantiles=[0.03499913,0.03999996,0.04999995,0.08500147,0.10000051,1.02000165]
 nonmonotonic=0, finite position/quaternion=YES, zero quaternion=0,
 quaternion norm bad (>1e-3)=0

Runehamar:
 rows=4371
 first=1763487661.516097 last=1763487934.0089297 duration=272.4928326607 s
 dt quantiles=[0.03499889,0.03999949,0.04999948,0.08499932,0.10154997,33.14470196]
 nonmonotonic=0, finite position/quaternion=YES, zero quaternion=0,
 quaternion norm bad (>1e-3)=0
```

Both TUM references pass the full-orientation validity gate. The long Runehamar
inter-reference gap is retained and not interpolated or cropped by the
evaluation contract.
