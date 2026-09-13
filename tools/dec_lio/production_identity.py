#!/usr/bin/env python3
"""Report and guard the native production-source boundary."""

import argparse
import hashlib
import pathlib
import subprocess
import sys


ALLOWED_SOURCE_CHANGES = {
    "src/super_lio/CMakeLists.txt",
    "src/super_lio/package.xml",
    "src/super_lio/config/geode_alpha.yaml",
    "src/super_lio/config/geode_stairs_alpha.yaml",
    "src/super_lio/config/geode_tunneling2_alpha.yaml",
    "src/super_lio/launch/dec_lio_bridge_inspect.launch",
    "src/super_lio/rviz/dec_lio_bridge_inspect.rviz",
    "src/super_lio/offline/OfflineReader.h",
    "src/super_lio/offline/OfflineReader.cpp",
    "src/super_lio/src/apps/super_lio_offline_node.cpp",
    "src/super_lio/include/dec_lio/DCRegAnalyzer.h",
    "src/super_lio/src/dec_lio/DCRegAnalyzer.cpp",
    "src/super_lio/include/dec_lio/D2ShadowAnalyzer.h",
    "src/super_lio/src/dec_lio/D2ShadowAnalyzer.cpp",
    "src/super_lio/include/dec_lio/ConsistencyAnalyzer.h",
    "src/super_lio/src/dec_lio/ConsistencyAnalyzer.cpp",
    "src/super_lio/include/dec_lio/WeakAxisAnalyzer.h",
    "src/super_lio/src/dec_lio/WeakAxisAnalyzer.cpp",
    "src/super_lio/include/lio/params.h",
    "src/super_lio/include/lio/super_lio.h",
    "src/super_lio/src/lio/params.cpp",
    "src/super_lio/src/lio/super_lio.cpp",
    "src/super_lio/src/ros/ROSWrapper.cpp",
}
FORBIDDEN = ("prob_lio", "pcg", "sa_gate")


def run(repo, *args):
    return subprocess.check_output(["git", "-C", str(repo), *args], text=True).strip()


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    repo = args.repo_root.resolve()
    try:
        head = run(repo, "rev-parse", "HEAD")
        ros1 = run(repo, "rev-parse", "origin/ros1")
        merge_base = run(repo, "merge-base", "HEAD", "origin/ros1")
        changed = run(repo, "diff", "--name-only", "origin/ros1", "--", "src/")
        changed_paths = set(changed.splitlines()) if changed else set()
    except subprocess.CalledProcessError as error:
        print(f"IDENTITY_FAIL: git query failed: {error}", file=sys.stderr)
        return 2

    print(f"head: {head}")
    print(f"origin_ros1: {ros1}")
    print(f"merge_base: {merge_base}")
    print(f"ancestry_exact: {merge_base == ros1}")
    print("source_changes:")
    for path in sorted(changed_paths):
        print(f"  {path}")

    errors = []
    if merge_base != ros1:
        errors.append("HEAD does not descend from the exact origin/ros1 base")
    unexpected = changed_paths - ALLOWED_SOURCE_CHANGES
    if unexpected:
        errors.append("unexpected source changes: " + ", ".join(sorted(unexpected)))
    for path in sorted(changed_paths):
        file_path = repo / path
        if not file_path.is_file():
            errors.append(f"missing changed source file: {path}")
            continue
        lowered = file_path.read_text(encoding="utf-8", errors="replace").lower()
        for word in FORBIDDEN:
            if word in lowered:
                errors.append(f"forbidden estimator token {word!r} in {path}")

    native_root = repo / "src/super_lio/src/lio"
    native_digest = hashlib.sha256()
    for path in sorted(native_root.rglob("*")):
        if path.is_file():
            native_digest.update(path.relative_to(native_root).as_posix().encode())
            native_digest.update(path.read_bytes())
    print(f"native_lio_source_sha256: {native_digest.hexdigest()}")
    print(f"production_code_oid: {run(repo, 'rev-parse', 'HEAD:src/super_lio')}")
    if errors:
        for error in errors:
            print(f"IDENTITY_FAIL: {error}", file=sys.stderr)
        return 1
    print("IDENTITY_PASS: native estimator boundary is clean")
    return 0


if __name__ == "__main__":
    sys.exit(main())
