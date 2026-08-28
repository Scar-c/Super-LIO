#!/bin/bash
# FAST-LIVO2 M3DGR transaction supervisor v3 (canonical implementation).
set -u
RUN_ID="${1:?run_id}"; OUT_ROOT="${2:?out root}"
BAG="${M3_BAG:?bag}"; LAUNCH="${M3_LAUNCH:?launch}"; CFG="${M3_CFG:?config}"; CAMCFG="${M3_CAMCFG:?camera config}"
LAUNCH_PATH="${M3_LAUNCH_PATH:-$(dirname "$CFG")/../launch/$LAUNCH}"
TRAJ="${M3_TRAJ_PATH:?trajectory path}"; MIN_ROWS="${M3_MIN_ROWS:-1}"; EVAL_CMD="${M3_EVAL:-:}"
TRAJ_FORMAT="${M3_TRAJ_FORMAT:-tum}"; TRAJ_EPOCH="${M3_TRAJ_EPOCH:-}"
RUN_MODE="${M3_RUN_MODE:-canonical}"
TRAJ_CONVERTER="${M3_TRAJ_CONVERTER:-/home/lc/super_livo/src/Super-LIO/scripts/super_livo/evaluation/fast_livo2_debug_to_tum.py}"
PARITY_TOOL="${M3_PARITY_TOOL:-/home/lc/super_livo/src/Super-LIO/scripts/super_livo/evaluation/m3_official_runtime_parity.py}"
LOCK_FILE="${M3_LOCK_FILE:-/home/lc/super_livo/base_ws/tools/benchmark_adapters/m3_adapter.lock}"; LOCK_META="${M3_LOCK_META:-${LOCK_FILE}.owner.json}"
RUN_DIR="$OUT_ROOT/$RUN_ID"; STATE="$RUN_DIR/state.json"; LOG="$RUN_DIR/supervisor.log"; CANCEL_FILE="$RUN_DIR/cancel"
mkdir -p "$RUN_DIR" "$(dirname "$LOCK_FILE")"
TRANSACTION_TOKEN="m3-${RUN_ID//\//_}-$$-$(date +%s%N)"; export M3_TRANSACTION_TOKEN="$TRANSACTION_TOKEN"
SUPERVISOR_START_TOKEN="$(awk '{print $22}' /proc/$$/stat)"
MASTER_PID=""; MASTER_PGID=""; MASTER_TOKEN=""; LAUNCH_PID=""; LAUNCH_PGID=""; LAUNCH_TOKEN=""; BAG_PID=""; BAG_PGID=""; BAG_TOKEN=""
FINAL_STATE=FAILED; FINAL_CLASS=SUPERVISOR_FAILURE; FINAL_REASON="unexpected exit"; CLEANUP_OK=false; EXPERIMENT_VALID=false; LOCK_ACQUIRED=false; CLEANUP_RAN=false

