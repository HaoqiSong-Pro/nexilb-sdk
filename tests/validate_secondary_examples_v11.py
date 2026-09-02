#!/usr/bin/env python3
"""Validate executable secondary-development contract bundle v1.1."""

from __future__ import annotations

import csv
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CASE_ROOT = ROOT / "examples" / "secondary-development"
CASES = {
    "nphase-wetting-minimal": "CASE-NPHASE-WETTING-MINIMAL",
    "imb-coupling-minimal": "CASE-IMB-COUPLING-MINIMAL",
    "dem-contact-restart": "CASE-DEM-CONTACT-RESTART",
    "pure-coupled-degeneration": "CASE-PURE-COUPLED-DEGENERATION",
}
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
EXPECTED_STATE = {
    "implementation": "c11_consumer_executable",
    "runtime": "compatible_runtime_config_path_lifecycle_available",
    "physical_acceptance": "not_evaluated",
}


def load(path: Path) -> dict:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"JSON root is not an object: {path}")
    return value


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def assert_relative(path: str) -> None:
    value = Path(path)
    if (not path or "\\" in path or value.is_absolute() or
            any(part in {"", ".", ".."} for part in path.split("/"))):
        raise ValueError(f"non-portable relative path: {path}")


def validate_evidence(path: Path) -> bool:
    if not path.exists():
        return False
    value = load(path)
    if (value.get("$schema") !=
            "urn:nexilb:schema:secondary-runtime-matrix-evidence:1.0" or
            value.get("execution_count") != 16 or
            value.get("scope_limit") !=
            "API lifecycle and bounded execution only; no physical, convergence or release-acceptance claim"):
        raise ValueError("runtime matrix evidence header differs")
    records = value.get("records")
    if not isinstance(records, list) or len(records) != 16:
        raise ValueError("runtime matrix must contain exactly 16 records")
    observed = set()
    for record in records:
        key = (record.get("variant"), record.get("case_id"))
        if key in observed:
            raise ValueError(f"duplicate runtime matrix record: {key}")
        observed.add(key)
        if (record.get("state") != "passed" or
                record.get("process_exit_code") != 0 or
                not SHA256_RE.fullmatch(record.get("runtime_sha256", "")) or
                not SHA256_RE.fullmatch(record.get("consumer_sha256", ""))):
            raise ValueError(f"invalid runtime matrix record: {key}")
        for item in record.get("input_files", []):
            assert_relative(item["path"])
            if item["byte_length"] < 1 or not SHA256_RE.fullmatch(item["sha256"]):
                raise ValueError(f"invalid input evidence: {key}")
    expected = {(variant, case) for variant in ("d2f32", "d2f64", "d3f32", "d3f64") for case in CASES}
    if observed != expected:
        raise ValueError("runtime matrix variant/case closure differs")
    return True


