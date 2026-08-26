# Round11AA — NTU eee_01 / nya_01 (D-state-off, stride1)

## eee_01

- B0: 9af9b9d9b7fdeda4ffcd031b9f0cb544 (3981 rows) — historical anchor
  EXACTLY reproduced
- D0 (imu_fullscan, stride1, offset -0.0199575325817): cc92834dcb89b69b48fd4e762e25e942
  (3981 rows)
- camera 3986 frames, stride1 all accepted; ownership/NaN: clean

## nya_01

- B0: d547a22acc911a7c712cdd35c4a9598a (3943 rows) — legacy anchor
  EXACTLY reproduced
- D0: 644e78f75faaadc1aa56529ecbc5ef65 (3943 rows)
- ownership/NaN: clean

## Notes

- No committed NTU GT evaluator (leica /gndtr extraction tool absent from
  scripts/super_livo/evaluation) — D/B0 RMSE ratio not computable with a
  committed dataset-specific gate; per continuation rule (completion +
  accounting/ownership clean) pipeline continues.
- online/offline direct trajectory MD5 parity: NOT VERIFIED (current-code
  online-node run not performed; registry TB-era parity is historical).
