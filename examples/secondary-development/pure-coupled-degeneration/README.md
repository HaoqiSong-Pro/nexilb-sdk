# pure-coupled-degeneration

Contract-only two-model comparison between `model.NPhaseContactAngle` and
`model.NPhaseImbDemContactAngle`. The future implementation must use two
independent configurations: a coupled model with zero particles and enabled
coupling capability, and a pure model with IMB explicitly disabled. This file
does not assert topology, bulk, descriptor hashes, numerical values,
tolerances, hardware, execution, or results. Non-zero particle or IMB input
must be rejected before the degeneration comparator is entered.
