# Ouster metadata authority

Each raw bag contains one `/ouster/metadata` `std_msgs/String` message.

Common decoded identity:

- product line: `OS-0-128`
- product number: `860-105000-07`
- serial: `122306000154`
- build revision/image: `v3.1.0` / `ousteros-image-prod-bootes-v3.1.0+20240426041747`
- columns per frame: 1024; pixels per column: 128; lidar mode: `1024x10`
- beam altitude/azimuth arrays: 128 entries each
- lidar UDP profile: `RNG19_RFL8_SIG16_NIR16_DUAL` (Fyllingsdalen), `RNG19_RFL8_SIG16_NIR16` (Runehamar)
- timestamp mode: `TIME_FROM_PTP_1588` (Fyllingsdalen), `TIME_FROM_SYNC_PULSE_IN` (Runehamar)

The raw metadata was inspected only; it was not written back into the source bags. Conversion was not attempted after the TF-C gate.
