# Three-chain consumption entry

The SDK exposes three parallel catalog chains, never one umbrella solver:

- `chain.nexilb.native` preserves the two native model entries.
- `chain.NPhaseContactAngle` is the independent particle-free N-phase wetting
  entry and must not accept particle batches.
- `chain.NPhaseImbDemContactAngle` is the independent IMB/DEM/wetting entry and
  requires particle, contact and checkpoint capabilities for coupled CASEs.

Every chain is compiled as the exact 2D/3D by f32/f64 Release matrix. Matrix
presence is a consumer-contract check only; it does not claim that a runtime or
model has passed validation. A consumer must read the catalog, match the exact
chain and model IDs, verify all required function-table slots, and stop when a
descriptor is not creatable or its runtime/schema state is unavailable.

The two N-phase entries reflect the private implementation's actual ownership
boundaries without exposing its classes or configuration files. The pure chain
uses field input only. The coupled chain additionally owns particle batches,
IMB/DEM synchronization, contact history and committed checkpoint/restart.
Zero-particle degeneration is verified by two independent model runs; it is
not dispatch from the coupled entry into the pure entry.

