/** @file nexilb_case_sdk.h Consumer-side CASE and chain contract helpers. */
#ifndef NEXILB_NEXILB_CASE_SDK_H
#define NEXILB_NEXILB_CASE_SDK_H

#include "nexilb/nexilb.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Stable catalog identifier for the native NexiLB model chain. */
#define NEXILB_CHAIN_NATIVE "chain.nexilb.native"
/** Stable catalog identifier for the N-phase contact-angle chain. */
#define NEXILB_CHAIN_NPHASE_CONTACT_ANGLE "chain.NPhaseContactAngle"
/** Stable catalog identifier for the N-phase/IMB/DEM contact-angle chain. */
#define NEXILB_CHAIN_NPHASE_IMB_DEM_CONTACT_ANGLE \
  "chain.NPhaseImbDemContactAngle"
/** Stable catalog identifier for the native NSAllen model. */
#define NEXILB_MODEL_NATIVE_NSALLEN "model.nexilb.NSAllen"
/** Stable catalog identifier for the prescribed-motion native model. */
#define NEXILB_MODEL_NATIVE_NSALLEN_IMB_PRESCRIBED_MOTION \
  "model.nexilb.NSAllenImbPrescribedMotion"
/** Stable catalog identifier for the N-phase contact-angle model. */
#define NEXILB_MODEL_NPHASE_CONTACT_ANGLE "model.NPhaseContactAngle"
/** Stable catalog identifier for the coupled N-phase/IMB/DEM model. */
#define NEXILB_MODEL_NPHASE_IMB_DEM_CONTACT_ANGLE \
  "model.NPhaseImbDemContactAngle"

/** Two-dimensional CASE variant selector. */
#define NEXILB_CASE_DIMENSION_2 UINT32_C(2)
/** Three-dimensional CASE variant selector. */
#define NEXILB_CASE_DIMENSION_3 UINT32_C(3)
/** CASE variant requires no additional run-plan behavior. */
#define NEXILB_CASE_VARIANT_FLAGS_NONE UINT64_C(0)
/** CASE variant requires a particle-input transaction. */
#define NEXILB_CASE_VARIANT_FLAG_PARTICLE_INPUT_REQUIRED UINT64_C(1)
/** CASE variant requires a checkpoint write/read restart sequence. */
#define NEXILB_CASE_VARIANT_FLAG_CHECKPOINT_RESTART_REQUIRED UINT64_C(2)
/** CASE variant compares independently executed model representations. */
#define NEXILB_CASE_VARIANT_FLAG_INDEPENDENT_EQUIVALENCE_RUN UINT64_C(4)

/** Compile-time/run-plan value. It is not accepted directly by the runtime. */
typedef struct nexilb_case_variant_t {
  uint32_t struct_size; /**< Total bytes provided for this descriptor. */
  uint32_t struct_version; /**< Must be NEXILB_STRUCT_VERSION_1. */
  uint32_t dimension; /**< NEXILB_CASE_DIMENSION_2 or _3. */
  nexilb_scalar_type_t precision; /**< NEXILB_SCALAR_F32 or _F64. */
  nexilb_string_view_t chain_id; /**< Exact catalog chain identifier. */
  nexilb_string_view_t model_id; /**< Exact catalog model identifier. */
  uint64_t flags; /**< Bitwise OR of NEXILB_CASE_VARIANT_FLAG_* values. */
} nexilb_case_variant_t;

/** Required byte size of a version-1 CASE variant descriptor. */
#define NEXILB_CASE_VARIANT_V1_MANDATORY_SIZE \
  ((uint64_t)sizeof(nexilb_case_variant_t))
