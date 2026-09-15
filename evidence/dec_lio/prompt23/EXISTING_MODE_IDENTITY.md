# Existing mode identity

Current-source replays after the shared `ObserveNative()` dispatch refactor:

- Native Tunnel2 Alpha SHA256: `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` — PASS
- L1 Tunnel2 Alpha SHA256: `7d583427442ef4428584179410ba4104a13022ee95278a4fee17a7d8c3db9d88` — PASS
- frozen historical Stairs L1 SHA256: `818dcb63aa78ca6e5494d7e8fc3f7c17656b559715a0d98df3f9d3993c6aa25a`
- frozen historical Gamma L1 SHA256: `6f8c255151ae611534e8d8657b9f4eb9f12dc48173ff950d3e2c411bc2d7f0fb`

No frozen Native/L1 estimator implementation was imported into the final gate.
