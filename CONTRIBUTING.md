# Contributing

Proposed changes should use a pull request and pass the public-boundary,
header/package, catalog, example and ABI-layout checks.

Contributions must not include solver implementation source, private tests,
internal VTS/results, private paths or symbols, credentials, applicant data,
generated binaries, build logs, or unapproved license text. Public API changes
must be additive within ABI major 1 and update the C header, C++ wrapper,
Doxygen contract, ABI baseline, consumer tests, catalog/schema bindings,
examples, and migration notes in one reviewed change.

Numerical or runtime-availability claims require reproducible runtime evidence;
prose, screenshots and compile-only tests cannot change a descriptor from
unavailable to available.