/** Minimum API table size needed by the config-path lifecycle gate. */
#define NEXILB_API_V1_CONFIG_PATH_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, reserved))
/** Minimum API table size needed by field-input transaction slots. */
#define NEXILB_API_V1_FIELD_INPUT_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, model_validate))
/** Minimum API table size needed by particle-input transaction slots. */
#define NEXILB_API_V1_PARTICLE_INPUT_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, input_commit))
/** Minimum API table size needed by validation and report slots. */
#define NEXILB_API_V1_VALIDATION_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, model_limits_evaluate_json))
/** Minimum API table size needed by limit and estimate slots. */
#define NEXILB_API_V1_LIMITS_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, model_initialize))
/** Minimum API table size needed by snapshot and read slots. */
#define NEXILB_API_V1_SNAPSHOT_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, checkpoint_info))
/** Minimum API table size needed by checkpoint inspection/read/write slots. */
#define NEXILB_API_V1_CHECKPOINT_SLOTS_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, error_info))

/* Retained source-level aliases. "Common" means only config-path execution. */
/** Source-compatible alias for the config-path lifecycle boundary. */
#define NEXILB_API_V1_COMMON_CASE_SLOTS_SIZE \
  NEXILB_API_V1_CONFIG_PATH_SLOTS_SIZE
/** Source-compatible alias for the complete coupled-case table boundary. */
#define NEXILB_API_V1_COUPLED_CASE_SLOTS_SIZE \
  NEXILB_API_V1_CHECKPOINT_SLOTS_SIZE

/**
 * Return true only for the public 2D/3D by f32/f64 build matrix.
 * @param variant CASE variant descriptor to validate.
 * @return NEXILB_TRUE when all mandatory descriptor fields are supported.
 */
static inline nexilb_bool_t
nexilb_case_variant_shape_supported(const nexilb_case_variant_t *variant) {
  if (variant == NULL ||
      variant->struct_size < sizeof(nexilb_case_variant_t) ||
      variant->struct_version != NEXILB_STRUCT_VERSION_1 ||
      (variant->dimension != NEXILB_CASE_DIMENSION_2 &&
       variant->dimension != NEXILB_CASE_DIMENSION_3) ||
      (variant->precision != NEXILB_SCALAR_F32 &&
       variant->precision != NEXILB_SCALAR_F64) ||
      variant->chain_id.data == NULL || variant->chain_id.byte_size == 0u ||
      variant->model_id.data == NULL || variant->model_id.byte_size == 0u) {
    return NEXILB_FALSE;
  }
  return NEXILB_TRUE;
}

/**
 * Check only the slots required by config-path execution.
 *
 * This is a necessary slot gate, not a capability claim.  The caller must
 * still verify ABI/API negotiation, library flags, the exact catalog model,
 * and the configuration package.  State queries, an effective context
 * catalog, input transactions, validation, snapshots and checkpoints are not
 * prerequisites for `set_config_json -> initialize -> step`.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when every mandatory config-path lifecycle slot exists.
 */
static inline nexilb_bool_t
nexilb_case_config_path_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL ||
      api->struct_size < NEXILB_API_V1_CONFIG_PATH_SLOTS_SIZE ||
      api->library_info == NULL || api->library_catalog_create == NULL ||
      api->catalog_count == NULL || api->catalog_id == NULL ||
      api->catalog_destroy == NULL || api->device_count == NULL ||
      api->context_create == NULL || api->context_destroy == NULL ||
      api->model_create == NULL || api->model_destroy == NULL ||
      api->model_set_config_json == NULL || api->model_initialize == NULL ||
      api->model_step == NULL || api->error_destroy == NULL) {
    return NEXILB_FALSE;
  }
  return NEXILB_TRUE;
}

/**
 * Check the atomic field-input transaction slot group.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when the complete field-input slot group exists.
 * @note A true result is not a catalog capability claim.
 */
static inline nexilb_bool_t
nexilb_case_field_input_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL ||
      api->struct_size < NEXILB_API_V1_FIELD_INPUT_SLOTS_SIZE ||
      api->input_begin == NULL || api->input_write_initial_field == NULL ||
      api->input_commit == NULL || api->input_abort == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check particle input layered on the atomic field-input gate.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when particle and field-input transaction slots exist.
 */
static inline nexilb_bool_t
nexilb_case_particle_input_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL ||
      api->struct_size < NEXILB_API_V1_PARTICLE_INPUT_SLOTS_SIZE ||
      nexilb_case_field_input_slots_available(api) != NEXILB_TRUE ||
      api->input_add_particles == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check model/geometry validation and readable report slots.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when the complete validation/report slot group exists.
 */
