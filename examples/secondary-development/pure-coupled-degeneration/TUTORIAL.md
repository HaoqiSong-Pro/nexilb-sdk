# Building the pure/coupled degeneration case

This is a two-run fixed-body representation correspondence contract, not a
zero-particle degeneration equivalence claim and not a hidden mode switch.
The pure config uses a fixed embedded solid; the coupled config represents the
corresponding geometry as exactly one fixed prescribed body.

The C11 executable takes one runtime and two independent relative config paths:
`nexilb_degeneration_consumer <runtime-library> assets/2d/pure-config-f32.txt
assets/2d/coupled-config.txt`. Run it from the copied case-package root. It completes the
configuration-file lifecycle for the pure model first and starts the coupled
model only if the first lifecycle succeeds. Auxiliary CSV paths remain local
to the case package. This proves neither equivalence nor acceptance; those
claims require the independent comparisons below.

The pure config is precision-specific because its Newton tolerance is part of
the f32/f64 numerical contract; select the exact path in `variants.json`.
Both runs must bind the same phase representation, topology, bulk and wetting
data, initial fields, boundary IDs, precision, dimension, integer targets and
public output definitions. Model-specific fields stay in separate config
documents. Validate and initialize each model independently, then compare only
publicly declared common fields and invariants at identical integer
synchronization points. The verifier must record both effective configuration
and catalog hashes and reject differences outside the approved mapping.

Input transaction slots are NULL, so the fixed-particle CSV is not submitted
through `input_begin`; the coupled config consumes its relative trajectory.
Validation and snapshot groups are available and separately gated. The matrix
records all four paired lifecycles but does not evaluate field correspondence.
Absence of crashes is not correspondence. Acceptance requires approved metrics
for phase conservation, partition error, interface position, wall wetting and
field differences, plus an explicit mapping between the embedded solid and
one fixed prescribed body. No metric, tolerance, zero-particle claim or
physical result is invented; all criteria remain `not_evaluated`.
