#!/usr/bin/env python3
"""Fast end-to-end lifecycle tests using fake ROS public CLIs."""
import json, os, pathlib, signal, subprocess, tempfile, time

ROOT = pathlib.Path(__file__).resolve().parents[3]
RUNNER = ROOT / "scripts/super_livo/experiments/run_m3_transaction.sh"
WATCHER = ROOT / "scripts/super_livo/experiments/wait_for_m3_run.py"

def make_exe(path, body):
    path.write_text("#!/bin/bash\nset -u\n" + body)
    path.chmod(0o755)

def state(d): return json.loads((d / "state.json").read_text())

def wait_state(d, wanted, timeout=5):
    end=time.time()+timeout
    while time.time()<end:
        try:
            if state(d).get("state")==wanted:return
        except (FileNotFoundError, ValueError):pass
        time.sleep(.03)
    raise AssertionError(f"did not reach {wanted}: {state(d) if (d/'state.json').exists() else None}")

def token_pids(token):
    found=[]
    for p in pathlib.Path('/proc').iterdir():
        if not p.name.isdigit():continue
        try: env=(p/'environ').read_bytes().split(b'\0')
        except (OSError, PermissionError):continue
        if f"M3_TRANSACTION_TOKEN={token}".encode() in env:found.append(int(p.name))
    return found

