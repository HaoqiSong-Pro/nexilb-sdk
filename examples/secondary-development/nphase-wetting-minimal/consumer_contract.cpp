#include "nexilb/nexilb.h"
#include "nexilb_case_contract.hpp"

constexpr nexilb::example::case_contract<1, 5> contract{
    "CASE-NPHASE-WETTING-MINIMAL",
    {"model.NPhaseContactAngle"},
    {"capability.nphase", "capability.wetting",
     "capability.atomic-field-input", "capability.snapshot-field-read",
     "capability.checkpoint-restart"},
    true,
    false};
static_assert(nexilb::example::valid(contract));
static_assert(!contract.requires_particle_input);
static_assert(NEXILB_CATALOG_MODEL == 1u);

int nexilb_nphase_wetting_contract_anchor() { return 0; }

