# Dec-LIO tools

- `run_baseline.sh`: bounded online/offline native runner.
- `validate_input.py`: exact bag/config/ground-truth SHA256 gate.
- `production_identity.py`: source and configuration identity report.
- `check_evidence_hygiene.py`: rejects tracked runtime artifacts and
  Prob-LIO estimator names in the Dec-LIO production tree.
- `run_baseline.sh --d1-shadow`: enables read-only D1 CSV diagnostics; the
  switch is off unless explicitly requested.
