# Secondary-development contract examples

These four examples contain real C11 dynamic-library consumers, materialized
2D/3D configurations and CSV fixtures, compile-time ABI checks, and per-asset
SHA-256 manifests. The source repository does not include the proprietary
runtime binary. The consumers have been exercised across the 2D/3D and
f32/f64 v1.0.0 runtime packages; physical acceptance remains outside the scope
of these compact lifecycle examples.

The currently executable public lifecycle is the configuration-file path:
resolve the two bootstrap symbols, negotiate API v1, inspect flags and only
the minimal slots, find the exact model ID in the library catalog, create a
device context and model, call `model_set_config_json` with
`{"config_path":"assets/.../config.txt"}`, initialize, and advance one macro step.
Run the executable with the copied case-package root as the working directory.
The configuration path must use `/`, be relative to that root, and contain no
empty, `.` or `..` component. Referenced CSV and other auxiliary assets must
also stay inside the case package and use paths defined by the configuration;
the consumer does not search private source trees, the process `PATH`, or a
developer checkout.

`nexilb_case_config_path_slots_available` is the minimal slot gate. A non-NULL
`model_state` may be used for additional lifecycle assertions, and a non-NULL
`context_catalog_create` may refine the library catalog, but neither blocks the
basic path. Transactional field input, particle input, validation/geometry,
limits, field/particle/contact snapshots and checkpoint/restart each have a
separate `nexilb_case_*_slots_available` helper. Every helper is only a slot
test: the exact effective catalog capability must also be approved before the
operation. A compile success or a completed one-step configuration run is not
restart, validation or acceptance evidence.

Every future runnable example must follow the same public lifecycle:

1. Select the runtime package matching the required dimension and precision,
   then pass its dynamic-library path explicitly to the consumer.
2. Resolve only `nexilb_get_abi_version` and `nexilb_get_api`; reject a
   different ABI major or an API interval that excludes `NEXILB_API_V1`.
3. Read `library_info`, inspect `api_flags`, and check every required function
   table slot for non-null before use.
4. Open the library catalog and match the exact model and capability IDs in
   the case contract. If the runtime provides a context catalog, repeat the
   check there. `creatable: false`, an unavailable schema, or a missing
   capability is a hard stop for the operation that requires it.
5. Create one context with explicit device and memory budgets. Its runtime
   module must remain loaded until the context and all children are destroyed.
6. Create the model and set one complete JSON configuration. Use an atomic
   input transaction only when the case needs it and every required input slot
   and effective-catalog capability is present. On any write error call
   `input_abort`; never retry a partial transaction as if it committed.
7. Gate validation, geometry, limits, snapshots and checkpoint/restart
   separately. A completely absent optional slot group is skipped by the base
   config-path lifecycle; a partially populated group is a hard error.
8. Initialize and advance only to declared integer synchronization targets.
   Capture snapshots or checkpoints only after the effective capability and
   all slots in that operation group are present.
9. Destroy snapshots and reports before their model; destroy models before
   their context; destroy catalogs and errors with their table functions; only
   then unload the module.

An error output argument must point to a null `nexilb_error_t`. A returned
handle is owned by the caller and is released only with `error_destroy`.
Strings and JSON use the documented two-call buffer protocol. Consumers must
not free runtime-owned handles or cross the ABI with C++ exceptions, STL
objects, compiler-specific classes, or an allocator from the other side.

The matrix is exactly 2D/3D by f32/f64. N-phase wetting and the pure side of
fixed-body correspondence use precision-specific configs because their Newton
tolerances are precision contracts. Run from the case directory:

```text
<build>/examples/secondary-development/nphase-wetting-minimal/nexilb_nphase_wetting_consumer <runtime-2d-f32> assets/2d/config-f32.txt
<build>/examples/secondary-development/imb-coupling-minimal/nexilb_imb_coupling_consumer <runtime-3d-f64> assets/3d/config.txt
<build>/examples/secondary-development/dem-contact-restart/nexilb_dem_contact_consumer <runtime-2d-f64> assets/2d/config.txt
<build>/examples/secondary-development/pure-coupled-degeneration/nexilb_degeneration_consumer <runtime-3d-f32> assets/3d/pure-config-f32.txt assets/3d/coupled-config.txt
```

Verify every byte count and digest in `asset-manifest.json` first. Particle
CSV files are fixed-particle fixtures; configs consume their relative
trajectory CSV. Input transaction slots remain NULL, so these consumers never
call `input_begin` or submit a particle batch.