with tempfile.TemporaryDirectory(prefix="m3-transaction-e2e-") as td:
    root=pathlib.Path(td); fake=root/'bin'; fake.mkdir(); out=root/'runs'; out.mkdir()
    bag=root/'Outdoor01.bag'; cfg=root/'m3dgr_avia.yaml'; cam=root/'camera.yaml'; launch=root/'mapping.launch'; traj=root/'mat_out.txt'; parity=root/'parity.py'
    for p in (bag,cfg,cam,launch):p.write_text('fixture\n')
    make_exe(fake/'roscore', 'exec sleep 60\n')
    make_exe(fake/'rosparam', 'if [ "$1" = dump ]; then echo "runtime: true" > "$2"; fi\nexit 0\n')
    make_exe(fake/'rosnode', '[ "${M3_FAKE_ALGO_FAIL:-0}" = 1 ] && exit 1\n[ "${M3_FAKE_DATA_PATH_FAIL:-0}" = 1 ] && { printf "/laserMapping\\n"; exit 0; }\nprintf "/laserMapping\\n/image_transport_republish\\n"\n')
    make_exe(fake/'roslaunch', '[ "${M3_FAKE_ALGO_FAIL:-0}" = 1 ] && exit 7\nexec sleep 60\n')
    make_exe(fake/'rosbag', 'sleep "${M3_FAKE_PLAY_SEC:-0.3}"\n[ "${M3_FAKE_BAG_FAIL:-0}" = 1 ] && exit 8\nif [ "${M3_FAKE_EMPTY_TRAJ:-0}" = 1 ]; then : > "$M3_TRAJ_PATH"; else printf "1 0 0 0 0 0 0 1\\n2 0 0 0 0 0 0 1\\n" > "$M3_TRAJ_PATH"; fi\n')
    parity.write_text('import json,os,sys\np=sys.argv[sys.argv.index("--out")+1]\njson.dump({"parity":"PASS"},open(p,"w"))\nraise SystemExit(1 if os.environ.get("M3_FAKE_PARITY_FAIL")=="1" else 0)\n')
    base=os.environ.copy();base.update(PATH=f"{fake}:{base['PATH']}",M3_TEST_MODE='1',M3_BAG=str(bag),M3_LAUNCH='mapping.launch',M3_LAUNCH_PATH=str(launch),M3_CFG=str(cfg),M3_CAMCFG=str(cam),M3_TRAJ_PATH=str(traj),M3_MIN_ROWS='2',M3_PARITY_TOOL=str(parity),M3_LOCK_FILE=str(root/'adapter.lock'),M3_LOCK_META=str(root/'owner.json'))

    def start(name, extra=None):
        env=base.copy();env.update(extra or {});d=out/name;d.mkdir(parents=True,exist_ok=False)
        return subprocess.Popen([str(RUNNER),name,str(out)],env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True),d

    # TX-T1 missing config fails before ROS/bag.
    missing=root/'missing.yaml';p,d=start('missing_config',{'M3_CFG':str(missing)})
    assert p.wait(timeout=3)!=0; s=state(d)
    assert s['failure_class']=='STATIC_PREFLIGHT_FAIL' and not (d/'play.log').exists()

    # TX-T2 parity mismatch blocks playback.
    p,d=start('parity_fail',{'M3_FAKE_PARITY_FAIL':'1'});assert p.wait(timeout=8)!=0;s=state(d)
    assert s['failure_class']=='CONFIG_PARITY_FAIL' and not (d/'play.log').exists(), s

    # TX-T6 missing republisher/data path blocks playback.
    p,d=start('data_path_fail',{'M3_FAKE_DATA_PATH_FAIL':'1'});assert p.wait(timeout=8)!=0;s=state(d)
    assert s['failure_class']=='DATA_DELIVERY_FAIL' and not (d/'play.log').exists()

    # TX-T7 empty output is OUTPUT_FAIL, never canonical.
    p,d=start('empty_output',{'M3_FAKE_EMPTY_TRAJ':'1'});assert p.wait(timeout=8)!=0;s=state(d)
    assert s['failure_class']=='OUTPUT_FAIL' and s['experiment_valid'] is False

    # normal SUCCESS -> cleanup; terminal state survives cleanup
    p,d=start('success'); assert p.wait(timeout=8)==0; s=state(d)
    assert s['state']=='SUCCESS' and s['cleanup_verified'] is True and s['experiment_valid'] is True
    assert token_pids(s['transaction_token'])==[]

    # TX-T9 a second sequential transaction also succeeds without leftovers.
    p,d=start('success_sequential'); assert p.wait(timeout=8)==0; s=state(d)
    assert s['state']=='SUCCESS' and s['cleanup_verified'] is True
    assert token_pids(s['transaction_token'])==[]

    # algorithm FAIL -> cleanup
    p,d=start('algorithm_fail',{'M3_FAKE_ALGO_FAIL':'1'}); assert p.wait(timeout=8)!=0; s=state(d)
    assert s['state']=='FAILED' and s['failure_class']=='PROCESS_LIFECYCLE_FAIL' and s['cleanup_verified'] is True

    # watcher SIGINT -> authoritative supervisor CANCELLED -> cleanup
    p,d=start('watcher_cancel',{'M3_FAKE_PLAY_SEC':'5'}); wait_state(d,'PLAYBACK_STARTED')
    w=subprocess.Popen([str(WATCHER),'--run-dir',str(d),'--poll','.03','--max-wall','8'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    time.sleep(.1);w.send_signal(signal.SIGINT);out_text,_=w.communicate(timeout=8);p.wait(timeout=8);s=state(d)
    assert w.returncode==1 and s['state']=='CANCELLED' and s['failure_class']=='USER_CANCELLED' and s['cleanup_verified'] is True, out_text
    assert token_pids(s['transaction_token'])==[]

    # supervisor SIGTERM -> cleanup
    p,d=start('supervisor_term',{'M3_FAKE_PLAY_SEC':'5'});wait_state(d,'PLAYBACK_STARTED');p.send_signal(signal.SIGTERM);p.wait(timeout=8);s=state(d)
    assert s['state']=='CANCELLED' and s['cleanup_verified'] is True and token_pids(s['transaction_token'])==[]

    # active transaction and duplicate same sequence both REFUSED by held lock
    p1,d1=start('exclusive_owner',{'M3_FAKE_PLAY_SEC':'3'});wait_state(d1,'PLAYBACK_STARTED')
    p2,d2=start('duplicate_attempt');assert p2.wait(timeout=3)==90;s2=state(d2)
    assert s2['failure_class']=='REFUSED_ACTIVE_TRANSACTION' and s2['cleanup_verified'] is True
    p1.send_signal(signal.SIGTERM);p1.wait(timeout=8)

    # stale previous transaction -> REFUSED even with unlocked lock
    stale=out/'stale';stale.mkdir();(stale/'state.json').write_text(json.dumps({'state':'PLAYBACK_STARTED','cleanup_verified':False}))
    p,d=start('stale_refused');assert p.wait(timeout=3)==91;assert state(d)['failure_class']=='REFUSED_ACTIVE_TRANSACTION'
    (stale/'state.json').write_text(json.dumps({'state':'CANCELLED','cleanup_verified':True}))

    # reused/live PID with wrong start token: watcher must not signal it
    sleeper=subprocess.Popen(['sleep','5']);reuse=out/'pid_reuse';reuse.mkdir();(reuse/'state.json').write_text(json.dumps({'run_id':'pid_reuse','state':'PLAYBACK_STARTED','cleanup_verified':False,'supervisor_pid':sleeper.pid,'supervisor_start_token':'not-the-live-token'}))
    w=subprocess.Popen([str(WATCHER),'--run-dir',str(reuse),'--poll','.03','--max-wall','1'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);time.sleep(.1);w.send_signal(signal.SIGINT);w.communicate(timeout=2)
    assert sleeper.poll() is None
    sleeper.terminate();sleeper.wait(timeout=2)

print('TRANSACTION LIFECYCLE E2E: ALL PASS')
