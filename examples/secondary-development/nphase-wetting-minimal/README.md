# nphase-wetting-minimal

Minimal C11 consumer for `model.NPhaseContactAngle`. It loads the selected
NexiLB v1.0.0 runtime, validates the requested model and API lifecycle,
consumes a complete 2D or 3D configuration, initializes the model, and advances
one macro step. Separate configurations are provided for f32 and f64 so the
precision-dependent nonlinear tolerances remain explicit.

This example demonstrates integration and bounded execution. It is not a
physical validation case and does not claim contact-angle accuracy,
convergence, or production-ready parameter choices.
