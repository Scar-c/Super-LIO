#!/bin/bash
# GTP transaction supervisor for the canonical offline Super-LIVO runner.
set -u
RUN_ID="${1:?run id}"; OUT_ROOT="${2:?output root}"; RUN_DIR="$OUT_ROOT/$RUN_ID"
RUNNER="${SLV_RUNNER:-/home/lc/super_livo/src/Super-LIO/scripts/super_livo/experiments/run_offline_variant.sh}"
LOCK="${SLV_LOCK_FILE:-/home/lc/super_livo/base_ws/tools/benchmark_adapters/superlivo_adapter.lock}"
STATE="$RUN_DIR/state.json"; LOG="$RUN_DIR/supervisor.log"; CANCEL="$RUN_DIR/cancel"
mkdir -p "$RUN_DIR" "$(dirname "$LOCK")"
TOKEN="slv-${RUN_ID//\//_}-$$-$(date +%s%N)"; export SLV_TRANSACTION_TOKEN="$TOKEN"
SUP_TOKEN="$(awk '{print $22}' /proc/$$/stat)"
CHILD_PID=""; CHILD_PGID=""; CHILD_TOKEN=""; CLEANED=false; CLEANUP_OK=false
FINAL_STATE=FAILED; FINAL_CLASS=SUPERVISOR_FAILURE; FINAL_REASON="unexpected exit"; VALID=false

state() {
  python3 - "$STATE" "$RUN_ID" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" "$VALID" "$$" "$SUP_TOKEN" "$CHILD_PID" "$CHILD_PGID" "$CHILD_TOKEN" <<'PY'
import json,os,sys,tempfile
p,r,s,c,w,cl,v,sp,st,cp,cg,ct=sys.argv[1:]
n=lambda x:int(x) if x else None
d={'run_id':r,'state':s,'failure_class':c,'reason':w,'cleanup_verified':cl=='true',
   'experiment_valid':v=='true','supervisor_pid':int(sp),'supervisor_start_token':st,
   'transaction_token':os.environ['SLV_TRANSACTION_TOKEN'],'runner_pid':n(cp),
   'runner_pgid':n(cg),'runner_start_token':ct or None}
fd,t=tempfile.mkstemp(prefix='.state.',dir=os.path.dirname(p),text=True)
with os.fdopen(fd,'w') as f: json.dump(d,f,sort_keys=True);f.write('\n')
os.replace(t,p)
PY
  printf '[%s] state=%s class=%s reason=%s cleanup=%s\n' "$(date -u +%T)" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" >> "$LOG"
}
start_token(){ awk '{print $22}' "/proc/$1/stat" 2>/dev/null || true; }
pgid(){ ps -o pgid= -p "$1" 2>/dev/null | tr -d ' ' || true; }
group_pids(){ ps -eo pid=,pgid=,stat= | awk -v g="$1" '$2==g&&$3!~/^Z/{print $1}'; }
owned_group(){
  local p e; [ -n "$1" ] && [ -n "$(group_pids "$1")" ] || return 1
  for p in $(group_pids "$1"); do
    e="$(tr '\0' '\n' < "/proc/$p/environ" 2>/dev/null | sed -n 's/^SLV_TRANSACTION_TOKEN=//p')"
    [ "$e" = "$TOKEN" ] || return 1
  done
}
stop_child(){
  [ -n "$CHILD_PGID" ] || return 0
  if [ -r "/proc/$CHILD_PID/stat" ]; then
    [ "$(start_token "$CHILD_PID")" = "$CHILD_TOKEN" ] || return 0
    [ "$(pgid "$CHILD_PID")" = "$CHILD_PGID" ] || return 0
  fi
  owned_group "$CHILD_PGID" || return 0
  local sig
  for sig in TERM KILL; do
    kill -"$sig" -- -"$CHILD_PGID" 2>/dev/null || true
    for _ in $(seq 1 50); do [ -z "$(group_pids "$CHILD_PGID")" ] && return 0; sleep .1; done
  done
}
cleanup(){
  [ "$CLEANED" = false ] || return; CLEANED=true; trap - EXIT INT TERM HUP
  stop_child
  if [ -z "$CHILD_PGID" ] || [ -z "$(group_pids "$CHILD_PGID")" ]; then CLEANUP_OK=true
  else FINAL_REASON="$FINAL_REASON; owned runner group remains:$CHILD_PGID"; fi
  state "$FINAL_STATE" "$FINAL_CLASS" "$FINAL_REASON"
}
trap 'FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="supervisor signal received";exit 130' INT TERM HUP
trap cleanup EXIT

exec 9>"$LOCK"
if ! flock -n 9; then CLEANUP_OK=true;FINAL_CLASS=REFUSED_ACTIVE_TRANSACTION;FINAL_REASON="Super-LIVO lock held";state FAILED "$FINAL_CLASS" "$FINAL_REASON";trap - EXIT;exit 90;fi
state CREATED "" "exclusive lock acquired"

active="$(python3 - "$OUT_ROOT" "$RUN_DIR" <<'PY'
import json,pathlib,sys
root,cur=map(pathlib.Path,sys.argv[1:]); bad=[]
for p in root.rglob('state.json') if root.exists() else []:
 if p.parent == cur: continue
 try: d=json.loads(p.read_text())
 except Exception: bad.append(str(p)); continue
 if not (d.get('state') in {'SUCCESS','FAILED','CANCELLED'} and d.get('cleanup_verified') is True): bad.append(str(p))
