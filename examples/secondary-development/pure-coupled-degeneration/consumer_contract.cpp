#include "nexilb/nexilb.h"
#include "nexilb_case_contract.hpp"

constexpr nexilb::example::case_contract<2, 6> contract{
    "CASE-PURE-COUPLED-DEGENERATION",
    {"model.NPhaseContactAngle", "model.NPhaseImbDemContactAngle"},
    {"capability.nphase", "capability.wetting",
     "capability.atomic-field-input", "capability.snapshot-field-read",
     "capability.integer-synchronization", "capability.independent-verification"},
    false,
    false};
static_assert(nexilb::example::valid(contract));
static_assert(contract.model_ids[0] != contract.model_ids[1]);

int nexilb_pure_coupled_degeneration_contract_anchor() { return 0; }

