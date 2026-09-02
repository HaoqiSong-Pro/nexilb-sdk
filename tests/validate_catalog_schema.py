"""Validate catalog descriptors against the committed Draft 2020-12 schema.

The validator implements the closed keyword subset used by the catalog schema
and fails if that schema starts using an unsupported validation keyword.  This
keeps the prototype offline while still making the schema, rather than a
second set of hard-coded descriptor constants, the validation authority.
"""

from __future__ import annotations

import argparse
import copy
import json
import re
import sys
from pathlib import Path
from typing import Any


class SchemaValidationError(ValueError):
    pass


_ANNOTATIONS = {"$schema", "$id", "title", "description", "default", "examples"}
_VALIDATION_KEYWORDS = {
    "type",
    "const",
    "enum",
    "required",
    "additionalProperties",
    "properties",
    "minLength",
    "pattern",
    "minItems",
    "maxItems",
    "uniqueItems",
    "items",
    "minProperties",
    "maxProperties",
    "allOf",
    "if",
    "then",
    "else",
}


def _json_equal(left: Any, right: Any) -> bool:
    if type(left) is not type(right):
        return False
    return left == right


def _type_matches(value: Any, expected: str) -> bool:
    return {
        "object": isinstance(value, dict),
        "array": isinstance(value, list),
        "string": isinstance(value, str),
        "integer": isinstance(value, int) and not isinstance(value, bool),
        "number": isinstance(value, (int, float)) and not isinstance(value, bool),
        "boolean": isinstance(value, bool),
        "null": value is None,
    }.get(expected, False)


def _validate(instance: Any, schema: dict[str, Any], location: str = "$") -> None:
    unsupported = set(schema) - _ANNOTATIONS - _VALIDATION_KEYWORDS
    if unsupported:
        raise SchemaValidationError(
            f"{location}: unsupported schema keyword(s): {sorted(unsupported)}"
        )

    expected_type = schema.get("type")
    if expected_type is not None and not _type_matches(instance, expected_type):
        raise SchemaValidationError(f"{location}: expected type {expected_type}")
    if "const" in schema and not _json_equal(instance, schema["const"]):
        raise SchemaValidationError(f"{location}: value differs from const")
    if "enum" in schema and not any(_json_equal(instance, item) for item in schema["enum"]):
        raise SchemaValidationError(f"{location}: value is not in enum")

    if isinstance(instance, dict):
        required = schema.get("required", [])
        missing = [name for name in required if name not in instance]
        if missing:
            raise SchemaValidationError(f"{location}: missing required keys {missing}")
        properties = schema.get("properties", {})
        if schema.get("additionalProperties") is False:
            extra = sorted(set(instance) - set(properties))
            if extra:
                raise SchemaValidationError(f"{location}: additional keys {extra}")
        for name, child_schema in properties.items():
            if name in instance:
                _validate(instance[name], child_schema, f"{location}.{name}")
        if len(instance) < schema.get("minProperties", 0):
            raise SchemaValidationError(f"{location}: too few properties")
        if len(instance) > schema.get("maxProperties", sys.maxsize):
            raise SchemaValidationError(f"{location}: too many properties")

    if isinstance(instance, list):
        if len(instance) < schema.get("minItems", 0):
            raise SchemaValidationError(f"{location}: too few items")
        if len(instance) > schema.get("maxItems", sys.maxsize):
            raise SchemaValidationError(f"{location}: too many items")
        if schema.get("uniqueItems"):
            canonical = [json.dumps(item, sort_keys=True, separators=(",", ":")) for item in instance]
            if len(canonical) != len(set(canonical)):
                raise SchemaValidationError(f"{location}: duplicate array items")
        item_schema = schema.get("items")
        if item_schema is not None:
            for index, item in enumerate(instance):
                _validate(item, item_schema, f"{location}[{index}]")

    if isinstance(instance, str):
        if len(instance) < schema.get("minLength", 0):
            raise SchemaValidationError(f"{location}: string is too short")
        pattern = schema.get("pattern")
        if pattern is not None and re.search(pattern, instance) is None:
            raise SchemaValidationError(f"{location}: string does not match pattern")

    for child_schema in schema.get("allOf", []):
        _validate(instance, child_schema, location)
    if "if" in schema:
        try:
            _validate(instance, schema["if"], location)
        except SchemaValidationError:
            if "else" in schema:
                _validate(instance, schema["else"], location)
        else:
            if "then" in schema:
                _validate(instance, schema["then"], location)


