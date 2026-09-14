# NTNU deskew audit

The converter supplies a scan header timestamp and physical per-point offsets
from 0 to approximately 100 ms. Super's existing Ouster path passes the
physical seconds to the native undistortion stage. The terminal point uses
the last decoded native offset; no new interpolation or fallback was added.

```text
physical scan span: approximately 99.9 ms
interpolation: existing native IMU interpolation
terminal fallback: existing native behavior
native behavior modified: NO
```
