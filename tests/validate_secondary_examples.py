#!/usr/bin/env python3
"""Run the deep CASE validator on its frozen contract subset, then overlays."""

from pathlib import Path
import json
import re
import shutil
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True
from validate_secondary_examples_v11 import main as validate_v11

if __name__ == "__main__":
    validate_v11()
    raise SystemExit(0)

root = Path(__file__).resolve().parents[1]
case_root = root / "examples/secondary-development"
cases = ("nphase-wetting-minimal", "imb-coupling-minimal",
         "dem-contact-restart", "pure-coupled-degeneration")
contract_files = (
    "README.md", "acceptance.schema.json", "case-config-envelope.schema.json",
    "case-manifest.schema.json", "media-manifest.schema.json",
    "reference-manifest.schema.json", "run-manifest.schema.json",
    "verification-result.schema.json",
)
case_files = (
    "CMakeLists.txt", "README.md", "case.json", "case-manifest.json",
    "expected/acceptance.json", "expected/reference-manifest.json",
    "contract-state/run-manifest.json",
    "contract-state/verification-result.json",
    "contract-state/media-manifest.json",
)


def validate_config_path_request(raw: str) -> None:
    if re.search(r"\\u[0-9A-Fa-f]{4}", raw):
        raise ValueError("Unicode escape is forbidden; use original UTF-8")
    value = json.loads(raw)
    if not isinstance(value, dict) or set(value) != {"config_path"}:
        raise ValueError("request must contain only config_path")
    path = value["config_path"]
    if (not isinstance(path, str) or not path or path.startswith(("/", "\\"))
            or (len(path) >= 2 and path[1] == ":") or "\\" in path
            or "//" in path or any(part in {"", ".", ".."} for part in path.split("/"))
            or any(ord(character) < 0x20 for character in path)):
        raise ValueError("config_path is not normalized and relative")


config_schema = json.loads(
    (root / "contracts/config-path-request.schema.json").read_text(encoding="utf-8")
)
if (config_schema.get("$id") != "urn:nexilb:schema:config-path-request:1.0"
        or config_schema.get("required") != ["config_path"]
        or config_schema.get("additionalProperties") is not False):
    raise SystemExit("config-path request schema is not the single-member contract")
config_fixtures = root / "tests/fixtures/config-path-request"
validate_config_path_request((config_fixtures / "valid.json").read_text(encoding="utf-8"))
invalid_requests = json.loads(
    (config_fixtures / "invalid-cases.json").read_text(encoding="utf-8")
)
for case in invalid_requests:
    try:
        validate_config_path_request(case["raw_request"])
    except (ValueError, json.JSONDecodeError):
        continue
    raise SystemExit(f"invalid config-path request accepted: {case['case_id']}")

variant_schema = json.loads(
    (root / "contracts/case-variant-matrix.schema.json").read_text(encoding="utf-8")
)
variant_properties = variant_schema["properties"]["variants"]["items"]["properties"]
if (variant_properties["dimension"].get("enum") != [2, 3]
        or variant_properties["precision"].get("enum") != ["f32", "f64"]):
    raise SystemExit("public dimension/precision domain is not closed")
digest_contract = json.loads(
    (root / "catalog/digest-canonicalization-v1.json").read_text(encoding="utf-8")
)
if digest_contract != {
    "schema_id": "urn:nexilb:catalog-digest-canonicalization:1",
    "schema_version": 1, "scope": "effective_catalog",
    "hash_algorithm": "sha256", "kind_order": [14, 1],
    "kind_names": ["model_chain", "model"],
    "item_order": "catalog_index_ascending",
    "record_source": "catalog_descriptor_json",
    "record_bytes": "exact_utf8_excluding_trailing_nul",
    "record_suffix_hex": "0a", "json_recanonicalization": "forbidden",
}:
    raise SystemExit("effective catalog digest contract is inconsistent")

with tempfile.TemporaryDirectory(prefix="nexilb-case-contract-") as temporary:
    mirror = Path(temporary)
    for relative in contract_files:
        target = mirror / "contracts" / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(root / "contracts" / relative, target)
    mirror_test = mirror / "tests/VerifySecondaryDevelopmentContracts.cmake"
    mirror_test.parent.mkdir(parents=True)
    shutil.copyfile(root / "tests/VerifySecondaryDevelopmentContracts.cmake", mirror_test)
    for case in cases:
        destination = mirror / "examples/secondary-development" / case
        for relative in case_files:
            source = case_root / case / relative
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
        for required in ("TUTORIAL.md", "consumer_contract.cpp", "main.c", "variants.json"):
            if not (case_root / case / required).is_file():
                raise SystemExit(f"missing {required} for {case}")
        variants = json.loads((case_root / case / "variants.json").read_text(encoding="utf-8"))
        observed = {(item["dimension"], item["precision"]) for item in variants["variants"]}
        if observed != {(2, "f32"), (2, "f64"), (3, "f32"), (3, "f64")}:
            raise SystemExit(f"dimension/precision variant closure differs for {case}")
        cmake_text = (case_root / case / "CMakeLists.txt").read_text(encoding="utf-8")
        if ("consumer_contract.cpp" not in cmake_text or "cxx_std_17" not in cmake_text
                or "main.c" not in cmake_text or "c_std_11" not in cmake_text
                or "CMAKE_DL_LIBS" not in cmake_text):
            raise SystemExit(f"consumer compile target is not wired for {case}")
        main_text = (case_root / case / "main.c").read_text(encoding="utf-8")
        if (("nexilb_run_config_case" not in main_text and
             "nexilb_run_checkpoint_case" not in main_text) or
                "NEXILB_CONSUMER_BLOCKED" not in main_text):
            raise SystemExit(f"runtime consumer lifecycle is not wired for {case}")
    completed = subprocess.run(
        ["cmake", "-P", str(mirror_test)], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
    )
    if completed.returncode != 0:
        print(completed.stdout)
        raise SystemExit(completed.returncode)
print("deep contracts, config-path lexical gate, digest order, and four 2D/3D f32/f64 CASE overlays validated")
