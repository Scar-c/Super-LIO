#!/bin/bash
# Generic NTU transaction supervisor; parent semantics are supplied by separate adapters.
set -u
RUN_ID="${1:?run_id}"; OUT_ROOT="${2:?out root}"; RUN_DIR="$OUT_ROOT/$RUN_ID"
BAG="${NTU_BAG:?bag}"; CFG="${NTU_CONFIG:?config}"; LAUNCH_PATH="${NTU_LAUNCH_PATH:?launch path}"
PKG="${NTU_LAUNCH_PACKAGE:?launch package}"; LAUNCH="${NTU_LAUNCH_FILE:?launch file}"
NODE="${NTU_REQUIRED_NODE:?required node}"; OUTPUT="${NTU_OUTPUT_PATH:?output path}"
MIN_ROWS="${NTU_MIN_ROWS:-2}"; LOCK="${NTU_LOCK_FILE:-/home/lc/super_livo/base_ws/tools/benchmark_adapters/ntu_adapter.lock}"
STATE="$RUN_DIR/state.json"; LOG="$RUN_DIR/supervisor.log"; CANCEL="$RUN_DIR/cancel"
mkdir -p "$RUN_DIR" "$(dirname "$LOCK")"
TOKEN="ntu-${RUN_ID//\//_}-$$-$(date +%s%N)"; export NTU_TRANSACTION_TOKEN="$TOKEN"
SUP_TOKEN="$(awk '{print $22}' /proc/$$/stat)"
MASTER_PID=""; MASTER_PGID=""; MASTER_TOKEN=""; LAUNCH_PID=""; LAUNCH_PGID=""; LAUNCH_TOKEN=""; BAG_PID=""; BAG_PGID=""; BAG_TOKEN=""
FINAL_STATE=FAILED; FINAL_CLASS=SUPERVISOR_FAILURE; FINAL_REASON="unexpected exit"; CLEANUP_OK=false; VALID=false; CLEANED=false

state() {
 python3 - "$STATE" "$RUN_ID" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" "$VALID" "$$" "$SUP_TOKEN" "$MASTER_PID" "$MASTER_PGID" "$MASTER_TOKEN" "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN" "$BAG_PID" "$BAG_PGID" "$BAG_TOKEN" <<'PY'
import json,os,sys,tempfile
p,r,s,c,w,cl,v,sp,st,mp,mg,mt,lp,lg,lt,bp,bg,bt=sys.argv[1:]
n=lambda x:int(x) if x else None
d={'run_id':r,'state':s,'failure_class':c,'reason':w,'cleanup_verified':cl=='true','experiment_valid':v=='true','supervisor_pid':int(sp),'supervisor_start_token':st,'transaction_token':os.environ['NTU_TRANSACTION_TOKEN'],'master_pid':n(mp),'master_pgid':n(mg),'master_start_token':mt or None,'launch_pid':n(lp),'launch_pgid':n(lg),'launch_start_token':lt or None,'bag_pid':n(bp),'bag_pgid':n(bg),'bag_start_token':bt or None}
fd,t=tempfile.mkstemp(prefix='.state.',dir=os.path.dirname(p),text=True)
with os.fdopen(fd,'w') as f: json.dump(d,f,sort_keys=True);f.write('\n')
os.replace(t,p)
PY
 printf '[%s] state=%s class=%s reason=%s cleanup=%s\n' "$(date -u +%T)" "$1" "${2:-}" "${3:-}" "$CLEANUP_OK" >> "$LOG"
}
start_token(){ awk '{print $22}' "/proc/$1/stat" 2>/dev/null||true; }
pgid(){ ps -o pgid= -p "$1" 2>/dev/null|tr -d ' '||true; }
group_pids(){ ps -eo pid=,pgid=,stat=|awk -v g="$1" '$2==g&&$3!~/^Z/{print $1}'; }
owned(){ local p e;[ -n "$(group_pids "$1")" ]||return 1;for p in $(group_pids "$1");do e="$(tr '\0' '\n'</proc/$p/environ 2>/dev/null|sed -n 's/^NTU_TRANSACTION_TOKEN=//p')";[ "$e" = "$TOKEN" ]||return 1;done; }
stop_group(){ local p="$1" g="$2" t="$3" sig;[ -n "$g" ]||return 0;if [ -r "/proc/$p/stat" ];then [ "$(start_token "$p")" = "$t" ]||return 0;[ "$(pgid "$p")" = "$g" ]||return 0;fi;owned "$g"||return 0;for sig in INT TERM KILL;do kill -$sig -- -$g 2>/dev/null||true;for _ in $(seq 1 20);do [ -z "$(group_pids "$g")" ]&&return 0;sleep .1;done;done; }
cleanup(){
 [ "$CLEANED" = false ]||return;CLEANED=true;trap - EXIT INT TERM HUP
 stop_group "$BAG_PID" "$BAG_PGID" "$BAG_TOKEN";stop_group "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN";stop_group "$MASTER_PID" "$MASTER_PGID" "$MASTER_TOKEN"
 local left="" g;for g in "$BAG_PGID" "$LAUNCH_PGID" "$MASTER_PGID";do [ -n "$g" ]&&[ -n "$(group_pids "$g")" ]&&left="$left $g";done
 [ -z "$left" ]&&CLEANUP_OK=true||FINAL_REASON="$FINAL_REASON; groups remain:$left";state "$FINAL_STATE" "$FINAL_CLASS" "$FINAL_REASON"
}
trap 'FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="supervisor signal received";exit 130' INT TERM HUP
trap cleanup EXIT

