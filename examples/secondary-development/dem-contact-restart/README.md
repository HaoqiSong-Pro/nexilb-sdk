# dem-contact-restart

Contract-only skeleton for the contact-history and restart obligations of
`model.NPhaseImbDemContactAngle`. It asserts no geometry values, contact-law
parameters, checkpoint time, history values, tolerance, hash, hardware, or
result. Approval requires both particle-particle and particle-wall contact and
an active contact with non-zero public history at the checkpoint. A fixture
with no contact, zero history, or a pre-contact checkpoint cannot satisfy this
case.
