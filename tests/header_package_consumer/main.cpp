#include "nexilb/nexilb_case_sdk.h"

#include <cstring>

int main() {
  nexilb_case_variant_t variant{};
  variant.struct_size = sizeof(variant);
  variant.struct_version = NEXILB_STRUCT_VERSION_1;
  variant.dimension = NEXILB_CASE_DIMENSION_3;
  variant.precision = NEXILB_SCALAR_F64;
  variant.chain_id = {NEXILB_CHAIN_NPHASE_CONTACT_ANGLE,
                      sizeof(NEXILB_CHAIN_NPHASE_CONTACT_ANGLE) - 1u};
  variant.model_id = {NEXILB_MODEL_NPHASE_CONTACT_ANGLE,
                      sizeof(NEXILB_MODEL_NPHASE_CONTACT_ANGLE) - 1u};
  return nexilb_case_variant_shape_supported(&variant) == NEXILB_TRUE ? 0 : 1;
}

