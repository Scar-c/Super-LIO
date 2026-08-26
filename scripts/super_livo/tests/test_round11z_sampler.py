#!/usr/bin/env python3
"""Round11Z Z-T5..T11: camera temporal sampler seam + parallel-safety audit."""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[3]
SRC = ROOT / "src/super_lio/src"
INCLUDE = ROOT / "src/super_lio/include"


def main():
    ok = True

    def expect(name, cond, detail=""):
        nonlocal ok
        print("%s: %s %s" % (name, "PASS" if cond else "FAIL", detail))
        ok &= cond

    wrapper_cpp = (SRC / "ros/ROSWrapper.cpp").read_text(encoding="utf-8")
    wrapper_h = (INCLUDE / "ros/ROSWrapper.h").read_text(encoding="utf-8")
    reader = (SRC / "offline/OfflineReader.cpp").read_text(encoding="utf-8")

    # Z-T5: decimated image never enters the S0 camera queue — the sampler
    # returns before CameraFrame construction / camera_buffer_.push
    accept_ok = "shouldAcceptCameraFrame()" in wrapper_cpp
    pre_queue = wrapper_cpp.find("shouldAcceptCameraFrame()") < wrapper_cpp.find(
        "camera_buffer_.push(")
    expect("Z-T5 sampler before queue insertion", accept_ok and pre_queue)

    # Z-T6: accepted image preserves timestamp/offset semantics (frame
    # construction untouched after the accept gate)
    frame_ts = wrapper_cpp.find("frame.timestamp = msg->header.stamp")
    accept_site = wrapper_cpp.find("shouldAcceptCameraFrame()")
    expect("Z-T6 accepted frame semantics preserved",
           frame_ts > accept_site)

    # Z-T7: online/offline share the same seam — HandleImage is the common
    # ingress; OfflineReader calls wrapper.HandleImage for raw + compressed
    calls = len(re.findall(r"wrapper\.HandleImage\(|HandleImage\(msg\)", reader))
    expect("Z-T7 offline raw+compressed share HandleImage", calls >= 2)

    # Z-T8: exactly one sampling decision per source frame (one counter
    # increment per HandleImage invocation)
    counter_sites = len(re.findall(r"raw_camera_input_\+\+", wrapper_cpp))
    expect("Z-T8 single decision per frame", counter_sites == 1)

    # Z-T11: no vector<bool> / bit-packed parallel writes in the sampler path
    grep = subprocess.run(
        ["git", "grep", "-n", "-E",
         r"std::vector[[:space:]]*<[[:space:]]*bool|vector[[:space:]]*<[[:space:]]*bool",
         "--", "src"],
        cwd=str(ROOT), text=True, capture_output=True)
    vector_bool = [l for l in grep.stdout.splitlines()
                   if "Bvalid" not in l and "comment" not in l.lower()
                   and "//" not in l.split(":", 2)[-1].lstrip()]
    expect("Z-T11 no concurrent vector<bool>", len(vector_bool) == 0,
           str(vector_bool[:2]))

    print("Z-T5..T8/T11: %s" % ("ALL PASS" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
