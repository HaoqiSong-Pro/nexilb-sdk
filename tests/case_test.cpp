#include "nexilb/nexilb_case_sdk.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace {
nexilb_string_view_t view(const char *value) {
  return {value, static_cast<uint64_t>(std::strlen(value))};
}

bool pair_allowed(std::string_view chain, std::string_view model) {
  if (chain == NEXILB_CHAIN_NATIVE) {
    return model == NEXILB_MODEL_NATIVE_NSALLEN ||
           model == NEXILB_MODEL_NATIVE_NSALLEN_IMB_PRESCRIBED_MOTION;
  }
  if (chain == NEXILB_CHAIN_NPHASE_CONTACT_ANGLE) {
    return model == NEXILB_MODEL_NPHASE_CONTACT_ANGLE;
  }
  if (chain == NEXILB_CHAIN_NPHASE_IMB_DEM_CONTACT_ANGLE) {
    return model == NEXILB_MODEL_NPHASE_IMB_DEM_CONTACT_ANGLE;
  }
  return false;
}

template <typename Slot> Slot present_slot() {
  return reinterpret_cast<Slot>(static_cast<std::uintptr_t>(1u));
}
} // namespace

int main(int argc, char **argv) {
  if (argc != 5) {
    return EXIT_FAILURE;
  }
  const auto dimension = static_cast<uint32_t>(std::strtoul(argv[3], nullptr, 10));
  nexilb_scalar_type_t precision = 0u;
  if (std::string_view(argv[4]) == "f32") {
    precision = NEXILB_SCALAR_F32;
  } else if (std::string_view(argv[4]) == "f64") {
    precision = NEXILB_SCALAR_F64;
  }
  nexilb_case_variant_t variant{};
  variant.struct_size = sizeof(variant);
  variant.struct_version = NEXILB_STRUCT_VERSION_1;
  variant.dimension = dimension;
  variant.precision = precision;
  variant.chain_id = view(argv[1]);
  variant.model_id = view(argv[2]);
  if (nexilb_case_variant_shape_supported(&variant) != NEXILB_TRUE ||
      !pair_allowed(argv[1], argv[2])) {
    return EXIT_FAILURE;
  }
  nexilb_api_v1 unavailable{};
  unavailable.struct_size = sizeof(unavailable);
  if (nexilb_case_common_slots_available(&unavailable) != NEXILB_FALSE ||
      nexilb_case_coupled_slots_available(&unavailable) != NEXILB_FALSE) {
    return EXIT_FAILURE;
  }

  nexilb_api_v1 config_path{};
  config_path.struct_size = sizeof(config_path);
  config_path.library_info = present_slot<decltype(config_path.library_info)>();
  config_path.library_catalog_create =
      present_slot<decltype(config_path.library_catalog_create)>();
  config_path.catalog_count = present_slot<decltype(config_path.catalog_count)>();
  config_path.catalog_id = present_slot<decltype(config_path.catalog_id)>();
  config_path.catalog_destroy = present_slot<decltype(config_path.catalog_destroy)>();
  config_path.device_count = present_slot<decltype(config_path.device_count)>();
  config_path.context_create = present_slot<decltype(config_path.context_create)>();
  config_path.context_destroy = present_slot<decltype(config_path.context_destroy)>();
  config_path.model_create = present_slot<decltype(config_path.model_create)>();
  config_path.model_destroy = present_slot<decltype(config_path.model_destroy)>();
  config_path.model_set_config_json =
      present_slot<decltype(config_path.model_set_config_json)>();
  config_path.model_initialize =
      present_slot<decltype(config_path.model_initialize)>();
  config_path.model_step = present_slot<decltype(config_path.model_step)>();
  config_path.error_destroy = present_slot<decltype(config_path.error_destroy)>();
  if (nexilb_case_config_path_slots_available(&config_path) != NEXILB_TRUE ||
      nexilb_case_common_slots_available(&config_path) != NEXILB_TRUE ||
      nexilb_case_field_input_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_particle_input_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_validation_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_limits_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_snapshot_field_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_snapshot_particle_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_snapshot_contact_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_checkpoint_slots_available(&config_path) != NEXILB_FALSE ||
      nexilb_case_coupled_slots_available(&config_path) != NEXILB_FALSE) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