print(';'.join(bad))
PY
)"
[ -z "$active" ] || { FINAL_CLASS=REFUSED_ACTIVE_TRANSACTION;FINAL_REASON="active/stale transaction: $active";exit 90; }
conflicting_bag="$(pgrep -af '[r]osbag play' || true)"
conflicting_estimator="$(pgrep -af '[s]uper_lio_offline_node' || true)"
[ -z "$conflicting_bag" ] || { FINAL_CLASS=REFUSED_SHARED_RESOURCE;FINAL_REASON="conflicting rosbag play";exit 90; }
[ -z "$conflicting_estimator" ] || { FINAL_CLASS=REFUSED_SHARED_RESOURCE;FINAL_REASON="conflicting estimator";exit 90; }
for f in "$RUNNER" "${SLV_CFG:?config}" "${SLV_BAG:?bag}"; do [ -f "$f" ] || { FINAL_CLASS=STATIC_PREFLIGHT_FAIL;FINAL_REASON="missing $f";exit 2; }; done
PROFILE="${SLV_SEMANTIC_PROFILE:?semantic profile selection required from start request}"
{
  echo "active Super-LIVO transaction: NONE"
  echo "conflicting rosbag play: NONE"
  echo "conflicting estimator: NONE"
  echo "shared-resource lock: ACQUIRED"
  echo "semantic profile: PENDING_CHILD_FAIL_CLOSED_VALIDATION"
  echo "producer gates: PENDING_CHILD_FAIL_CLOSED_READBACK"
} > "$RUN_DIR/preflight_evidence.txt"
state TRANSACTION_PREFLIGHT_PASS "" "active/conflicts none; lock acquired"

OUT_DIR="$RUN_DIR/out"
SLV_TRANSACTION_TOKEN="$TOKEN" SLV_TRANSACTION_GATE_DIR="$RUN_DIR" setsid "$RUNNER" \
  "$SLV_CFG" "$SLV_BAG" "$OUT_DIR" "${SLV_VARIANT:-d0}" \
  "${SLV_CAM_TOPIC:-}" "${SLV_CAM_CALIB:-}" "${SLV_DURATION:--1}" \
  "${SLV_S0_AUDIT:-1}" "profile_resolved" "${SLV_LAYER_AUDIT:-1}" \
  "${SLV_STRIDE:-1}" "${SLV_CAM_OFFSET:-0.0}" "${SLV_DATASET:-NTU}" \
  "${SLV_SEQUENCE:-eee_01}" "$PROFILE" \
  "${SLV_LEGACY_ALIAS:-NONE}" > "$RUN_DIR/runner.log" 2>&1 < /dev/null &
CHILD_PID=$!; sleep .05; CHILD_PGID="$(pgid "$CHILD_PID")"; CHILD_TOKEN="$(start_token "$CHILD_PID")"
state RUNNER_STARTED "" "waiting for semantic pre-playback gate"
ready=0
for _ in $(seq 1 300); do
  [ -f "$RUN_DIR/semantic_preplay_ready" ] && { ready=1; break; }
  kill -0 "$CHILD_PID" 2>/dev/null || break
  [ -f "$CANCEL" ] && { FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="cancel requested";exit 130; }
  sleep .1
done
[ "$ready" = 1 ] || { FINAL_CLASS=SEMANTIC_PROFILE_FAIL;FINAL_REASON="semantic pre-playback handshake missing";exit 1; }
MANIFEST="$OUT_DIR/resolved_experiment_semantics.yaml"; TRAJ="$OUT_DIR/trajectory.tum"
python3 /home/lc/super_livo/src/Super-LIO/scripts/super_livo/experiments/semantic_profiles.py validate --manifest "$MANIFEST" || { FINAL_CLASS=SEMANTIC_PROFILE_FAIL;FINAL_REASON="manifest validation failed";exit 1; }
sed -i 's/PENDING_CHILD_FAIL_CLOSED_VALIDATION/PASS/;s/PENDING_CHILD_FAIL_CLOSED_READBACK/PASS/' "$RUN_DIR/preflight_evidence.txt"
cat "$RUN_DIR/preflight_evidence.txt"
touch "$RUN_DIR/transaction_playback_authorized"
state PLAYBACK_STARTED "" "semantic profile and producer gates PASS"
while kill -0 "$CHILD_PID" 2>/dev/null; do [ -f "$CANCEL" ] && { FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="cancel requested";exit 130; }; sleep .2; done
wait "$CHILD_PID"; rc=$?
[ "$rc" = 0 ] || { FINAL_CLASS=PROCESS_LIFECYCLE_FAIL;FINAL_REASON="runner rc=$rc";exit 1; }
python3 /home/lc/super_livo/src/Super-LIO/scripts/super_livo/experiments/semantic_profiles.py validate --manifest "$MANIFEST" || { FINAL_CLASS=SEMANTIC_PROFILE_FAIL;FINAL_REASON="manifest validation failed";exit 1; }
[ -f "$TRAJ" ] || { FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory missing";exit 1; }
rows="$(wc -l < "$TRAJ")"; [ "$rows" -ge "${SLV_MIN_ROWS:-3000}" ] || { FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory rows=$rows";exit 1; }
python3 /home/lc/super_livo/src/Super-LIO/scripts/super_livo/experiments/validate_d_visual_shadow_result.py \
  --log "$OUT_DIR/node_stdout.log" --out "$RUN_DIR/d_visual_shadow_gate.yaml" || {
    FINAL_CLASS=SEMANTIC_RESULT_GATE_FAIL;FINAL_REASON="mandatory Visual-shadow counters missing or failed";exit 1;
  }
cp "$TRAJ" "$RUN_DIR/trajectory.tum"
VALID=true; FINAL_STATE=SUCCESS; FINAL_CLASS=""; FINAL_REASON="canonical semantic run; rows=$rows"
exit 0