write_state() {
 python3 - "$STATE" "$RUN_ID" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" "$EXPERIMENT_VALID" "$$" "$SUPERVISOR_START_TOKEN" "$MASTER_PID" "$MASTER_PGID" "$MASTER_TOKEN" "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN" "$BAG_PID" "$BAG_PGID" "$BAG_TOKEN" <<'PY'
import json,os,sys,tempfile
p,r,s,c,w,cl,v,sp,st,mp,mg,mt,lp,lg,lt,bp,bg,bt=sys.argv[1:]
n=lambda x:int(x) if x else None
d={"run_id":r,"state":s,"failure_class":c,"reason":w,"cleanup_verified":cl=="true","experiment_valid":v=="true","supervisor_pid":int(sp),"supervisor_start_token":st,"transaction_token":os.environ["M3_TRANSACTION_TOKEN"],"master_pid":n(mp),"master_pgid":n(mg),"master_start_token":mt or None,"launch_pid":n(lp),"launch_pgid":n(lg),"launch_start_token":lt or None,"bag_pid":n(bp),"bag_pgid":n(bg),"bag_start_token":bt or None}
fd,t=tempfile.mkstemp(prefix=".state.",dir=os.path.dirname(p),text=True)
with os.fdopen(fd,"w") as f:json.dump(d,f,sort_keys=True);f.write("\n")
os.replace(t,p)
PY
 printf '[%s] %s %s %s cleanup=%s\n' "$(date -u +%T)" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" >> "$LOG"
}
process_start_token(){ awk '{print $22}' "/proc/$1/stat" 2>/dev/null||true; }
process_pgid(){ ps -o pgid= -p "$1" 2>/dev/null|tr -d ' '||true; }
group_pids(){ ps -eo pid=,pgid=,stat=|awk -v g="$1" '$2==g&&$3!~/^Z/{print $1}'; }
group_owned(){ local p e; [ -n "$(group_pids "$1")" ]||return 1; for p in $(group_pids "$1");do e="$(tr '\0' '\n'</proc/$p/environ 2>/dev/null|sed -n 's/^M3_TRANSACTION_TOKEN=//p')";[ "$e" = "$TRANSACTION_TOKEN" ]||return 1;done; }
stop_group(){ local p="$1" g="$2" t="$3" s;[ -n "$g" ]||return 0;if [ -r /proc/$p/stat ];then [ "$(process_start_token "$p")" = "$t" ]||return 0;[ "$(process_pgid "$p")" = "$g" ]||return 0;fi;group_owned "$g"||return 0;for s in INT TERM KILL;do kill -$s -- -$g 2>/dev/null||true;for _ in $(seq 1 20);do [ -z "$(group_pids "$g")" ]&&return 0;sleep .1;done;done; }
cleanup(){
 [ "$CLEANUP_RAN" = false ]||return;CLEANUP_RAN=true;trap - EXIT INT TERM HUP
 stop_group "$BAG_PID" "$BAG_PGID" "$BAG_TOKEN";stop_group "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN";stop_group "$MASTER_PID" "$MASTER_PGID" "$MASTER_TOKEN"
 local left="" g;for g in "$BAG_PGID" "$LAUNCH_PGID" "$MASTER_PGID";do [ -n "$g" ]&&[ -n "$(group_pids "$g")" ]&&left="$left $g";done;[ -z "$left" ]&&CLEANUP_OK=true||FINAL_REASON="$FINAL_REASON; groups remain:$left"
 if [ "$LOCK_ACQUIRED" = true ];then python3 - "$LOCK_META" "$RUN_ID" "$SUPERVISOR_START_TOKEN" <<'PY'
import json,os,sys
p,r,t=sys.argv[1:]
try:d=json.load(open(p))
except (OSError,ValueError):d={}
if d.get('run_id')==r and d.get('supervisor_start_token')==t:
 try:os.unlink(p)
 except FileNotFoundError:pass
PY
 fi
 write_state "$FINAL_STATE" "$FINAL_CLASS" "$FINAL_REASON"
}
trap 'FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="supervisor signal received";exit 130' INT TERM HUP
trap cleanup EXIT

exec 9>"$LOCK_FILE"
if ! flock -n 9;then FINAL_CLASS=REFUSED_ACTIVE_TRANSACTION;FINAL_REASON="shared trajectory-path lock held";CLEANUP_OK=true;write_state FAILED "$FINAL_CLASS" "$FINAL_REASON";trap - EXIT;exit 90;fi
LOCK_ACQUIRED=true
python3 - "$LOCK_META" "$RUN_ID" "$$" "$SUPERVISOR_START_TOKEN" "$TRAJ" "$BAG" <<'PY'
import json,sys,time
p,r,pid,t,traj,bag=sys.argv[1:];json.dump({'run_id':r,'supervisor_pid':int(pid),'supervisor_start_token':t,'start_time':time.time(),'shared_resources':[traj,'FAST_LIVO2_M3DGR_ADAPTER',bag]},open(p,'w'),sort_keys=True)
PY
if ! python3 - "$OUT_ROOT" "$RUN_DIR" <<'PY'
import json,pathlib,sys
root,cur=map(pathlib.Path,sys.argv[1:]);bad=[]
for p in root.rglob('state.json') if root.exists() else ():
 if p.parent==cur:continue
 try:d=json.loads(p.read_text())
 except (OSError,ValueError):continue
 if d.get('state') not in {'SUCCESS','FAILED','CANCELLED'} or not d.get('cleanup_verified',False):bad.append(str(p))
if bad:print('active/stale transaction: '+','.join(bad));raise SystemExit(1)
PY
then FINAL_CLASS=REFUSED_ACTIVE_TRANSACTION;FINAL_REASON="nonterminal or cleanup-unverified transaction exists";exit 91;fi
if [ "${M3_TEST_MODE:-0}" != 1 ];then
 conflicts="$(python3 - "$$" <<'PY'