def main() -> None:
    bundle = load(ROOT / "contracts" / "secondary-case-contract-bundle-v1.1.schema.json")
    evidence_schema = load(ROOT / "contracts" / "secondary-runtime-matrix-evidence.schema.json")
    if (bundle.get("$id") != "urn:nexilb:schema:secondary-case-contract-bundle:1.1" or
            evidence_schema.get("$id") != "urn:nexilb:schema:secondary-runtime-matrix-evidence:1.0"):
        raise ValueError("v1.1 contract schema identity differs")
    evidence_present = validate_evidence(ROOT / "evidence" / "secondary-runtime-matrix.json")

    parsed = 0
    hashed_assets = 0
    for case_name, case_id in CASES.items():
        case_dir = CASE_ROOT / case_name
        documents = {
            "case": load(case_dir / "case.json"),
            "manifest": load(case_dir / "case-manifest.json"),
            "variants": load(case_dir / "variants.json"),
            "run": load(case_dir / "contract-state" / "run-manifest.json"),
            "verification": load(case_dir / "contract-state" / "verification-result.json"),
            "reference": load(case_dir / "expected" / "reference-manifest.json"),
            "acceptance": load(case_dir / "expected" / "acceptance.json"),
            "assets": load(case_dir / "asset-manifest.json"),
        }
        parsed += len(documents)
        for value in documents.values():
            if value.get("schema_version") != "1.1" or value.get("case_id") != case_id:
                raise ValueError(f"v1.1/case identity differs: {case_name}")
        if documents["case"].get("contract_state") != EXPECTED_STATE or documents["manifest"].get("contract_state") != EXPECTED_STATE:
            raise ValueError(f"executable state differs: {case_name}")
        slots = documents["case"].get("optional_slots", {})
        if slots.get("input_transaction") != "null_in_current_runtime":
            raise ValueError(f"input transaction must remain NULL: {case_name}")
        allowed_optional = "available_in_current_runtime" if evidence_present else "null_in_current_runtime"
        for name in ("validation", "snapshot", "checkpoint_restart"):
            if slots.get(name) != allowed_optional:
                raise ValueError(f"{case_name} {name} state lacks matching evidence")

        variants = documents["variants"].get("variants", [])
        observed = {(row.get("dimension"), row.get("precision")) for row in variants}
        if observed != {(2, "f32"), (2, "f64"), (3, "f32"), (3, "f64")}:
            raise ValueError(f"2D/3D f32/f64 closure differs: {case_name}")
        for row in variants:
            if row.get("physical_acceptance") != "not_evaluated":
                raise ValueError(f"false physical state: {case_name}")
            for relative in row.get("config_paths", []):
                assert_relative(relative)
                if not (case_dir / relative).is_file():
                    raise ValueError(f"missing variant config: {case_name}/{relative}")
            if case_name == "nphase-wetting-minimal" and not row["config_paths"][0].endswith(f"-{row['precision']}.txt"):
                raise ValueError("N-phase wetting precision config mismatch")
            if case_name == "pure-coupled-degeneration" and not row["config_paths"][0].endswith(f"-{row['precision']}.txt"):
                raise ValueError("pure-side precision config mismatch")

        manifest = documents["assets"]
        listed = set()
        for item in manifest.get("assets", []):
            relative = item["path"]
            assert_relative(relative)
            path = case_dir / relative
            if relative in listed or not path.is_file():
                raise ValueError(f"duplicate/missing asset: {case_name}/{relative}")
            listed.add(relative)
            if path.stat().st_size != item["size_bytes"] or digest(path) != item["sha256"]:
                raise ValueError(f"asset digest differs: {case_name}/{relative}")
            hashed_assets += 1
        actual = {path.relative_to(case_dir).as_posix() for path in (case_dir / "assets").rglob("*") if path.is_file()}
        if listed != actual:
            raise ValueError(f"asset inventory differs: {case_name}")

        if documents["verification"].get("verification_status") != "not_evaluated" or documents["reference"].get("reference_status") != "not_evaluated" or documents["acceptance"].get("verification_status") != "not_evaluated":
            raise ValueError(f"physical verification must remain not_evaluated: {case_name}")
        if any(item.get("evaluation_status") != "not_evaluated" for item in documents["acceptance"].get("criteria", [])):
            raise ValueError(f"acceptance criterion falsely evaluated: {case_name}")

        main_text = (case_dir / "main.c").read_text(encoding="utf-8")
        if "nexilb_run_" not in main_text or "NEXILB_CONSUMER_BLOCKED" not in main_text:
            raise ValueError(f"C11 consumer is not wired: {case_name}")

    dem_dir = CASE_ROOT / "dem-contact-restart"
    for particle_file in (dem_dir / "assets").glob("*/particles_*.csv"):
        with particle_file.open(newline="", encoding="utf-8") as stream:
            rows = list(csv.DictReader(stream))
        if len(rows) != 1 or rows[0].get("fixed") != "1":
            raise ValueError(f"DEM fixture is not exactly one fixed particle: {particle_file}")
    dem_semantics = "\n".join((dem_dir / path).read_text(encoding="utf-8") for path in (
        "case.json", "case-manifest.json", "TUTORIAL.md",
        "expected/reference-manifest.json", "expected/acceptance.json"))
    if re.search(r"two_or_more_particles|particle-particle and particle-wall|TST-DEM-PARTICLE-PARTICLE", dem_semantics):
        raise ValueError("DEM contract retains a two-particle/contact claim")
    pure_dir = CASE_ROOT / "pure-coupled-degeneration"
    pure_semantics = "\n".join((pure_dir / path).read_text(encoding="utf-8") for path in (
        "case.json", "case-manifest.json", "TUTORIAL.md",
        "expected/reference-manifest.json", "expected/acceptance.json"))
    if re.search(r"zero_particle_coupled|TST-DEGENERATION-ZERO-PARTICLE-COUPLED|coupled_zero_particle_config|zero particles and coupling enabled", pure_semantics):
        raise ValueError("fixed-body correspondence retains zero-particle equivalence")
    print(f"secondary-v1.1-ok: {parsed} documents, {hashed_assets} assets, evidence={evidence_present}")


if __name__ == "__main__":
    main()
