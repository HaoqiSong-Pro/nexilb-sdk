#!/usr/bin/env python3
"""Install the header SDK, configure an external consumer, build, and run it."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build-dir", required=True, type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]

with tempfile.TemporaryDirectory(prefix="nexilb-headers-install-") as temporary:
    work = Path(temporary)
    prefix = work / "prefix"
    consumer_build = work / "consumer-build"
    commands = [
        ["cmake", "--install", str(args.build_dir.resolve()), "--prefix", str(prefix)],
        ["cmake", "-S", str(root / "tests/header_package_consumer"),
         "-B", str(consumer_build), "-G", "Ninja",
         f"-DNexiLBHeaders_DIR={prefix / 'lib/cmake/NexiLBHeaders'}"],
        ["cmake", "--build", str(consumer_build)],
    ]
    for command in commands:
        completed = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, check=False)
        if completed.returncode != 0:
            print(completed.stdout)
            raise SystemExit(completed.returncode)
    executable = consumer_build / ("nexilb_headers_consumer.exe" if __import__("os").name == "nt" else "nexilb_headers_consumer")
    completed = subprocess.run([str(executable)], check=False)
    if completed.returncode != 0:
        raise SystemExit("installed header consumer failed")
    forbidden = {".dll", ".so", ".dylib", ".lib", ".a", ".vts"}
    if any(path.suffix.lower() in forbidden for path in prefix.rglob("*") if path.is_file()):
        raise SystemExit("header package unexpectedly installed a runtime/result artifact")
    doc_root = prefix / "share/doc/NexiLB"
    required_documents = [
        doc_root / "LICENSE.md", doc_root / "NOTICE.md",
        doc_root / "SECURITY.md", doc_root / "README.md",
        doc_root / "README.zh-CN.md",
        doc_root / "licensing/README.md",
        doc_root / "licensing/SDK-USE-BOUNDARY.md",
        doc_root / "licensing/OFFLINE-APPLICATION.md",
        doc_root / "licensing/PRIVACY-NOTICE.md",
        doc_root / "licensing/WRITTEN-AUTHORIZATION-TEMPLATE.md",
        doc_root / "licensing/application-link.schema.json",
        doc_root / "licensing/application-link.template.json",
        doc_root / "licensing/license-metadata.json",
    ]
    missing = [str(path) for path in required_documents if not path.is_file()]
    if missing:
        raise SystemExit(f"header SDK install is missing public usage files: {missing}")
    metadata = json.loads(
        (doc_root / "licensing/license-metadata.json").read_text(encoding="utf-8")
    )
    installed_license_hash = hashlib.sha256(
        (doc_root / "LICENSE.md").read_bytes()
    ).hexdigest()
    if (metadata.get("license_id") != "LicenseRef-NexiLB-Research-Use-1.0"
            or metadata.get("sha256") != installed_license_hash
            or metadata.get("download_grants_use_rights") is not False
            or metadata.get("separate_written_authorization_required") is not True):
        raise SystemExit("installed SDK license metadata does not bind the formal text")
    application = json.loads(
        (doc_root / "licensing/application-link.template.json").read_text(
            encoding="utf-8"
        )
    )
    if (application.get("state")
            != "offline_email_application_available_not_authorization"
            or application.get("delivery_mode") != "offline-email"
            or application.get("online_endpoint") is not None
            or application.get("submission_channel", {}).get("address")
            != "haoqisong@126.com"
            or application.get("submission_channel", {}).get(
                "online_form_available"
            ) is not False
            or application.get("authorization_boundary", {}).get(
                "application_submission_grants_rights"
            ) is not False
            or application.get("authorization_boundary", {}).get(
                "automatic_issuance"
            ) is not False
            or application.get("authorization_boundary", {}).get(
                "separate_written_authorization_required"
            ) is not True):
        raise SystemExit("installed SDK licensing descriptor weakens the offline authorization boundary")
    for document in application.get("documents", {}).values():
        relative = Path(document["path"])
        installed_document = (doc_root / relative).resolve()
        installed_document.relative_to(doc_root.resolve())
        if (not installed_document.is_file()
                or hashlib.sha256(installed_document.read_bytes()).hexdigest()
                != document["sha256"]):
            raise SystemExit(
                f"installed licensing document is missing or hash-mismatched: {relative}"
            )
    runtime_probe = subprocess.run(
        ["cmake", "-S", str(root / "tests/runtime_package_probe"),
         "-B", str(work / "runtime-probe-build"), "-G", "Ninja",
         f"-DNexiLB_DIR={prefix / 'lib/cmake/NexiLB'}",
         "-DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF",
         "-DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF"],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False,
    )
    if runtime_probe.returncode == 0:
        print(runtime_probe.stdout)
        raise SystemExit("header-only install unexpectedly satisfied the runtime package")
print("installed NexiLBHeaders package consumed with bound public usage documents and no runtime artifacts")
