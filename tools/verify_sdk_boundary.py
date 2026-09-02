#!/usr/bin/env python3
"""Verify the live public SDK boundary without inventory or signing baselines."""

from __future__ import annotations

import json
import os
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_ROOT_FILES = {
    ".gitattributes", ".gitignore", ".mailmap", "CITATION.cff",
    "CMakeLists.txt", "CMakePresets.json", "CONTRIBUTING.md", "LICENSE.md",
    "NOTICE.md", "README.md", "SECURITY.md",
}
EXPECTED_ROOT_DIRS = {
    ".github", "abi", "catalog", "cmake", "contracts", "evidence",
    "examples", "include", "licensing", "tests", "tools",
}
IGNORED_DIRS = {".git", "__pycache__", "_build", "build", "out"}
FORBIDDEN_DIRS = {"private", "results", "runtime", "src", "vts"}
FORBIDDEN_SUFFIXES = {
    ".a", ".cubin", ".cu", ".cuh", ".dll", ".dylib", ".exe", ".exp",
    ".key", ".lib", ".log", ".o", ".obj", ".p12", ".pdb", ".pem",
    ".pfx", ".ptx", ".pyc", ".so", ".vtk", ".vtp", ".vts",
}
TEXT_SUFFIXES = {
    "", ".c", ".cff", ".cmake", ".cpp", ".csv", ".h", ".hpp", ".in",
    ".json", ".list", ".md", ".py", ".toml", ".tsv", ".txt", ".yaml", ".yml",
}
ABSOLUTE_PATH = re.compile(r"(?<![A-Za-z0-9+.-])(?:[A-Za-z]:[/\\]|/(?:home|Users|root|workspace|workspaces|tmp)(?:/|\\))")
PRIVATE_REFERENCE = re.compile(r"(?i)(?:nexilb-engine-private|user[/\\](?:0[1-5])_|private[/\\](?:src|tests))")
PRIVATE_KEY = re.compile(r"BEGIN (?:RSA |OPENSSH |EC |DSA )?PRIVATE KEY")


def fail(message: str) -> None:
    raise SystemExit("SDK boundary violation: " + message)


def _reject_duplicate_json(path: Path) -> None:
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError(f"duplicate JSON key {key!r}")
            result[key] = value
        return result

    json.loads(
        path.read_text(encoding="utf-8"),
        object_pairs_hook=pairs,
        parse_constant=lambda value: (_ for _ in ()).throw(ValueError(f"non-finite JSON value {value}")),
    )


def main() -> None:
    root_files = {path.name for path in ROOT.iterdir() if path.is_file()}
    root_dirs = {path.name for path in ROOT.iterdir() if path.is_dir() and path.name not in IGNORED_DIRS}
    if root_files != EXPECTED_ROOT_FILES:
        fail(f"unexpected root files; extra={sorted(root_files - EXPECTED_ROOT_FILES)}, missing={sorted(EXPECTED_ROOT_FILES - root_files)}")
    if root_dirs != EXPECTED_ROOT_DIRS:
        fail(f"unexpected root directories; extra={sorted(root_dirs - EXPECTED_ROOT_DIRS)}, missing={sorted(EXPECTED_ROOT_DIRS - root_dirs)}")

    legacy_name = "".join(("c", "l", "i", "p"))
    seen_casefold: dict[str, str] = {}
    checked = 0
    for directory, dirnames, filenames in os.walk(ROOT, followlinks=False):
        base = Path(directory)
        dirnames[:] = [name for name in dirnames if name not in IGNORED_DIRS]
        for name in dirnames:
            path = base / name
            relative = path.relative_to(ROOT).as_posix()
            if path.is_symlink() or name.casefold() in FORBIDDEN_DIRS:
                fail("forbidden directory: " + relative)
        for name in filenames:
            path = base / name
            relative = path.relative_to(ROOT).as_posix()
            checked += 1
            folded = relative.casefold()
            if folded in seen_casefold and seen_casefold[folded] != relative:
                fail(f"case-insensitive path collision: {seen_casefold[folded]} / {relative}")
            seen_casefold[folded] = relative
            if path.is_symlink() or path.suffix.casefold() in FORBIDDEN_SUFFIXES:
                fail("forbidden file: " + relative)
            if path.suffix.casefold() == ".json":
                try:
                    _reject_duplicate_json(path)
                except (OSError, UnicodeError, ValueError, json.JSONDecodeError) as exc:
                    fail(f"invalid JSON {relative}: {exc}")
            if path.suffix.casefold() not in TEXT_SUFFIXES and name != "CMakeLists.txt":
                continue
            try:
                content = path.read_text(encoding="utf-8")
            except UnicodeError as exc:
                fail(f"expected UTF-8 text {relative}: {exc}")
            if re.search(rf"\b{re.escape(legacy_name)}\b", content, re.IGNORECASE):
                fail("legacy project name: " + relative)
            if PRIVATE_KEY.search(content):
                fail("private-key marker: " + relative)
            if relative != "tools/verify_sdk_boundary.py" and not relative.startswith("tests/") and (
                    ABSOLUTE_PATH.search(content) or PRIVATE_REFERENCE.search(content)):
                fail("private or host-local reference: " + relative)

    print(f"SDK boundary verified directly: {checked} files")


if __name__ == "__main__":
    main()
