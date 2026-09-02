# Building the N-phase wetting case

This case targets exactly `model.NPhaseContactAngle` and forbids particle
input. Configure the standalone build with `cmake -S . -B build` and
`cmake --build build`; this compiles both the static contract and the C11
dynamic-library consumer. From a copied case-package root, run
`nexilb_nphase_wetting_consumer <runtime-library> assets/2d/config-f32.txt`
(choose the matching row in `variants.json`). The path is passed as the sole
`config_path` member and is never replaced by a
private-source path. Compilation alone proves no runtime capability or result.

A compatible runtime must expose the exact creatable model in its library
catalog; a context catalog, when present, must expose the same ID. The configuration
must declare dimension, precision, phase count and stable phase IDs, lattice
and boundary topology, the phase representation, bulk properties, pairwise
surface-tension data, wall IDs, wetting/contact-angle convention, integer time
grid, output fields and checkpoint policy. Units, axis order, halo policy and
field association must come from the catalog, never from array shape guesses.

For a configuration whose fields are referenced by the config file, keep its
CSV and geometry assets under the same case-package root and preserve their
declared relative paths. If the model instead requires in-memory initial
fields, require non-NULL `input_begin`, `input_write_initial_field`,
`input_commit` and `input_abort` slots plus the matching catalog capability
before starting the transaction. Do not call `input_add_particles`. Abort on
the first error. Then require successful model
and geometry validation before initialization. The acceptance contract must
define bounded phase conservation, partition error, interface/contact-line
sampling, wall normal convention and contact-angle measurement independently
of the solver. A restart cut must be an integer synchronization point; compare
the continuous and resumed trajectories with the approved TEST/VAL policy.

The 2D and 3D assets each have distinct f32 and f64 configs. Their
`wettingNewtonTolerance` values are precision-specific numerical contracts;
the runtime precision must match the file. `asset-manifest.json` binds all
four inputs by size and SHA-256.

Input transaction slots are NULL. Validation, snapshot and checkpoint/restart
are available and remain separately capability/slot gated. The runtime matrix
records all four dimension/precision lifecycle runs, but no approved reference
or tolerance was applied; conservation and contact-angle acceptance therefore
remain `not_evaluated`.
