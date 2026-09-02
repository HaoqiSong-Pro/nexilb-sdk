# CMake package boundaries

This source tree defines two deliberately different config-mode packages.

`NexiLBHeaders` is the public package produced by this SDK repository. It
exports `NexiLB::Headers` and contains no runtime. Installing it must not make
`find_package(NexiLB ...)` succeed. Its install tree also carries the approved
LICENSE, NOTICE, security policy, SDK README and licensing/application metadata
under `share/doc/NexiLB`; these files do
not grant use rights or turn the header package into a runtime package.

`NexiLB` is the v1.0.0 runtime-package contract. Its generated target file
must provide imported targets
`NexiLB::Runtime`, `NexiLB::CaseVerify`, and `NexiLB::CaseTest`. The config
file rejects wrong target kinds, then verifies all seven package artifacts
(runtime, CaseVerify, CaseTest, both public headers, ABI layout and catalog)
against the colocated runtime manifest before returning from `find_package`.
`NexiLB::Runtime` carries public include requirements and a platform-specific
shared-library location; the two case tools are imported executables. None of
the targets may refer to a private build tree.

Runtime manifest v4 accepts only the `release` state. CMake verifies the
closed artifact set, relative paths, byte lengths, individual SHA-256 values
and the aggregate artifact-set digest before exposing imported targets.

Consumers select one exact package directory with `NO_DEFAULT_PATH`. They do
not construct include/library paths, search a system package registry, or use
an environment library path to repair a package. Runtime binaries remain
outside the SDK Git repository and are delivered in the versioned release
packages.