import os, pathlib, sys
me=int(sys.argv[1])
for p in pathlib.Path('/proc').iterdir():
 if not p.name.isdigit() or int(p.name)==me:continue
 try:a=(p/'cmdline').read_bytes().rstrip(b'\0').split(b'\0')
 except (OSError,PermissionError):continue
 s=[x.decode(errors='replace') for x in a];exe=os.path.basename(s[0]) if s else ''
 conflict=(exe=='rosbag' and 'play' in s and any('M3DGR' in x for x in s)) or (exe=='roslaunch' and 'fast_livo' in s) or exe=='fastlivo_mapping'
 if conflict:print(p.name+' '+' '.join(s))
PY
)"
 if [ -n "$conflicts" ];then printf '%s\n' "$conflicts">"$RUN_DIR/conflicting_processes.txt";FINAL_CLASS=REFUSED_CONFLICTING_PROCESS;FINAL_REASON="conflicting process";exit 92;fi
fi
{
 echo "active FAST-LIVO2 transaction: NONE"
 echo "conflicting rosbag play: NONE"
 echo "conflicting roslaunch/mapping: NONE"
 echo "shared trajectory-path lock: ACQUIRED"
} | tee "$RUN_DIR/preflight_exclusivity.txt"
write_state CREATED "" "exclusive lock acquired; preflight clear"
for f in "$BAG" "$CFG" "$CAMCFG" "$LAUNCH_PATH";do [ -f "$f" ]||{ FINAL_CLASS=STATIC_PREFLIGHT_FAIL;FINAL_REASON="missing $f";exit 1;};done
if [ "$TRAJ_FORMAT" = fast_livo2_debug ];then
 [ -f "$TRAJ_CONVERTER" ]||{ FINAL_CLASS=STATIC_PREFLIGHT_FAIL;FINAL_REASON="missing trajectory converter $TRAJ_CONVERTER";exit 1;}
 [ -n "$TRAJ_EPOCH" ]||{ FINAL_CLASS=STATIC_PREFLIGHT_FAIL;FINAL_REASON="M3_TRAJ_EPOCH required for fast_livo2_debug";exit 1;}
