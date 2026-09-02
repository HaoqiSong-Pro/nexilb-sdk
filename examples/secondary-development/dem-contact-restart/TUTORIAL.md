# Building the DEM contact/restart case

This case contains exactly one fixed prescribed particle in each dimension.
It does not contain two particles and makes no particle-particle or active
contact claim. Its planned public observation surface is `wall_contact.*`;
physical/contact acceptance remains `not_evaluated`.

The C11 executable is a real configuration-file consumer. From the copied
case-package root run
`nexilb_dem_contact_consumer <runtime-library> assets/2d/config.txt`; it negotiates
the ABI, finds `model.NPhaseImbDemContactAngle`, creates the model, configures,
initializes and advances one step. The particle CSV contains one row with
`fixed=1`; it is verified but is not transaction-submitted because the input
transaction slots are NULL. The config consumes its relative trajectory CSV.

The catalog must expose the exact coupled model. Wall-contact and restart
evaluation additionally bind the contact-array and checkpoint descriptors.
Particle ID, wall ID, array association and integer synchronization must be
stable; no active contact row or non-zero history is inferred from the input.

The restart form is `nexilb_dem_contact_consumer <runtime-library>
assets/3d/config.txt checkpoint`. Before attempting it, require non-NULL `checkpoint_info`,
`checkpoint_write` and `checkpoint_read` slots and an approved effective
checkpoint capability. If any gate is absent, report `BLOCKED:`; do not infer
restart support from configuration-file execution. Then, before model creation,
call `checkpoint_info` with a bounded path and reject an
unsupported format major, model ID, dimension, precision, schema digest,
capability digest or producer environment. Restore only into an EMPTY model
using the identical approved effective configuration. Never patch checkpoint
bytes or silently initialize missing history. Run the continuous and resumed
branches to the same integer targets and compare public particle and contact
arrays, force/torque results and history variables with the approved
restart policy. Destroy snapshots before resetting or destroying the model.

The verified runtime matrix records all four variants completing
write, info, reset, EMPTY-model restore and continuation to macro step 2; it
also exercises public particle and wall-contact array reads. This is bounded
API lifecycle evidence only. It does not establish active contact, contact-law
accuracy, convergence, tolerance compliance, or physical acceptance, which
remain `not_evaluated`.