def _load_json(path: Path) -> Any:
    if path.is_symlink() or not path.is_file():
        raise SchemaValidationError(f"input must be a regular non-link file: {path}")
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise SchemaValidationError(f"cannot parse {path}: {exc}") from exc


def validate_catalog(schema_path: Path, list_path: Path, directory: Path) -> list[dict[str, Any]]:
    schema = _load_json(schema_path)
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        raise SchemaValidationError("catalog schema must declare Draft 2020-12")
    if schema.get("$id") != "urn:nexilb:schema:catalog-descriptor:0.2":
        raise SchemaValidationError("unexpected catalog schema ID")

    names = [line.strip() for line in list_path.read_text(encoding="utf-8").splitlines() if line.strip()]
    if len(names) != len(set(names)):
        raise SchemaValidationError("descriptor list contains duplicate filenames")
    approved_ids = schema["properties"]["entity_id"]["enum"]
    documents: list[dict[str, Any]] = []
    for name in names:
        if re.fullmatch(r"[A-Za-z0-9._-]+\.json", name) is None:
            raise SchemaValidationError(f"invalid descriptor filename: {name!r}")
        document = _load_json(directory / name)
        _validate(document, schema)
        documents.append(document)
    actual_ids = [document["entity_id"] for document in documents]
    if len(actual_ids) != len(set(actual_ids)):
        raise SchemaValidationError("descriptor entity IDs are not unique")
    if sorted(actual_ids) != sorted(approved_ids):
        raise SchemaValidationError("descriptor inventory differs from schema enum")
    return documents


def _expect_rejected(document: dict[str, Any], schema: dict[str, Any], label: str) -> None:
    try:
        _validate(document, schema)
    except SchemaValidationError:
        return
    raise SchemaValidationError(f"negative schema fixture unexpectedly passed: {label}")


def run_self_test(schema_path: Path, documents: list[dict[str, Any]]) -> None:
    schema = _load_json(schema_path)
    base = documents[0]

    mutated = copy.deepcopy(base)
    mutated["descriptor_revision"]["minor"] = 1
    _expect_rejected(mutated, schema, "stale descriptor revision")

    mutated = copy.deepcopy(base)
    mutated["private_path"] = "D:/private/source.cu"
    _expect_rejected(mutated, schema, "additional private field")

    mutated = copy.deepcopy(base)
    mutated["status"]["runtime"] = "claimed_without_runtime_binding"
    _expect_rejected(mutated, schema, "false runtime availability")

    mutated = copy.deepcopy(base)
    mutated["kind"]["code"] = 14 if mutated["kind"]["id"] != "model_chain" else 1
    _expect_rejected(mutated, schema, "kind/code mismatch")

    secondary = next(document for document in documents if document["entity_id"].startswith("CASE-"))
    mutated = copy.deepcopy(secondary)
    mutated["relations"].pop()
    _expect_rejected(mutated, schema, "secondary case missing relation")

    mutated = copy.deepcopy(secondary)
    mutated["payload"]["contract_state"] = "available"
    _expect_rejected(mutated, schema, "secondary case false availability")

    mutated = copy.deepcopy(secondary)
    mutated["blocking_requirement_ids"] = ["requirement" + ".legacy"]
    _expect_rejected(mutated, schema, "legacy requirement ID")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--schema", required=True, type=Path)
    parser.add_argument("--descriptor-list", required=True, type=Path)
    parser.add_argument("--descriptor-directory", required=True, type=Path)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)
    try:
        documents = validate_catalog(
            args.schema.resolve(strict=True),
            args.descriptor_list.resolve(strict=True),
            args.descriptor_directory.resolve(strict=True),
        )
        if args.self_test:
            run_self_test(args.schema.resolve(strict=True), documents)
    except (KeyError, OSError, SchemaValidationError) as exc:
        print(f"catalog schema validation failed: {exc}", file=sys.stderr)
        return 1
    print(f"validated {len(documents)} catalog descriptors against Draft 2020-12 schema")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