fi
sha256sum "$CFG" "$CAMCFG" "$LAUNCH_PATH">"$RUN_DIR/config_hashes.txt"
sha256sum "$RUN_DIR/config_hashes.txt">"$RUN_DIR/config_hashes.sha256";write_state STATIC_PREFLIGHT_PASS "" "files present and hashed"
# Never validate a trajectory left by an earlier transaction. Preserve it as
# contaminated provenance inside this immutable run directory, then require
# the estimator to create a new shared-path file.
if [ -e "$TRAJ" ];then mv "$TRAJ" "$RUN_DIR/preexisting_shared_trajectory.INVALID";fi
if [ "${M3_TEST_MODE:-0}" = 1 ];then PORT="${M3_TEST_PORT:-11399}";elif ! PORT="$(python3 -c 'import socket;s=socket.socket();s.bind(("127.0.0.1",0));print(s.getsockname()[1])')";then FINAL_CLASS=INFRA_FAIL;FINAL_REASON="cannot allocate ROS master port";exit 1;fi;export ROS_MASTER_URI="http://127.0.0.1:$PORT";if [ "${M3_TEST_MODE:-0}" != 1 ];then source /opt/ros/noetic/setup.bash;source /home/lc/super_livo/base_ws/ws_m3o/devel/setup.bash;fi;export LD_LIBRARY_PATH="/home/lc/super_livo/base_ws/third_party/sophus_build:${LD_LIBRARY_PATH:-}"
setsid roscore -p "$PORT">"$RUN_DIR/roscore.log" 2>&1 </dev/null & MASTER_PID=$!;sleep .05;MASTER_PGID="$(process_pgid "$MASTER_PID")";MASTER_TOKEN="$(process_start_token "$MASTER_PID")"
ready=0;for _ in $(seq 1 40);do timeout 2 rosparam list>/dev/null 2>&1&&{ ready=1;break;};sleep .5;done;[ "$ready" = 1 ]||{ FINAL_CLASS=INFRA_FAIL;FINAL_REASON="master not ready";exit 1;};write_state ISOLATED_MASTER_READY "" "master ready"
setsid roslaunch fast_livo "$LAUNCH" rviz:=false>"$RUN_DIR/node.log" 2>&1 </dev/null & LAUNCH_PID=$!;sleep .05;LAUNCH_PGID="$(process_pgid "$LAUNCH_PID")";LAUNCH_TOKEN="$(process_start_token "$LAUNCH_PID")"
ready=0;for _ in $(seq 1 40);do if timeout 2 rosnode list 2>/dev/null|grep -q laserMapping;then ready=1;break;fi;kill -0 "$LAUNCH_PID" 2>/dev/null||break;sleep .5;done;[ "$ready" = 1 ]||{ FINAL_CLASS=PROCESS_LIFECYCLE_FAIL;FINAL_REASON="estimator node not ready";exit 1;};write_state NODES_READY "" "estimator node ready"
rosparam dump "$RUN_DIR/effective_rosparams.after_launch.yaml" / 2>/dev/null||{ FINAL_CLASS=CONFIG_FAIL;FINAL_REASON="snapshot fail";exit 1;};write_state EFFECTIVE_CONFIG_CAPTURED "" "snapshot saved"
python3 "$PARITY_TOOL" --official "$CFG" "$CAMCFG" --snapshot "$RUN_DIR/effective_rosparams.after_launch.yaml" --out "$RUN_DIR/official_runtime_config_parity.json">"$RUN_DIR/parity.log" 2>&1||{ FINAL_CLASS=CONFIG_PARITY_FAIL;FINAL_REASON="parity FAIL";exit 1;};write_state CONFIG_PARITY_PASS "" "parity PASS"
if ! timeout 2 rosnode list 2>/dev/null|grep -qE 'republish|image_transport';then FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="required image data path absent";exit 1;fi
ls -la "$BAG">"$RUN_DIR/bag_info.txt"||{ FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="bag unreadable";exit 1;};write_state DATA_PATH_READY "" "bag and republisher present"
setsid rosbag play --rate 1.0 ${M3_DURATION:+--duration "$M3_DURATION"} "$BAG">"$RUN_DIR/play.log" 2>&1 </dev/null & BAG_PID=$!;sleep .05;BAG_PGID="$(process_pgid "$BAG_PID")";BAG_TOKEN="$(process_start_token "$BAG_PID")";write_state PLAYBACK_STARTED "" "rosbag play"
while kill -0 "$BAG_PID" 2>/dev/null;do [ -f "$CANCEL_FILE" ]&&{ FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="cancel requested";exit 130;};sleep .2;done;wait "$BAG_PID";rc=$?;[ "$rc" = 0 ]||{ FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="play rc=$rc";exit 1;};write_state PLAYBACK_FINISHED "" "bag done"
stop_group "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN";write_state ESTIMATOR_DRAINED "" "estimator drained"
[ -f "$TRAJ" ]||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory missing";exit 1;};ROWS="$(wc -l<"$TRAJ")";[ "$ROWS" -ge "$MIN_ROWS" ]||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="too few rows";exit 1;}
python3 - "$TRAJ" <<'PY'||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory invalid";exit 1;}
import numpy as np,sys
d=np.atleast_2d(np.loadtxt(sys.argv[1]));raise SystemExit(0 if np.all(np.isfinite(d)) and np.all(np.diff(d[:,0])>=0) else 1)
PY
EVAL_TRAJ="$RUN_DIR/trajectory.tum"
if [ "$TRAJ_FORMAT" = fast_livo2_debug ];then
 python3 "$TRAJ_CONVERTER" --input "$TRAJ" --output "$EVAL_TRAJ" --epoch "$TRAJ_EPOCH">"$RUN_DIR/trajectory_conversion.txt" 2>&1||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory conversion failed";exit 1;}
else cp "$TRAJ" "$EVAL_TRAJ";fi
python3 - "$EVAL_TRAJ" <<'PY'||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="evaluator trajectory invalid";exit 1;}
import numpy as np,sys
d=np.atleast_2d(np.loadtxt(sys.argv[1]));raise SystemExit(0 if d.shape[1]==8 and np.all(np.isfinite(d)) and np.all(np.diff(d[:,0])>=0) else 1)
PY
sha256sum "$TRAJ" "$EVAL_TRAJ">"$RUN_DIR/trajectory_hashes.txt"
write_state OUTPUT_VALIDATED "" "raw and evaluator trajectories valid";export EVAL_TRAJ RUN_DIR
[ "$EVAL_CMD" = : ]||bash -c "$EVAL_CMD">"$RUN_DIR/evaluation.txt" 2>&1||{ FINAL_CLASS=EVALUATOR_FAIL;FINAL_REASON="evaluator failed";exit 1;};write_state EVALUATED "" "evaluated"
if [ "$RUN_MODE" = smoke ];then write_state SMOKE_VALID "" "infrastructure smoke result";FINAL_STATE=SUCCESS;FINAL_CLASS="";FINAL_REASON="smoke only; not an accuracy experiment"
else EXPERIMENT_VALID=true;write_state CANONICAL_VALID "" "canonical result";FINAL_STATE=SUCCESS;FINAL_CLASS="";FINAL_REASON="canonical result";fi