exec 9>"$LOCK";if ! flock -n 9;then CLEANUP_OK=true;state FAILED REFUSED_ACTIVE_TRANSACTION "NTU adapter lock held";trap - EXIT;exit 90;fi
state CREATED "" "exclusive lock acquired"
for f in "$BAG" "$CFG" "$LAUNCH_PATH";do [ -f "$f" ]||{ FINAL_CLASS=STATIC_PREFLIGHT_FAIL;FINAL_REASON="missing $f";exit 1;};done
sha256sum "$BAG" "$CFG" "$LAUNCH_PATH">"$RUN_DIR/input_hashes.txt";state STATIC_PREFLIGHT_PASS "" "inputs hashed"
[ ! -e "$OUTPUT" ]||mv "$OUTPUT" "$RUN_DIR/preexisting_output.INVALID"
if [ "${NTU_TEST_MODE:-0}" = 1 ];then PORT="${NTU_TEST_PORT:-11419}";else PORT="$(python3 -c 'import socket;s=socket.socket();s.bind(("127.0.0.1",0));print(s.getsockname()[1])')";source /opt/ros/noetic/setup.bash;source "${NTU_WS_SETUP:?workspace setup}";fi
export ROS_MASTER_URI="http://127.0.0.1:$PORT" ROS_HOME="$RUN_DIR/ros_home";mkdir -p "$ROS_HOME"
setsid roscore -p "$PORT">"$RUN_DIR/roscore.log" 2>&1 </dev/null & MASTER_PID=$!;sleep .05;MASTER_PGID="$(pgid "$MASTER_PID")";MASTER_TOKEN="$(start_token "$MASTER_PID")"
ready=0;for _ in $(seq 1 40);do timeout 2 rosparam list>/dev/null 2>&1&&{ ready=1;break;};sleep .25;done;[ "$ready" = 1 ]||{ FINAL_CLASS=INFRA_FAIL;FINAL_REASON="isolated master not ready";exit 1;};state ISOLATED_MASTER_READY "" "master ready"
if [ "$PKG" = __file__ ];then setsid roslaunch "$LAUNCH_PATH" ${NTU_LAUNCH_ARGS:-}>"$RUN_DIR/node.log" 2>&1 </dev/null &
else setsid roslaunch "$PKG" "$LAUNCH" ${NTU_LAUNCH_ARGS:-}>"$RUN_DIR/node.log" 2>&1 </dev/null & fi
LAUNCH_PID=$!;sleep .05;LAUNCH_PGID="$(pgid "$LAUNCH_PID")";LAUNCH_TOKEN="$(start_token "$LAUNCH_PID")"
ready=0;for _ in $(seq 1 80);do timeout 2 rosnode list 2>/dev/null|grep -q "$NODE"&&{ ready=1;break;};kill -0 "$LAUNCH_PID" 2>/dev/null||break;sleep .25;done;[ "$ready" = 1 ]||{ FINAL_CLASS=PROCESS_LIFECYCLE_FAIL;FINAL_REASON="parent node not ready";exit 1;};state NODES_READY "" "parent node ready"
if [ "${NTU_TEST_MODE:-0}" != 1 ];then
 for topic in ${NTU_REQUIRED_TOPICS:-};do
  ready=0
  for _ in $(seq 1 40);do timeout 2 rostopic info "$topic" 2>/dev/null|grep -q "$NODE"&&{ ready=1;break;};sleep .25;done
  [ "$ready" = 1 ]||{ FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="no parent subscription on $topic";exit 1;}
 done
