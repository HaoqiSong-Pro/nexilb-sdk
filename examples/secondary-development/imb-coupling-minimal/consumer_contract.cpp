#include "nexilb/nexilb.h"
#include "nexilb_case_contract.hpp"

constexpr nexilb::example::case_contract<1, 7> contract{
    "CASE-IMB-COUPLING-MINIMAL",
    {"model.NPhaseImbDemContactAngle"},
    {"capability.nphase", "capability.wetting", "capability.imb",
     "capability.dem", "capability.atomic-particle-input",
     "capability.snapshot-particle-read", "capability.resource-estimate"},
    false,
    true};
static_assert(nexilb::example::valid(contract));
static_assert(contract.requires_particle_input);
static_assert(NEXILB_ASSOCIATION_PARTICLE != NEXILB_ASSOCIATION_CONTACT);

int nexilb_imb_coupling_contract_anchor() { return 0; }

