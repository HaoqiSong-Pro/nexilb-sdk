#!/usr/bin/env python3
"""Run every public secondary-development consumer against four runtimes."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import tempfile
from pathlib import Path


VARIANTS = {
    "d2f32": "2d",
    "d2f64": "2d",
    "d3f32": "3d",
    "d3f64": "3d",
}
CASES = {
    "nphase-wetting-minimal": (
        "nexilb_nphase_wetting_consumer",
        ("config-{precision}.txt",),
        ("RUN-COMPLETED: model=model.NPhaseContactAngle macro_step=1",),
    ),
    "imb-coupling-minimal": (
        "nexilb_imb_coupling_consumer",
        ("config.txt",),
        ("RUN-COMPLETED: model=model.NPhaseImbDemContactAngle macro_step=1",),
    ),
    "dem-contact-restart": (
        "nexilb_dem_contact_consumer",
        ("config.txt", "checkpoint"),
        ("RESTART-COMPLETED: model=model.NPhaseImbDemContactAngle macro_step=2",),
    ),
    "pure-coupled-degeneration": (
        "nexilb_degeneration_consumer",
        ("pure-config-{precision}.txt", "coupled-config.txt"),
        (
            "RUN-COMPLETED: model=model.NPhaseContactAngle macro_step=1",
            "RUN-COMPLETED: model=model.NPhaseImbDemContactAngle macro_step=1",
        ),
    ),
}


def require_file(path: Path, label: str) -> Path:
    resolved = path.resolve(strict=True)
    if not resolved.is_file() or resolved.is_symlink():
        raise ValueError(f"{label} must be a regular file: {resolved}")
    return resolved


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--consumer-build", type=Path, required=True)
    parser.add_argument(
        "--case-root",
        type=Path,
        default=Path(__file__).resolve().parents[1]
        / "examples"
        / "secondary-development",
    )
    parser.add_argument("--evidence-output", type=Path)
    args = parser.parse_args()
    runtime_root = args.runtime_root.resolve(strict=True)
    consumer_build = args.consumer_build.resolve(strict=True)
    case_root = args.case_root.resolve(strict=True)
    completed = 0
    evidence_records: list[dict[str, object]] = []
    with tempfile.TemporaryDirectory(prefix="nexilb-public-runtime-matrix-") as temporary:
        work_root = Path(temporary)
        for variant, dimension in VARIANTS.items():
            library = require_file(
                runtime_root / variant / "libnexilb_runtime.so.1",
                f"{variant} runtime",
            )
            for case_name, (executable_name, relative_arguments,
                            expected_markers) in CASES.items():
                executable = require_file(
                    consumer_build
                    / "examples"
                    / "secondary-development"
                    / case_name
                    / executable_name,
                    f"{case_name} consumer",
                )
                source_assets = case_root / case_name / "assets" / dimension
                case_work = work_root / variant / case_name
                shutil.copytree(source_assets, case_work)
                command = [
                    str(executable),
                    str(library),
                    *(value.format(precision=variant[-3:])
                      for value in relative_arguments),
                ]
                result = subprocess.run(
                    command,
                    cwd=case_work,
                    text=True,
                    capture_output=True,
                    check=False,
                )
                if result.returncode:
                    print(f"FAIL {variant} {case_name}")
                    print(result.stdout)
                    print(result.stderr)
                    return result.returncode
                missing = [
                    marker for marker in expected_markers
                    if marker not in result.stdout
                ]
                if missing:
                    print(f"FAIL {variant} {case_name}: missing {missing}")
                    print(result.stdout)
                    return 1
                print(
                    f"PASS {variant} {case_name}: "
                    + " | ".join(expected_markers)
                )
                evidence_records.append({
                    "variant": variant,
                    "dimension": int(dimension[0]),
                    "precision": variant[-3:],
                    "case_id": case_name,
                    "runtime_sha256": sha256(library),
                    "consumer_sha256": sha256(executable),
                    "input_files": [
                        {
                            "path": argument,
                            "byte_length": (case_work / argument).stat().st_size,
                            "sha256": sha256(case_work / argument),
                        }
                        for argument in command[2:]
                        if argument != "checkpoint"
                    ],
                    "required_completion_markers": list(expected_markers),
                    "process_exit_code": result.returncode,
                    "state": "passed",
                })
                completed += 1
    if args.evidence_output:
        output = args.evidence_output.resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        value = {
            "$schema": "urn:nexilb:schema:secondary-runtime-matrix-evidence:1.0",
            "schema_version": "1.0",
            "state": "verified_public_secondary_runtime_matrix",
            "scope_limit": (
                "API lifecycle and bounded execution only; no physical, "
                "convergence or release-acceptance claim"
            ),
            "execution_count": completed,
            "records": evidence_records,
        }
        temporary = output.with_name(output.name + ".tmp")
        temporary.write_text(
            json.dumps(value, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
        )
        temporary.replace(output)
    print(f"PASS: {completed} public runtime case executions completed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