static inline nexilb_bool_t
nexilb_case_validation_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL ||
      api->struct_size < NEXILB_API_V1_VALIDATION_SLOTS_SIZE ||
      api->model_validate == NULL || api->model_geometry_check == NULL ||
      api->validation_report_count == NULL ||
      api->validation_report_get == NULL ||
      api->validation_report_destroy == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check limit evaluation and resource-estimation slots.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when both limit and resource-estimation slots exist.
 */
static inline nexilb_bool_t
nexilb_case_limits_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL || api->struct_size < NEXILB_API_V1_LIMITS_SLOTS_SIZE ||
      api->model_limits_evaluate_json == NULL ||
      api->model_estimate_resources_json == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check immutable snapshot lifecycle and public field-read slots.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when the snapshot and field-read slot group exists.
 */
static inline nexilb_bool_t
nexilb_case_snapshot_field_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL || api->struct_size < NEXILB_API_V1_SNAPSHOT_SLOTS_SIZE ||
      api->snapshot_create == NULL || api->snapshot_info == NULL ||
      api->field_read == NULL || api->snapshot_destroy == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check particle reads layered on the field snapshot lifecycle.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when snapshot lifecycle and particle-read slots exist.
 */
static inline nexilb_bool_t
nexilb_case_snapshot_particle_slots_available(const nexilb_api_v1 *api) {
  if (nexilb_case_snapshot_field_slots_available(api) != NEXILB_TRUE ||
      api->particle_read == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check contact reads layered on the field snapshot lifecycle.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when snapshot lifecycle and contact-read slots exist.
 */
static inline nexilb_bool_t
nexilb_case_snapshot_contact_slots_available(const nexilb_api_v1 *api) {
  if (nexilb_case_snapshot_field_slots_available(api) != NEXILB_TRUE ||
      api->contact_read == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Check checkpoint inspection/write/read slots.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE when the complete checkpoint slot group exists.
 * @note A true result is not a catalog capability claim.
 */
static inline nexilb_bool_t
nexilb_case_checkpoint_slots_available(const nexilb_api_v1 *api) {
  if (api == NULL ||
      api->struct_size < NEXILB_API_V1_CHECKPOINT_SLOTS_SIZE ||
      api->checkpoint_info == NULL || api->checkpoint_write == NULL ||
      api->checkpoint_read == NULL)
    return NEXILB_FALSE;
  return NEXILB_TRUE;
}

/**
 * Backward-compatible name for the config-path execution slot gate.
 * @param api Effective API table returned by successful negotiation.
 * @return Result of nexilb_case_config_path_slots_available().
 */
static inline nexilb_bool_t
nexilb_case_common_slots_available(const nexilb_api_v1 *api) {
  return nexilb_case_config_path_slots_available(api);
}

/**
 * Check the composite full IMB/DEM evidence slot set.
 * @param api Effective API table returned by successful negotiation.
 * @return NEXILB_TRUE only when every composite evidence slot exists.
 * @note This is not the gate for a basic config-path run and does not by
 * itself establish catalog capabilities or physical acceptance.
 */
static inline nexilb_bool_t
nexilb_case_coupled_slots_available(const nexilb_api_v1 *api) {
  if (nexilb_case_config_path_slots_available(api) != NEXILB_TRUE ||
      nexilb_case_particle_input_slots_available(api) != NEXILB_TRUE ||
      nexilb_case_validation_slots_available(api) != NEXILB_TRUE ||
      nexilb_case_snapshot_particle_slots_available(api) != NEXILB_TRUE ||
      nexilb_case_snapshot_contact_slots_available(api) != NEXILB_TRUE ||
      nexilb_case_checkpoint_slots_available(api) != NEXILB_TRUE) {
    return NEXILB_FALSE;
  }
  return NEXILB_TRUE;
}

#ifdef __cplusplus
}
#endif
#endif
