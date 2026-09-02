# Secondary-development contract schemas

This directory defines the contract-only JSON vocabulary for the four public
secondary-development cases.  The schemas describe required evidence without
claiming that a solver, runtime, reference asset, tolerance, digest, GPU, or
verification result exists.

Schema identifiers are immutable `urn:nexilb:schema:*:1.0` identifiers.  A
future incompatible contract must use a new identifier; these files must not
be silently reinterpreted.

All file references are case-root-relative portable paths.  Absolute paths,
parent traversal, empty path segments, backslashes, and Windows device names
are outside the contract.
