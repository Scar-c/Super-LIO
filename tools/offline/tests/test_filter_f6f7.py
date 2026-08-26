#!/usr/bin/env python3
"""Behavioral TDD for F6/F7: exact record-Time preservation and
deterministic closing of every opened bag handle on all exit paths.

Uses an in-process mock of the rosbag module so handle open/close counts
are observable (not source-string matching).
"""
import sys
import types


class FakeTime:
    def __init__(self, sec, nsec):
        self.sec = sec
        self.nsec = nsec

    def to_nsec(self):
        return self.sec * 1000000000 + self.nsec

    def to_sec(self):
        return self.sec + self.nsec / 1e9


class FakeInfo:
    def __init__(self, count):
        self.message_count = count


class FakeBag:
    opened = []
    closed = []
    writes = []
    renamed = []
    removed = []
    fail_read = None
    times = []

    @classmethod
    def reset(cls):
        cls.opened = []
        cls.closed = []
        cls.writes = []
        cls.renamed = []
        cls.removed = []
        cls.fail_read = None
        cls.times = []

    def __init__(self, path, mode="r", compression=None):
        self.path = path
        self.mode = mode
        self.is_closed = False
        FakeBag.opened.append((path, mode))

    def get_type_and_topic_info(self):
        # bag name encodes its topics: "L" lidar, "I" imu, "C" camera
        topics = {}
        if "L" in self.path:
            topics["/lidar"] = FakeInfo(2)
        if "I" in self.path:
            topics["/imu"] = FakeInfo(2)
        if "C" in self.path:
            topics["/camera"] = FakeInfo(1)
        return ({}, topics)

    def read_messages(self, topics):
        # deterministic synthetic stream with nontrivial nanoseconds
        if self.mode != "r":
            raise RuntimeError("read on writer")
        base = 1699999999
        seq = [0]

        def gen():
            for step, topic in enumerate(topics + topics):
                if FakeBag.fail_read and step >= FakeBag.fail_read:
                    raise RuntimeError("injected read failure")
                ts = FakeTime(base, 123456789 + step * 7)
                FakeBag.times.append(ts)
                yield topic, None, ts
                seq[0] += 1
        return gen()

    def close(self):
        self.is_closed = True
        FakeBag.closed.append(self.path)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()

    def write(self, topic, msg, t=None):
        FakeBag.writes.append((topic, t))


def patch_rosbag(module_name):
    """Monkeypatch the filter module's rosbag/rospy names with the mock."""
    import importlib.util
    import pathlib
    here = pathlib.Path(__file__).resolve()
    root = here.parents[3]
    spec = importlib.util.spec_from_file_location(
        "filter_mcd_under_test",
        str(root / "tools/offline/filter_mcd.py"))
    filter_mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(filter_mod)
    fake_rosbag = types.ModuleType("rosbag")
    fake_rosbag.Bag = FakeBag
    filter_mod.rosbag = fake_rosbag
    return filter_mod


def main():
    ok = True
    import sys as _sys

    def expect(name, cond):
        nonlocal ok
        print("%s: %s" % (name, "PASS" if cond else "FAIL"))
        ok &= cond

    def run_main(argv):
        import os as _os
        saved = _sys.argv[1:]
        real_rename = _os.rename
        real_remove = _os.remove
        real_exists = _os.path.exists
        _sys.argv[1:] = argv
        _os.rename = lambda src, dst: FakeBag.renamed.append((src, dst))
        _os.remove = lambda path: FakeBag.removed.append(path)
        _os.path.exists = lambda path: True
        try:
            return filter_mod.main()
        finally:
            _sys.argv[1:] = saved
            _os.rename = real_rename
            _os.remove = real_remove
            _os.path.exists = real_exists

    filter_mod = patch_rosbag("tools.offline.filter_mcd")

    # --- success path: both source bags + writer close; Time exact ---
    FakeBag.reset()
    try:
        run_main([
            "--lidar", "/lidar", "--imu", "/imu",
            "--out", "/tmp/fake", "--bags", "/b/LI", "/b/LI",
        ])
    except SystemExit as e:
        expect("success no SystemExit", False)
        ok &= False
    # topic_counts runs once per bag in the avail_all pre-pass + once in the
    # main open loop -> 2 reads per bag, plus one writer.
    opened_read = [p for p, m in FakeBag.opened if m == "r"]
    # 2 topic_counts (avail_all) + 2 topic_counts (main loop) + 2 read bags
    expect("F7 success readers opened", len(opened_read) == 6)
    expect("F7 success all handles closed",
           len(FakeBag.closed) == len(FakeBag.opened))
    writer = [p for p, m in FakeBag.opened if m == "w"]
    expect("F7 writer opened", len(writer) == 1)
    expect("F7 writer closed", writer[0] in FakeBag.closed)
    # F6: every write passed the ORIGINAL FakeTime object (identity, not a
    # from_sec reconstruction); all 8 records preserved exactly.
    got_writes = FakeBag.writes
    created = FakeBag.times
    expect("F6 exact Time object identity",
           len(got_writes) == 8 and all(
               t is not None and any(t is o for o in created)
               for _, t in got_writes))
    expect("F6 all record ns preserved exactly",
           sorted(t.to_nsec() for _, t in got_writes) == sorted(
               o.to_nsec() for o in created))
    expect("F6 tie determinism",
           [w[0] for w in got_writes] ==
           ["/lidar", "/lidar", "/imu", "/imu",
            "/lidar", "/lidar", "/imu", "/imu"])

    # --- required-topic failure after open (topic_counts handles) ---
    FakeBag.reset()
    exited = False
    try:
        run_main([
            "--lidar", "/lidar", "--imu", "/imu",
            "--out", "/tmp/fake", "--bags", "/b/I", "/b/I",  # no bag has lidar
        ])
    except SystemExit:
        exited = True
    expect("F7 required-topic SystemExit", exited)
    expect("F7 required-topic handles closed",
           len(FakeBag.opened) > 0 and
           len(FakeBag.closed) == len(FakeBag.opened))

    # --- write/iteration exception mid-stream: all handles close ---
    FakeBag.reset()
    FakeBag.fail_read = 2
    raised = False
    try:
        run_main([
            "--lidar", "/lidar", "--imu", "/imu",
            "--out", "/tmp/fake", "--bags", "/b/LI", "/b/LI",
        ])
    except RuntimeError:
        raised = True
    expect("F7 exception propagates", raised)
    expect("F7 exception path all handles closed",
           len(FakeBag.opened) > 0 and
           len(FakeBag.closed) == len(FakeBag.opened))

    print("FILTER F6/F7 BEHAVIORAL TDD:", "ALL PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
