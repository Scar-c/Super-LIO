#!/usr/bin/env python3
"""Deterministic identities for production code and dataset config."""

import argparse
import hashlib
import pathlib
import sys


CODE_ROOTS = (
    pathlib.Path("src/super_lio/include"),
    pathlib.Path("src/super_lio/src"),
)
CODE_FILES = (
    pathlib.Path("CMakeLists.txt"),
    pathlib.Path("src/super_lio/CMakeLists.txt"),
    pathlib.Path("src/super_lio/package.xml"),
)


def _sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def dataset_config_sha256(config_path):
    return _sha256_bytes(pathlib.Path(config_path).read_bytes())


def production_code_files(repo_root):
    repo_root = pathlib.Path(repo_root).resolve()
    paths = set()
    for root in CODE_ROOTS:
        directory = repo_root / root
        if directory.is_dir():
            paths.update(path for path in directory.rglob("*") if path.is_file())
    paths.update(
        repo_root / path for path in CODE_FILES if (repo_root / path).is_file()
    )
    return sorted(paths, key=lambda path: path.relative_to(repo_root).as_posix())


def production_code_oid(repo_root):
    repo_root = pathlib.Path(repo_root).resolve()
    digest = hashlib.sha256()
    for path in production_code_files(repo_root):
        relative = path.relative_to(repo_root).as_posix()
        content_hash = _sha256_bytes(path.read_bytes())
        digest.update(relative.encode("utf-8"))
        digest.update(b"\0")
        digest.update(content_hash.encode("ascii"))
        digest.update(b"\n")
    return digest.hexdigest()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--config")
    args = parser.parse_args(argv)
    print(f"production_code_oid: {production_code_oid(args.repo_root)}")
    if args.config:
        print(f"dataset_config_sha256: {dataset_config_sha256(args.config)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
