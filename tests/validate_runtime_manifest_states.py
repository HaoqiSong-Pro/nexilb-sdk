#!/usr/bin/env python3
"""Validate the release-only runtime-manifest v4 contract."""

from copy import deepcopy
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests/fixtures/runtime-manifest"
SHA256 = re.compile(r"^[0-9a-f]{64}$")
PATH = re.compile(r"^(?!/)(?![A-Za-z]:)(?!.*(?:^|/)\.\.(?:/|$))(?!.*\\)[A-Za-z0-9._/-]+$")
LOAD_NAME = re.compile(r"^[A-Za-z0-9._+-]+$")
ROLES = ["runtime", "case_verify", "case_test", "public_header",
         "public_cpp_header", "abi_layout", "catalog_bundle"]


def load(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def assign_dotted(document: dict, dotted: str, value) -> None:
    target = document
    parts = dotted.split(".")
    for part in parts[:-1]:
        target = target[int(part)] if isinstance(target, list) else target[part]
    if isinstance(target, list):
        target[int(parts[-1])] = value
    else:
        target[parts[-1]] = value


def validate(document: dict) -> None:
    if document.get("schema_id") != "urn:nexilb:runtime-manifest:4" or document.get("schema_version") != 4:
        raise ValueError("schema")
    if document.get("package_state") != "release":
        raise ValueError("package state")
    if LOAD_NAME.fullmatch(document.get("runtime_load_name", "")) is None:
        raise ValueError("runtime load name")
    artifacts = document.get("artifacts", [])
    if [item.get("role") for item in artifacts] != ROLES:
        raise ValueError("roles")
    for item in artifacts:
        if (PATH.fullmatch(item.get("path", "")) is None
                or not isinstance(item.get("size"), int) or item["size"] < 1
                or SHA256.fullmatch(item.get("sha256", "")) is None):
            raise ValueError("artifact")
    if SHA256.fullmatch(document.get("artifact_set_sha256", "")) is None:
        raise ValueError("artifact set")


schema = load(ROOT / "contracts/runtime-manifest.schema.json")
assert schema["$id"] == "urn:nexilb:runtime-manifest:4"
assert schema["properties"]["package_state"]["const"] == "release"
assert "authentication" not in schema["properties"]
cmake_config = (ROOT / "cmake/NexiLBConfig.cmake.in").read_text(encoding="utf-8")
cmake_verify = (ROOT / "cmake/NexiLBVerifyPackage.cmake").read_text(encoding="utf-8")
cmake_stage = (ROOT / "cmake/NexiLBStageRuntime.cmake").read_text(encoding="utf-8")
assert "urn:nexilb:runtime-manifest:4" in cmake_verify
assert "signature" not in (cmake_config + cmake_verify + cmake_stage).lower()
assert "release" in cmake_stage
validate(load(FIXTURES / "release-valid.json"))
invalid = load(FIXTURES / "invalid-states.json")
for case in invalid:
    document = deepcopy(load(FIXTURES / case["base"]))
    for dotted, value in case["replace"].items():
        assign_dotted(document, dotted, value)
    try:
        validate(document)
    except ValueError:
        continue
    raise SystemExit(f"invalid runtime manifest was accepted: {case['case_id']}")
print(f"runtime manifest states passed: 1 release valid, {len(invalid)} invalid rejected")
