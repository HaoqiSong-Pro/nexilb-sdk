# Building the minimal IMB coupling case

This case targets `model.NPhaseImbDemContactAngle`. Its C++ file checks the ABI
contract and its C11 executable performs the configuration-file lifecycle.
From the copied case-package root run
`nexilb_imb_coupling_consumer <runtime-library> assets/2d/config.txt` (or the
3D path). Keep particle
and geometry CSV paths relative to that root exactly as declared by the
configuration. Neither compilation nor a one-step run claims that the
transactional particle-input path is available.

The approved model schema must specify all N-phase wetting fields plus the
immersed-boundary representation, particle shape and material descriptors,
fluid/particle coupling scheme, DEM integrator, synchronization ratio and
integer tick mapping. The particle batch is columnar: every column has the
same row count; stable particle IDs are unique; scalar type, component count,
association, extents, strides and byte capacity are explicit. All borrowed
arrays and string-table bytes remain valid until commit or abort returns.

When the effective schema requires in-memory particle submission, first require
non-NULL `input_begin`, `input_write_initial_field`, `input_add_particles`,
`input_commit` and `input_abort` slots and the corresponding catalog
capabilities. Otherwise stop before opening a transaction. Submit fields and
particles in one input transaction, then run model and
geometry validation. Require public validation results for overlap, wall clearance,
domain ownership, invalid inertia/mass, unsupported shapes, memory limits and
coupling stability. Resource estimates are admission checks, not promises.
After initialization compare fluid conservation, particle force/torque,
position/orientation and synchronization identities through an independent
verifier. A model that lacks any declared capability or returns a schema hash
different from the approved case manifest is rejected before allocation.

Each dimension supplies exactly one fixed-particle CSV and one prescribed
trajectory CSV. The config references the trajectory relative to its asset
directory. The particle CSV is a verified fixture for future transaction and
snapshot checks; it is not submitted because input transaction slots are NULL.

Select f32/f64 through the compatible runtime in `variants.json`; the same
dimension config bytes serve both precisions. Validation, snapshot and
checkpoint/restart are implemented and separately gated. The runtime matrix
records all four lifecycle runs, but a completed run is not approved
force/torque, field, restart-continuity or physical-acceptance evidence.
