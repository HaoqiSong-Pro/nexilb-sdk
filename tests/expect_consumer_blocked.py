#!/usr/bin/env python3
"""Prove that an example consumer fails closed without explicit inputs."""

import subprocess
import sys

completed = subprocess.run(
    [sys.argv[1]], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    check=False,
)
if completed.returncode != 64 or "BLOCKED:" not in completed.stdout:
    print(completed.stdout)
    raise SystemExit(
        f"consumer must return 64 with a BLOCKED diagnostic; got {completed.returncode}"
    )
print(f"consumer fail-closed contract passed: {sys.argv[1]}")
