#include "nexilb/nexilb.h"
#include "nexilb_case_contract.hpp"

constexpr nexilb::example::case_contract<1, 7> contract{
    "CASE-DEM-CONTACT-RESTART",
    {"model.NPhaseImbDemContactAngle"},
    {"capability.dem", "capability.contact-history",
     "capability.particle-particle-contact", "capability.particle-wall-contact",
     "capability.checkpoint-inspection", "capability.checkpoint-restart",
     "capability.snapshot-contact-read"},
    true,
    true};
static_assert(nexilb::example::valid(contract));
static_assert(contract.requires_checkpoint_restart);
static_assert(contract.requires_particle_input);
static_assert(NEXILB_ASSOCIATION_CONTACT == 4u);

int nexilb_dem_contact_restart_contract_anchor() { return 0; }

