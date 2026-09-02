#!/usr/bin/env python3
"""Verify three-chain and four-CASE public contracts without solver claims."""

from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
VARIANTS = {(2, "f32"), (2, "f64"), (3, "f32"), (3, "f64")}
CASES = {
    "nphase-wetting-minimal": ("CASE-NPHASE-WETTING-MINIMAL", ["model.NPhaseContactAngle"]),
    "imb-coupling-minimal": ("CASE-IMB-COUPLING-MINIMAL", ["model.NPhaseImbDemContactAngle"]),
    "dem-contact-restart": ("CASE-DEM-CONTACT-RESTART", ["model.NPhaseImbDemContactAngle"]),
    "pure-coupled-degeneration": (
        "CASE-PURE-COUPLED-DEGENERATION",
        ["model.NPhaseContactAngle", "model.NPhaseImbDemContactAngle"],
    ),
}
CHAINS = {
    "chain.nexilb.native": ["model.nexilb.NSAllen", "model.nexilb.NSAllenImbPrescribedMotion"],
    "chain.NPhaseContactAngle": ["model.NPhaseContactAngle"],
    "chain.NPhaseImbDemContactAngle": ["model.NPhaseImbDemContactAngle"],
}


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def validate_variant_document(document: dict) -> None:
    schema = document.get("$schema")
    if schema not in {
        "urn:nexilb:schema:case-variant-matrix:1.0",
        "urn:nexilb:schema:secondary-case-variants:1.1",
    }:
        raise ValueError("wrong variant schema")
    variants = document.get("variants")
    if not isinstance(variants, list) or len(variants) != 4:
        raise ValueError("variant matrix must contain four entries")
    pairs = {(item.get("dimension"), item.get("precision")) for item in variants}
    if pairs != VARIANTS or len({item.get("variant_id") for item in variants}) != 4:
        raise ValueError("variant matrix is not the exact 2D/3D by f32/f64 product")
    if schema.endswith(":1.0"):
        if document.get("state") != "contract_only_runtime_unavailable":
            raise ValueError("v1.0 variant document made a runtime claim")
        for item in variants:
            if item.get("build_type") != "Release" or item.get("runtime_state") != "runtime_unavailable":
                raise ValueError("v1.0 variant made an unsupported claim")
    else:
        if document.get("implementation_state") != "c11_consumer_executable":
            raise ValueError("v1.1 consumer is not executable")
        for item in variants:
            if (item.get("build_type") != "Release" or
                    item.get("runtime_state") != "compatible_runtime_config_path_lifecycle_available" or
                    item.get("physical_acceptance") != "not_evaluated" or
                    not item.get("config_paths")):
                raise ValueError("v1.1 lifecycle/physical state differs")


for directory, (case_id, model_ids) in CASES.items():
    root = ROOT / "examples/secondary-development" / directory
    variants = load(root / "variants.json")
    validate_variant_document(variants)
    if variants.get("case_id") != case_id:
        raise SystemExit(f"case variant identity mismatch: {directory}")
    manifest = load(root / "case-manifest.json")
    if manifest.get("model_ids") != model_ids:
        raise SystemExit(f"case model mapping mismatch: {directory}")
    if manifest["contract_state"] != {
        "implementation": "c11_consumer_executable",
        "runtime": "compatible_runtime_config_path_lifecycle_available",
        "physical_acceptance": "not_evaluated",
    }:
        raise SystemExit(f"case availability claim changed: {directory}")

matrix = load(ROOT / "examples/three-chain-matrix.json")
if matrix.get("state") != "contract_only_runtime_unavailable":
    raise SystemExit("three-chain matrix made a runtime claim")
actual_chains = {item.get("chain_id"): item.get("model_ids") for item in matrix.get("chains", [])}
if actual_chains != CHAINS:
    raise SystemExit("three-chain membership differs from the public catalog")
if any(item.get("variants") != ["2d-f32", "2d-f64", "3d-f32", "3d-f64"]
       for item in matrix["chains"]):
    raise SystemExit("three-chain build matrix is incomplete")

negative_root = ROOT / "tests/fixtures/case-verify-invalid"
for fixture in sorted(negative_root.glob("*.json")):
    try:
        validate_variant_document(load(fixture))
    except ValueError:
        continue
    raise SystemExit(f"negative fixture unexpectedly passed: {fixture.name}")
if len(list(negative_root.glob("*.json"))) != 4:
    raise SystemExit("negative fixture inventory changed")
print("CaseVerify passed: 3 chains, 4 CASEs, 16 CASE variants, 4 rejected fixtures")
