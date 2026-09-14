# Physical axis under exact alignment

`O_course = u_course^T P_weak u_course` is the corrected semantic label for
the former O_long field. It is a weak-subspace-to-physical-axis alignment, not
a harmfulness score.

| scene | arm | O_yaw median/P95 | O_course median/P95 | O_lateral median/P95 |
|---|---|---|---|---|
| stairs | N | 0.00118403/0.0508828 | 0.885704/0.999084 | 0.00652365/0.986483 |
| stairs | S | 0.0011326/0.540965 | 0.274646/0.984084 | 0.188967/0.987856 |
| stairs | A_old | 0.00108262/0.048247 | 0.895557/0.999363 | 0.00535743/0.989718 |
| stairs | A* | 0.000252982/0.0563357 | 0.884729/0.999734 | 0.00265063/0.992013 |
| tunnel2 | N | 0.000849664/0.00478922 | 0.992275/0.998709 | 0.00531689/0.0943527 |
| tunnel2 | S | 0.000421092/0.00381998 | 0.987553/0.998663 | 0.0102945/0.421482 |
| tunnel2 | A_old | 0.000594551/0.00484561 | 0.988422/0.998345 | 0.00894271/0.38451 |
| tunnel2 | A* | 0.000405181/0.00399166 | 0.988697/0.998903 | 0.00886893/0.388215 |

The exact A* arm preserves course-axis dominance in both scenes. The isolated
S Stairs arm does not: its course occupancy falls to about 0.275 and its weak
rank behavior changes. Thus exact stride order is a real weak-geometry seam,
even though the complete A* arm remains close to A_old.