fi
if [ -n "${NTU_PRE_PLAY_CMD:-}" ];then bash -c "$NTU_PRE_PLAY_CMD" >"$RUN_DIR/pre_play.log" 2>&1||{ FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="pre-play adapter failed";exit 1;};fi
rosparam dump "$RUN_DIR/effective_rosparams.after_launch.yaml" / 2>/dev/null||{ FINAL_CLASS=CONFIG_FAIL;FINAL_REASON="snapshot failed";exit 1;};sha256sum "$RUN_DIR/effective_rosparams.after_launch.yaml">"$RUN_DIR/snapshot_sha256.txt";touch "$RUN_DIR/snapshot_before_playback";state EFFECTIVE_CONFIG_CAPTURED "" "snapshot before playback"
if [ -n "${NTU_PARITY_CMD:-}" ];then RUN_DIR="$RUN_DIR" bash -c "$NTU_PARITY_CMD" >"$RUN_DIR/parity.log" 2>&1||{ FINAL_CLASS=CONFIG_PARITY_FAIL;FINAL_REASON="config parity failed";exit 1;};fi;state CONFIG_PARITY_PASS "" "parent parity pass"
setsid rosbag play --rate "${NTU_PLAY_RATE:-1.0}" ${NTU_DURATION:+--duration "$NTU_DURATION"} "$BAG">"$RUN_DIR/play.log" 2>&1 </dev/null & BAG_PID=$!;sleep .05;BAG_PGID="$(pgid "$BAG_PID")";BAG_TOKEN="$(start_token "$BAG_PID")";state PLAYBACK_STARTED "" "bag playback"
while kill -0 "$BAG_PID" 2>/dev/null;do [ -f "$CANCEL" ]&&{ FINAL_STATE=CANCELLED;FINAL_CLASS=USER_CANCELLED;FINAL_REASON="cancel requested";exit 130;};sleep .2;done;wait "$BAG_PID";rc=$?;[ "$rc" = 0 ]||{ FINAL_CLASS=DATA_DELIVERY_FAIL;FINAL_REASON="play rc=$rc";exit 1;};state PLAYBACK_FINISHED "" "bag complete"
stop_group "$LAUNCH_PID" "$LAUNCH_PGID" "$LAUNCH_TOKEN";state ESTIMATOR_DRAINED "" "parent drained"
if [ -n "${NTU_POST_DRAIN_CMD:-}" ];then RUN_DIR="$RUN_DIR" bash -c "$NTU_POST_DRAIN_CMD" >"$RUN_DIR/post_drain.log" 2>&1||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="post-drain adapter failed";exit 1;};fi
[ -f "$OUTPUT" ]||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory missing";exit 1;};rows="$(wc -l<"$OUTPUT")";[ "$rows" -ge "$MIN_ROWS" ]||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory rows=$rows";exit 1;}
python3 - "$OUTPUT" <<'PY'||{ FINAL_CLASS=OUTPUT_FAIL;FINAL_REASON="trajectory invalid";exit 1;}
import numpy as np,sys
d=np.atleast_2d(np.loadtxt(sys.argv[1]));raise SystemExit(0 if d.shape[1]==8 and np.all(np.isfinite(d)) and np.all(np.diff(d[:,0])>=0) else 1)
PY
cp "$OUTPUT" "$RUN_DIR/trajectory.tum";state OUTPUT_VALIDATED "" "trajectory valid"
if [ "${NTU_RUN_MODE:-canonical}" = smoke ];then
 FINAL_STATE=SUCCESS;FINAL_CLASS="";FINAL_REASON="smoke only; accuracy not consumed";state SMOKE_VALID "" "$FINAL_REASON"
else
 if [ -n "${NTU_EVAL_CMD:-}" ];then EVAL_TRAJ="$RUN_DIR/trajectory.tum" RUN_DIR="$RUN_DIR" bash -c "$NTU_EVAL_CMD">"$RUN_DIR/evaluation.txt" 2>&1||{ FINAL_CLASS=EVALUATOR_FAIL;FINAL_REASON="evaluator failed";exit 1;};fi
 VALID=true;FINAL_STATE=SUCCESS;FINAL_CLASS="";FINAL_REASON="canonical result";state CANONICAL_VALID "" "canonical result"
fi
