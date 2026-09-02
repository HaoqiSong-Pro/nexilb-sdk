/**
 * @file nexilb.h
 * @brief Stable 64-bit C ABI contract for NexiLB discovery and runtime access.
 *
 * @defgroup nexilb_c_api NexiLB C API
 * @brief Versioned bootstrap, catalog, model, input, snapshot, checkpoint, and
 * error contracts exposed through a size-negotiated function table.
 *
 * This header defines an ABI contract and does not itself assert runtime
 * availability.  Callers must inspect `nexilb_library_info_t.api_flags`, the
 * effective catalog and the required model/capability descriptors, and test
 * every function-table slot for NULL before calling it.  A particular runtime
 * may expose only a subset of the table; a NULL slot is unavailable even when
 * another model or capability in the same runtime is usable.
 *
 * Unless an effective descriptor explicitly declares an operation thread-safe,
 * the caller serializes calls on the same handle.  Reset and destroy require
 * exclusive access; after either destroy returns, no thread may use the old
 * handle value.  Concurrency of different handles must also be declared and
 * must not be inferred from their distinct identities.
 * The first runtime profile declares `same_context_active_models == 1`: a
 * context may own several model handles, but only one may be READY/FAILED with
 * allocated solver state. Initialize or restore another only after resetting
 * or destroying the active model; otherwise the call returns
 * `NEXILB_STATUS_BUSY`.
 *
 * An output error-handle pointer, where accepted, must point to a NULL handle
 * on entry.  A non-NULL error returned by the runtime is caller-owned and must
 * be released only through `nexilb_api_v1.error_destroy`.
 *
 * Text and JSON outputs use `nexilb_buffer_t` as a two-call protocol.  First
 * pass `data == NULL` and `capacity_bytes == 0`; success reports the required
 * size, including the trailing NUL, in `required_bytes`. Allocate at least
 * that many bytes and call again.  A smaller second buffer returns
 * `NEXILB_STATUS_BUFFER_TOO_SMALL`.
 *
 * @{ 
 */

#ifndef NEXILB_NEXILB_H
#define NEXILB_NEXILB_H

#include <stddef.h>
#include <stdint.h>

#if UINTPTR_MAX != UINT64_MAX
#error "NexiLB ABI v1 requires a 64-bit process."
#endif

/**
 * @def NEXILB_CALL
 * @brief Platform calling-convention annotation for every ABI entry point.
 */
/**
 * @def NEXILB_PUBLIC
 * @brief Platform import/export and default-visibility annotation.
 */
#if defined(_WIN32)
#define NEXILB_CALL __cdecl
#if defined(NEXILB_BUILDING_RUNTIME)
#define NEXILB_PUBLIC __declspec(dllexport)
#else
#define NEXILB_PUBLIC __declspec(dllimport)
#endif
#else
#define NEXILB_CALL
#define NEXILB_PUBLIC __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Binary-incompatible NexiLB ABI generation. */
#define NEXILB_ABI_MAJOR 1u
/** Backward-compatible ABI revision within NEXILB_ABI_MAJOR. */
#define NEXILB_ABI_MINOR 0u
/** Version number of the nexilb_api_v1 function table. */
#define NEXILB_API_V1 1u
/** Initial version tag for all public size-versioned structures. */
#define NEXILB_STRUCT_VERSION_1 1u

/** Signed stable status-code representation. */
typedef int32_t nexilb_status_t;
/** ABI-stable Boolean representation; use NEXILB_FALSE or NEXILB_TRUE. */
typedef uint32_t nexilb_bool_t;
/** ABI-stable catalog entity-kind representation. */
typedef uint32_t nexilb_catalog_kind_t;
/** ABI-stable scalar element-type representation. */
typedef uint32_t nexilb_scalar_type_t;
/** ABI-stable array-association representation. */
typedef uint32_t nexilb_association_t;
/** ABI-stable model lifecycle-state representation. */
typedef uint32_t nexilb_model_state_t;
/** ABI-stable synchronous-run stop-reason representation. */
typedef uint32_t nexilb_stop_reason_t;

/** False value for nexilb_bool_t. */
#define NEXILB_FALSE ((nexilb_bool_t)0u)
/** True value for nexilb_bool_t. */
#define NEXILB_TRUE ((nexilb_bool_t)1u)

/** Operation completed successfully. */
#define NEXILB_STATUS_OK ((nexilb_status_t)0)
/** A required argument, value, pointer, or handle is invalid. */
#define NEXILB_STATUS_INVALID_ARGUMENT ((nexilb_status_t)-1)
/** A size-versioned structure is smaller than its mandatory prefix. */
#define NEXILB_STATUS_STRUCT_TOO_SMALL ((nexilb_status_t)-2)
/** The requested ABI, API, structure, or format version is unsupported. */
#define NEXILB_STATUS_UNSUPPORTED_VERSION ((nexilb_status_t)-3)
/** The operation is not legal in the current lifecycle state. */
#define NEXILB_STATUS_INVALID_STATE ((nexilb_status_t)-4)
/** The selected runtime or model does not expose the required capability. */
#define NEXILB_STATUS_UNSUPPORTED_CAPABILITY ((nexilb_status_t)-5)
/** JSON or another schema-governed payload is invalid. */
#define NEXILB_STATUS_SCHEMA_ERROR ((nexilb_status_t)-6)
/** Model input or geometry failed validation. */
#define NEXILB_STATUS_VALIDATION_ERROR ((nexilb_status_t)-7)
/** A caller-owned output buffer is smaller than required_bytes. */
#define NEXILB_STATUS_BUFFER_TOO_SMALL ((nexilb_status_t)-8)
/** A snapshot is no longer readable under its declared lifetime contract. */
#define NEXILB_STATUS_STALE_SNAPSHOT ((nexilb_status_t)-9)
/** A declared memory, size, count, or execution resource limit was exceeded. */
#define NEXILB_STATUS_RESOURCE_LIMIT ((nexilb_status_t)-10)
/** A filesystem or stream operation failed. */
#define NEXILB_STATUS_IO_ERROR ((nexilb_status_t)-11)
/** A required external runtime dependency failed or is incompatible. */
#define NEXILB_STATUS_DEPENDENCY_ERROR ((nexilb_status_t)-12)
/** Device discovery, setup, or execution failed. */
#define NEXILB_STATUS_DEVICE_ERROR ((nexilb_status_t)-13)
/** Solver execution failed a numerical-safety condition. */
#define NEXILB_STATUS_NUMERICAL_ERROR ((nexilb_status_t)-14)
/** The requested resource or exclusive lifecycle operation is busy. */
#define NEXILB_STATUS_BUSY ((nexilb_status_t)-15)
/** The negotiated table slot exists but its operation is not implemented. */
#define NEXILB_STATUS_NOT_IMPLEMENTED ((nexilb_status_t)-16)
/** The requested identifier, index, file, or entity was not found. */
#define NEXILB_STATUS_NOT_FOUND ((nexilb_status_t)-17)
/** An internal invariant failed without a more specific public status. */
#define NEXILB_STATUS_INTERNAL_ERROR ((nexilb_status_t)-127)

/** Model exists without accepted configuration or allocated solver state. */
#define NEXILB_MODEL_STATE_EMPTY ((nexilb_model_state_t)0u)
/** Complete model configuration has been accepted. */
#define NEXILB_MODEL_STATE_CONFIGURED ((nexilb_model_state_t)1u)
/** An atomic input transaction is currently open. */
#define NEXILB_MODEL_STATE_INPUT_OPEN ((nexilb_model_state_t)2u)
/** Configuration and staged input passed validation. */
#define NEXILB_MODEL_STATE_VALIDATED ((nexilb_model_state_t)3u)
/** Solver state is initialized and may be advanced. */
#define NEXILB_MODEL_STATE_READY ((nexilb_model_state_t)4u)
/** The model reached its declared terminal condition. */
#define NEXILB_MODEL_STATE_FINISHED ((nexilb_model_state_t)5u)
/** Model execution failed and requires reset or destruction. */
#define NEXILB_MODEL_STATE_FAILED ((nexilb_model_state_t)6u)
/** Handle state after release; old handle values must not be reused. */
#define NEXILB_MODEL_STATE_RELEASED ((nexilb_model_state_t)7u)

/** No synchronous-run stop reason has been reported. */
#define NEXILB_STOP_REASON_NONE ((nexilb_stop_reason_t)0u)
/** The exact requested target was reached. */
#define NEXILB_STOP_REASON_TARGET_REACHED ((nexilb_stop_reason_t)1u)
/** The caller-provided maximum step bound was reached first. */
#define NEXILB_STOP_REASON_MAX_STEPS_REACHED ((nexilb_stop_reason_t)2u)
/** The model reached its declared terminal state. */
#define NEXILB_STOP_REASON_MODEL_FINISHED ((nexilb_stop_reason_t)3u)
/** Execution stopped through an effective cancellation mechanism. */
#define NEXILB_STOP_REASON_CANCELLED ((nexilb_stop_reason_t)4u)
/** Execution stopped because a stable error status was produced. */
#define NEXILB_STOP_REASON_ERROR ((nexilb_stop_reason_t)5u)

/** Validation severity was not specified. */
#define NEXILB_VALIDATION_SEVERITY_UNSPECIFIED INT32_C(0)
/** Informational validation item. */
#define NEXILB_VALIDATION_SEVERITY_INFO INT32_C(1)
/** Non-fatal validation warning. */
#define NEXILB_VALIDATION_SEVERITY_WARNING INT32_C(2)
/** Validation error that prevents the requested transition. */
#define NEXILB_VALIDATION_SEVERITY_ERROR INT32_C(3)
/** Fatal validation condition for the requested model operation. */
#define NEXILB_VALIDATION_SEVERITY_FATAL INT32_C(4)

/* ABI v1 defines only the all-zero values for these option flag groups. */
/** No context option flags; the only context flag value defined by ABI v1. */
#define NEXILB_CONTEXT_FLAGS_NONE UINT64_C(0)
/** No array-view flags; the only array flag value defined by ABI v1. */
#define NEXILB_ARRAY_FLAGS_NONE UINT64_C(0)
/** No particle-batch flags; the only batch flag value defined by ABI v1. */
#define NEXILB_PARTICLE_BATCH_FLAGS_NONE UINT64_C(0)
/** No synchronous-run flags; the only run flag value defined by ABI v1. */
#define NEXILB_RUN_FLAGS_NONE UINT64_C(0)

/** Catalog kind for creatable or descriptive model entries. */
#define NEXILB_CATALOG_MODEL ((nexilb_catalog_kind_t)1u)
/** Catalog kind for physical or numerical representation entries. */
#define NEXILB_CATALOG_REPRESENTATION ((nexilb_catalog_kind_t)2u)
/** Catalog kind for public field entries. */
#define NEXILB_CATALOG_FIELD ((nexilb_catalog_kind_t)3u)
/** Catalog kind for public boundary-condition entries. */
#define NEXILB_CATALOG_BOUNDARY ((nexilb_catalog_kind_t)4u)
/** Catalog kind for phase-relation entries. */
#define NEXILB_CATALOG_PHASE_RELATION ((nexilb_catalog_kind_t)5u)
/** Catalog kind for supported particle-shape entries. */
#define NEXILB_CATALOG_PARTICLE_SHAPE ((nexilb_catalog_kind_t)6u)
/** Catalog kind for public particle-attribute entries. */
#define NEXILB_CATALOG_PARTICLE_ATTRIBUTE ((nexilb_catalog_kind_t)7u)
/** Catalog kind for material-model entries. */
#define NEXILB_CATALOG_MATERIAL ((nexilb_catalog_kind_t)8u)
/** Catalog kind for contact-law entries. */
#define NEXILB_CATALOG_CONTACT_LAW ((nexilb_catalog_kind_t)9u)
/** Catalog kind for time-integration entries. */
#define NEXILB_CATALOG_INTEGRATOR ((nexilb_catalog_kind_t)10u)
/** Catalog kind for multiphysics coupling-scheme entries. */
#define NEXILB_CATALOG_COUPLING_SCHEME ((nexilb_catalog_kind_t)11u)
/** Catalog kind for diagnostic-array entries. */
#define NEXILB_CATALOG_DIAGNOSTIC ((nexilb_catalog_kind_t)12u)
/** Catalog kind for output-format or output-channel entries. */
#define NEXILB_CATALOG_OUTPUT ((nexilb_catalog_kind_t)13u)
/** Catalog kind for model-chain entries. */
#define NEXILB_CATALOG_MODEL_CHAIN ((nexilb_catalog_kind_t)14u)
/** Catalog kind for checkpoint-format entries. */
#define NEXILB_CATALOG_CHECKPOINT_FORMAT ((nexilb_catalog_kind_t)15u)
/** Catalog kind for determinism-profile entries. */
#define NEXILB_CATALOG_DETERMINISM_PROFILE ((nexilb_catalog_kind_t)16u)
/** Catalog kind for packaged secondary-development CASE entries. */
#define NEXILB_CATALOG_CASE ((nexilb_catalog_kind_t)17u)

/** Unsigned 8-bit scalar element type. */
#define NEXILB_SCALAR_U8 ((nexilb_scalar_type_t)1u)
/** Signed 32-bit scalar element type. */
#define NEXILB_SCALAR_I32 ((nexilb_scalar_type_t)2u)
/** Unsigned 32-bit scalar element type. */
#define NEXILB_SCALAR_U32 ((nexilb_scalar_type_t)3u)
/** Signed 64-bit scalar element type. */
#define NEXILB_SCALAR_I64 ((nexilb_scalar_type_t)4u)
/** Unsigned 64-bit scalar element type. */
#define NEXILB_SCALAR_U64 ((nexilb_scalar_type_t)5u)
/** IEEE-754 binary32 scalar element type. */
#define NEXILB_SCALAR_F32 ((nexilb_scalar_type_t)6u)
/** IEEE-754 binary64 scalar element type. */
#define NEXILB_SCALAR_F64 ((nexilb_scalar_type_t)7u)

/** Array values are associated with mesh nodes. */
#define NEXILB_ASSOCIATION_NODE ((nexilb_association_t)1u)
/** Array values are associated with mesh cells. */
#define NEXILB_ASSOCIATION_CELL ((nexilb_association_t)2u)
/** Array values are associated with particles. */
#define NEXILB_ASSOCIATION_PARTICLE ((nexilb_association_t)3u)
/** Array values are associated with contacts. */
#define NEXILB_ASSOCIATION_CONTACT ((nexilb_association_t)4u)

/** Runtime identifies itself as a non-release contract prototype. */
#define NEXILB_API_FLAG_CONTRACT_PROTOTYPE UINT64_C(0x0000000000000001)
/** Runtime provides an immutable library-level catalog. */
#define NEXILB_API_FLAG_LIBRARY_CATALOG_AVAILABLE UINT64_C(0x0000000000000002)
/** Runtime can return owned machine-readable error objects. */
#define NEXILB_API_FLAG_ERROR_OBJECTS_AVAILABLE UINT64_C(0x0000000000000004)
/** Runtime provides device enumeration and descriptors. */
#define NEXILB_API_FLAG_DEVICE_ENUMERATION_AVAILABLE UINT64_C(0x0000000000000008)

/** Error originated in ABI negotiation or general API contract handling. */
#define NEXILB_ERROR_SUBSYSTEM_API UINT32_C(1)
/** Error originated in catalog lookup or descriptor handling. */
#define NEXILB_ERROR_SUBSYSTEM_CATALOG UINT32_C(2)
/** Error originated in device discovery or device selection. */
#define NEXILB_ERROR_SUBSYSTEM_DEVICE UINT32_C(3)

/** Incomplete backing type for an opaque execution context. */
typedef struct nexilb_context nexilb_context;
/** Incomplete backing type for an opaque model. */
typedef struct nexilb_model nexilb_model;
/** Incomplete backing type for an opaque immutable catalog. */
typedef struct nexilb_catalog nexilb_catalog;
/** Incomplete backing type for an opaque input transaction. */
typedef struct nexilb_input nexilb_input;
/** Incomplete backing type for an opaque immutable snapshot. */
typedef struct nexilb_snapshot nexilb_snapshot;
/** Incomplete backing type for an opaque validation report. */
typedef struct nexilb_validation_report nexilb_validation_report;
/** Incomplete backing type for an opaque owned error object. */
typedef struct nexilb_error nexilb_error;

/** Opaque execution-context handle; destroy it after all child handles. */
typedef nexilb_context *nexilb_context_t;
/** Opaque model handle owned by its context. */
typedef nexilb_model *nexilb_model_t;
/** Opaque immutable catalog handle. */
typedef nexilb_catalog *nexilb_catalog_t;
/** Opaque input transaction finished by commit or abort. */
typedef nexilb_input *nexilb_input_t;
/** Opaque immutable snapshot handle. */
typedef nexilb_snapshot *nexilb_snapshot_t;
/** Opaque validation-report handle. */
typedef nexilb_validation_report *nexilb_validation_report_t;
/** Opaque owned error handle; never release it with the C allocator. */
typedef nexilb_error *nexilb_error_t;

/** Non-owning UTF-8 byte view; the bytes need not be NUL-terminated. */
typedef struct nexilb_string_view_t {
  const char *data;    /**< Borrowed byte range. */
  uint64_t byte_size;  /**< Bytes at `data`, excluding any terminator. */
} nexilb_string_view_t;

/** Caller-owned destination implementing the documented two-call protocol. */
typedef struct nexilb_buffer_t {
  uint32_t struct_size;     /**< Set to `sizeof(nexilb_buffer_t)`. */
  uint32_t struct_version;  /**< Set to `NEXILB_STRUCT_VERSION_1`. */
  void *data;               /**< Writable bytes, or NULL for a size query. */
  uint64_t capacity_bytes;  /**< Writable byte capacity at `data`. */
  uint64_t required_bytes;  /**< Required capacity including trailing NUL. */
} nexilb_buffer_t;

/** ABI generation and supported API-table interval. */
typedef struct nexilb_abi_version_t {
  uint32_t abi_major;  /**< Binary-incompatible ABI generation. */
  uint32_t abi_minor;  /**< Backward-compatible ABI revision. */
  uint32_t api_min;    /**< Minimum accepted function-table version. */
  uint32_t api_max;    /**< Maximum accepted function-table version. */
} nexilb_abi_version_t;

/** Runtime identity and explicit capability flags. */
typedef struct nexilb_library_info_t {
  uint32_t struct_size;     /**< Caller-provided writable structure size. */
  uint32_t struct_version;  /**< Structure contract version. */
  uint32_t sdk_major;       /**< SDK semantic-version major. */
  uint32_t sdk_minor;       /**< SDK semantic-version minor. */
  uint32_t sdk_patch;       /**< SDK semantic-version patch. */
  uint32_t abi_major;       /**< Runtime ABI major. */
  uint32_t abi_minor;       /**< Runtime ABI minor. */
  uint32_t api_min;         /**< Minimum supported table version. */
  uint32_t api_max;         /**< Maximum supported table version. */
  uint32_t reserved_u32;    /**< Reserved; callers initialize to zero. */
  uint64_t api_flags;       /**< Bitwise OR of `NEXILB_API_FLAG_*`. */
  char build_id[64];        /**< NUL-terminated public build identifier. */
} nexilb_library_info_t;

/** Context device selection and hard memory budgets. */
typedef struct nexilb_context_options_t {
  uint32_t struct_size;                 /**< Set to `sizeof(nexilb_context_options_t)`. */
  uint32_t struct_version;              /**< Set to `NEXILB_STRUCT_VERSION_1`. */
  int32_t device_ordinal;               /**< Zero-based device ordinal selected by the caller. */
  uint32_t reserved_u32;                /**< Reserved; initialize to zero. */
  uint64_t host_memory_budget_bytes;    /**< Host-memory budget, or zero for runtime policy. */
  uint64_t device_memory_budget_bytes;  /**< Device-memory budget, or zero for runtime policy. */
  uint64_t snapshot_budget_bytes;       /**< Immutable-snapshot budget, or zero for runtime policy. */
  uint64_t flags;                       /**< Bitwise OR of `NEXILB_CONTEXT_FLAG_*`. */
} nexilb_context_options_t;

/**
 * Immutable catalog generation, schema version, and content digest.
 *
 * Effective-catalog digest v1 is independently reproducible. Iterate catalog
 * kind `NEXILB_CATALOG_MODEL_CHAIN` first and `NEXILB_CATALOG_MODEL` second;
 * within each kind use `catalog_id` index order from zero through count minus
 * one. For each ID append the exact UTF-8 bytes returned by
 * `catalog_descriptor_json`, excluding its trailing NUL, followed by one LF
 * byte (`0x0a`). SHA-256 the resulting byte stream. Do not parse, normalize,
 * reindent, reorder, or reserialize descriptor JSON.
 */
typedef struct nexilb_catalog_info_t {
  uint32_t struct_size;              /**< Caller-provided writable structure size. */
  uint32_t struct_version;           /**< Structure contract version. */
  uint64_t generation;               /**< Catalog generation bound to returned descriptors. */
  uint8_t catalog_sha256[32];        /**< Effective catalog SHA-256 digest bytes. */
  uint32_t descriptor_schema_major;  /**< Descriptor-schema incompatible generation. */
  uint32_t descriptor_schema_minor;  /**< Descriptor-schema compatible revision. */
  uint64_t flags;                    /**< Versioned catalog flags. */
} nexilb_catalog_info_t;

/** Integer synchronization identity and lifecycle state of a model. */
typedef struct nexilb_model_state_info_t {
  uint32_t struct_size;         /**< Caller-provided writable structure size. */
  uint32_t struct_version;      /**< Structure contract version. */
  nexilb_model_state_t state;   /**< Current model lifecycle state. */
  uint32_t reserved_u32;        /**< Reserved; zero in ABI v1. */
  uint64_t generation;          /**< Model generation invalidating older borrowed state. */
  uint64_t macro_step;          /**< Most recently completed macro step. */
  uint64_t exact_tick;          /**< Exact integer synchronization tick. */
} nexilb_model_state_info_t;

/** Rank-bounded logical subregion used for bulk field transfer. */
typedef struct nexilb_region_t {
  uint32_t struct_size;     /**< Set to `sizeof(nexilb_region_t)`. */
  uint32_t struct_version;  /**< Set to `NEXILB_STRUCT_VERSION_1`. */
  uint32_t rank;            /**< Active entries in `begin` and `extent`. */
  uint32_t reserved_u32;    /**< Reserved; initialize to zero. */
  uint64_t begin[4];        /**< Logical starting index by documented axis. */
  uint64_t extent[4];       /**< Logical element count by documented axis. */
} nexilb_region_t;

/** Explicitly typed, shaped, and strided caller memory view. */
typedef struct nexilb_array_view_t {
  uint32_t struct_size;                 /**< Caller-provided structure size. */
  uint32_t struct_version;              /**< Structure contract version. */
  void *data;                           /**< Borrowed payload address. */
  uint64_t element_count;               /**< Logical scalar element count. */
  uint64_t capacity_bytes;              /**< Accessible payload bytes. */
  nexilb_scalar_type_t scalar_type;     /**< One `NEXILB_SCALAR_*` value. */
  uint32_t component_count;             /**< Components per logical item. */
  uint32_t rank;                        /**< Active entries in extent/stride. */
  nexilb_association_t association;     /**< Node, cell, particle, or contact. */
  uint64_t logical_extent[4];           /**< Logical extent by documented axis. */
  int64_t byte_stride[4];               /**< Byte stride for each logical axis. */
  uint64_t flags;                       /**< Versioned view flags. */
} nexilb_array_view_t;

/**
 * Snapshot array reads use a two-call protocol. On the query call, initialize
 * `struct_size` and `struct_version` and leave `data == NULL` and
 * `capacity_bytes == 0`; success returns the required byte count in
 * `capacity_bytes` plus scalar type, element count, component count,
 * association, rank, logical extents and byte strides. Allocate that many
 * bytes, restore `data` and `capacity_bytes`, and call the same read slot
 * again. Returned component tuples are item-major contiguous. A smaller
 * payload buffer returns `NEXILB_STATUS_BUFFER_TOO_SMALL` without a partial
 * copy. The snapshot owns its copied bytes; model advancement cannot change
 * them.
 */

/** Stable attribute ID paired with one array view. */
typedef struct nexilb_named_array_view_t {
  uint32_t struct_size;                /**< Set to `sizeof(nexilb_named_array_view_t)`. */
  uint32_t struct_version;             /**< Set to `NEXILB_STRUCT_VERSION_1`. */
  nexilb_string_view_t attribute_id;   /**< Borrowed stable catalog attribute ID. */
  nexilb_array_view_t values;          /**< Borrowed values for the named attribute. */
} nexilb_named_array_view_t;

/** Columnar particle input batch submitted atomically within a transaction. */
typedef struct nexilb_particle_batch_t {
  uint32_t struct_size;                         /**< Structure size. */
  uint32_t struct_version;                      /**< Structure version. */
  uint64_t row_count;                           /**< Particle rows in every column. */
  nexilb_string_view_t string_table;            /**< Borrowed UTF-8 table bytes. */
  nexilb_array_view_t string_offsets;           /**< Offsets into `string_table`. */
  nexilb_array_view_t body_modes;               /**< Per-row body-mode values. */
  const nexilb_named_array_view_t *columns;      /**< Borrowed descriptor columns. */
  uint64_t column_count;                        /**< Number of `columns`. */
  uint64_t flags;                               /**< Versioned batch flags. */
} nexilb_particle_batch_t;

/** Integer target and hard work limit for synchronous `model_run_until`. */
typedef struct nexilb_run_options_t {
  uint32_t struct_size;        /**< Structure size. */
  uint32_t struct_version;     /**< Structure version. */
  uint64_t target_macro_step;  /**< Requested macro-step identity. */
  uint64_t target_exact_tick;  /**< Requested exact integer tick. */
  uint64_t max_steps;          /**< Hard maximum advances for this call. */
  uint64_t flags;              /**< Versioned run flags. */
} nexilb_run_options_t;

/** Synchronization identity and stop reason returned after advancement. */
typedef struct nexilb_step_result_t {
  uint32_t struct_size;                  /**< Structure size. */
  uint32_t struct_version;               /**< Structure version. */
  uint64_t macro_step;                   /**< Completed macro step. */
  uint64_t completed_dem_substep;        /**< Coupled chain's latest completed cumulative DEM synchronization substep, aligned with `exact_tick`; zero for a non-DEM chain. */
  uint64_t exact_tick;                   /**< Exact integer synchronization tick. */
  double display_time;                   /**< Display-only physical time. */
  nexilb_stop_reason_t stop_reason;      /**< Versioned stop-reason value. */
  uint32_t reserved_u32;                 /**< Reserved; zero in ABI v1. */
} nexilb_step_result_t;

/** Snapshot identity and effective input/catalog digests. */
typedef struct nexilb_snapshot_info_t {
  uint32_t struct_size;                    /**< Caller-provided writable structure size. */
  uint32_t struct_version;                 /**< Structure contract version. */
  uint64_t model_generation;               /**< Model generation captured by this snapshot. */
  uint64_t macro_step;                     /**< Captured completed macro step. */
  uint64_t exact_tick;                     /**< Captured exact integer tick. */
  double display_time;                     /**< Display-only physical time. */
  uint8_t effective_config_sha256[32];     /**< Effective configuration SHA-256 bytes. */
  uint8_t effective_catalog_sha256[32];    /**< Effective catalog SHA-256 bytes. */
} nexilb_snapshot_info_t;

/** Bounded checkpoint metadata readable before model creation. */
typedef struct nexilb_checkpoint_info_t {
  uint32_t struct_size;                        /**< Caller-provided writable structure size. */
  uint32_t struct_version;                     /**< Structure contract version. */
  uint32_t format_major;                       /**< Binary-incompatible checkpoint format. */
  uint32_t format_minor;                       /**< Backward-compatible checkpoint revision. */
  uint32_t dimension;                          /**< Spatial dimension encoded by the checkpoint. */
  nexilb_scalar_type_t precision;              /**< Stored scalar precision. */
  uint64_t macro_step;                         /**< Completed macro step at the checkpoint cut. */
  uint64_t exact_tick;                         /**< Exact integer tick at the checkpoint cut. */
  uint8_t effective_config_sha256[32];         /**< Effective configuration SHA-256 bytes. */
  uint8_t schema_sha256[32];                   /**< Bound public schema SHA-256 bytes. */
  uint8_t required_capability_sha256[32];      /**< Required-capability-set SHA-256 bytes. */
  uint8_t producer_environment_sha256[32];     /**< Producer-environment identity SHA-256 bytes. */
  char model_id[128];                          /**< NUL-terminated exact model ID. */
} nexilb_checkpoint_info_t;

/** Stable machine-readable validation diagnostic. */
typedef struct nexilb_validation_item_t {
  uint32_t struct_size;         /**< Caller-provided writable structure size. */
  uint32_t struct_version;      /**< Structure contract version. */
  int32_t severity;             /**< Stable validation severity value. */
  int32_t code;                 /**< Stable machine-readable validation code. */
  uint64_t model_generation;    /**< Model generation to which this item applies. */
  char diagnostic_id[96];       /**< NUL-terminated stable diagnostic ID. */
  char entity_id[128];          /**< NUL-terminated related entity ID, when applicable. */
  char json_pointer[256];       /**< NUL-terminated JSON Pointer, when applicable. */
} nexilb_validation_item_t;

/** Machine-readable view of an owned error handle. */
typedef struct nexilb_error_info_t {
  uint32_t struct_size;       /**< Caller-provided structure size. */
  uint32_t struct_version;    /**< Structure contract version. */
  nexilb_status_t status;     /**< Stable `NEXILB_STATUS_*` value. */
  uint32_t subsystem;         /**< Stable `NEXILB_ERROR_SUBSYSTEM_*` value. */
  char entity_id[128];        /**< NUL-terminated stable entity ID if applicable. */
  char json_pointer[256];     /**< NUL-terminated JSON Pointer if applicable. */
} nexilb_error_info_t;

/* Frozen v1 prefixes let a newer runtime append tail fields safely. */
/** Required byte size of a version-1 caller-owned buffer structure. */
#define NEXILB_BUFFER_V1_MANDATORY_SIZE                                       \
  ((uint64_t)sizeof(nexilb_buffer_t))
/** Required byte prefix of version-1 runtime library information. */
#define NEXILB_LIBRARY_INFO_V1_MANDATORY_SIZE                                 \
  ((uint64_t)offsetof(nexilb_library_info_t, build_id))
/** Required byte prefix of version-1 catalog information. */
#define NEXILB_CATALOG_INFO_V1_MANDATORY_SIZE                                 \
  ((uint64_t)offsetof(nexilb_catalog_info_t, flags))
/** Required byte prefix of version-1 machine-readable error information. */
#define NEXILB_ERROR_INFO_V1_MANDATORY_SIZE                                   \
  ((uint64_t)offsetof(nexilb_error_info_t, entity_id))

/**
 * Format a stable status identifier through the two-call buffer protocol.
 * @param status Status value to name.
 * @param[out] out_name Caller-owned destination buffer.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_status_name_fn)(
    nexilb_status_t status,
    nexilb_buffer_t *out_name);

/**
 * Read runtime identity and top-level capability flags.
 * @param[out] out_info Size/version-initialized library information.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_library_info_fn)(
    nexilb_library_info_t *out_info,
    nexilb_error_t *out_error);

/**
 * Create the immutable library-level catalog.
 * @param[out] out_catalog Receives the owned catalog handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_library_catalog_create_fn)(
    nexilb_catalog_t *out_catalog,
    nexilb_error_t *out_error);

/**
 * Create the effective catalog for an execution context.
 * @param context Context whose effective capabilities are queried.
 * @param[out] out_catalog Receives the owned catalog handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_context_catalog_create_fn)(
    nexilb_context_t context,
    nexilb_catalog_t *out_catalog,
    nexilb_error_t *out_error);

/**
 * Read catalog identity, counts, and canonical digest metadata.
 * @param catalog Catalog handle to inspect.
 * @param[out] out_info Size/version-initialized catalog information.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_catalog_info_fn)(
    nexilb_catalog_t catalog,
    nexilb_catalog_info_t *out_info,
    nexilb_error_t *out_error);

/**
 * Count catalog entities of one stable kind.
 * @param catalog Catalog handle to inspect.
 * @param kind Stable NEXILB_CATALOG_* entity kind.
 * @param[out] out_count Receives the number of entities of that kind.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_catalog_count_fn)(
    nexilb_catalog_t catalog,
    nexilb_catalog_kind_t kind,
    uint64_t *out_count,
    nexilb_error_t *out_error);

/**
 * Read a stable catalog entity identifier by kind and zero-based index.
 * @param catalog Catalog handle to inspect.
 * @param kind Stable NEXILB_CATALOG_* entity kind.
 * @param index Zero-based index less than the corresponding entity count.
 * @param[out] out_id Caller-owned two-call destination buffer.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_catalog_id_fn)(
    nexilb_catalog_t catalog,
    nexilb_catalog_kind_t kind,
    uint64_t index,
    nexilb_buffer_t *out_id,
    nexilb_error_t *out_error);

/**
 * Read descriptor or schema JSON for an exact catalog entity identifier.
 * @param catalog Catalog handle to inspect.
 * @param entity_id Exact UTF-8 catalog entity identifier.
 * @param[out] out_json Caller-owned two-call destination buffer.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_catalog_json_fn)(
    nexilb_catalog_t catalog,
    nexilb_string_view_t entity_id,
    nexilb_buffer_t *out_json,
    nexilb_error_t *out_error);

/**
 * Destroy an owned catalog handle and set it to NULL.
 * @param[in,out] inout_catalog Catalog handle to release.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_catalog_destroy_fn)(
    nexilb_catalog_t *inout_catalog,
    nexilb_error_t *out_error);

/**
 * Count devices discoverable by this runtime.
 * @param[out] out_count Receives the device count.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_device_count_fn)(
    uint32_t *out_count,
    nexilb_error_t *out_error);

/**
 * Read the JSON descriptor for a zero-based device index.
 * @param index Zero-based index less than the discoverable device count.
 * @param[out] out_json Caller-owned two-call destination buffer.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_device_info_json_fn)(
    uint32_t index,
    nexilb_buffer_t *out_json,
    nexilb_error_t *out_error);

/**
 * Create an execution context from explicit size-versioned options.
 * @param options Context options, including the selected device and precision.
 * @param[out] out_context Receives the owned context handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_context_create_fn)(
    const nexilb_context_options_t *options,
    nexilb_context_t *out_context,
    nexilb_error_t *out_error);

/**
 * Read effective execution-context information as JSON.
 * @param context Context handle to inspect.
 * @param[out] out_json Caller-owned two-call destination buffer.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_context_info_json_fn)(
    nexilb_context_t context,
    nexilb_buffer_t *out_json,
    nexilb_error_t *out_error);

/**
 * Destroy an idle context and set its handle to NULL.
 * @param[in,out] inout_context Context handle without live child handles.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_context_destroy_fn)(
    nexilb_context_t *inout_context,
    nexilb_error_t *out_error);

/**
 * Create an EMPTY model for an exact effective-catalog model identifier.
 * @param context Owning execution context.
 * @param model_id Exact UTF-8 model identifier from the effective catalog.
 * @param[out] out_model Receives the owned model handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_create_fn)(
    nexilb_context_t context,
    nexilb_string_view_t model_id,
    nexilb_model_t *out_model,
    nexilb_error_t *out_error);

/**
 * Read the current model state and exact integer progress counters.
 * @param model Model handle to inspect.
 * @param[out] out_state Size/version-initialized state information.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_state_fn)(
    nexilb_model_t model,
    nexilb_model_state_info_t *out_state,
    nexilb_error_t *out_error);

/**
 * Reset a model to the documented EMPTY state.
 * @param model Model handle on which the caller holds exclusive access.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_reset_fn)(
    nexilb_model_t model,
    nexilb_error_t *out_error);

/**
 * Destroy a model and set its handle to NULL.
 * @param[in,out] inout_model Model handle on which the caller has exclusivity.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_destroy_fn)(
    nexilb_model_t *inout_model,
    nexilb_error_t *out_error);
/**
 * Set complete model configuration JSON.
 *
 * The config-path runtime profile accepts exactly one object member,
 * `config_path`, whose value is a normalized case-root-relative path. Common
 * JSON escapes are parsed, but `\\uXXXX` escapes are rejected; pass non-ASCII
 * path text as original UTF-8. The public request schema is
 * `urn:nexilb:schema:config-path-request:1.0`.
 * @param model Model handle in a state that accepts configuration.
 * @param config_json Complete UTF-8 configuration request JSON.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_set_config_json_fn)(
    nexilb_model_t model,
    nexilb_string_view_t config_json,
    nexilb_error_t *out_error);

/**
 * Begin an atomic input transaction for a configured model.
 * @param model Model that will own the transaction.
 * @param[out] out_input Receives the owned input transaction handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_input_begin_fn)(
    nexilb_model_t model,
    nexilb_input_t *out_input,
    nexilb_error_t *out_error);

/**
 * Stage one initial-field region in an open input transaction.
 * @param input Open input transaction.
 * @param field_id Exact effective-catalog field identifier.
 * @param region Logical region described by the values view.
 * @param values Borrowed field values staged for atomic commit.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_input_write_initial_field_fn)(
    nexilb_input_t input,
    nexilb_string_view_t field_id,
    const nexilb_region_t *region,
    const nexilb_array_view_t *values,
    nexilb_error_t *out_error);

/**
 * Stage a particle batch in an open input transaction.
 * @param input Open input transaction.
 * @param representation_id Exact effective-catalog representation identifier.
 * @param batch Borrowed particle attribute batch to stage.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_input_add_particles_fn)(
    nexilb_input_t input,
    nexilb_string_view_t representation_id,
    const nexilb_particle_batch_t *batch,
    nexilb_error_t *out_error);

/**
 * Commit or abort an input transaction and set its handle to NULL.
 * @param[in,out] inout_input Open input transaction to finish.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 * @note The function-table member selects commit or abort semantics.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_input_finish_fn)(
    nexilb_input_t *inout_input,
    nexilb_error_t *out_error);

/**
 * Create a validation or geometry report for a model.
 * @param model Model handle to validate or inspect.
 * @param[out] out_report Receives the owned immutable report handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 * @note The function-table member selects validation or geometry semantics.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_report_fn)(
    nexilb_model_t model,
    nexilb_validation_report_t *out_report,
    nexilb_error_t *out_error);

/**
 * Count items in an immutable validation report.
 * @param report Validation report handle to inspect.
 * @param[out] out_count Receives the number of report items.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_validation_report_count_fn)(
    nexilb_validation_report_t report,
    uint64_t *out_count);

/**
 * Read one validation item by zero-based index.
 * @param report Validation report handle to inspect.
 * @param index Zero-based index less than the report item count.
 * @param[out] out_item Size/version-initialized validation item.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_validation_report_get_fn)(
    nexilb_validation_report_t report,
    uint64_t index,
    nexilb_validation_item_t *out_item);

/**
 * Destroy a validation report and set its handle to NULL.
 * @param[in,out] inout_report Validation report handle to release.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_validation_report_destroy_fn)(
    nexilb_validation_report_t *inout_report);
/**
 * Evaluate a model query whose result is returned as JSON.
 * @param model Model handle to inspect.
 * @param[out] out_json Caller-owned two-call destination buffer.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 * @note The function-table member selects limits or resource-estimate output.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_json_fn)(
    nexilb_model_t model,
    nexilb_buffer_t *out_json,
    nexilb_error_t *out_error);

/**
 * Initialize a configured and accepted model for execution.
 * @param model Model handle to initialize.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_initialize_fn)(
    nexilb_model_t model,
    nexilb_error_t *out_error);

/**
 * Advance a READY model by one macro step.
 * @param model Model handle to advance.
 * @param[out] out_result Size/version-initialized exact step result.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_step_fn)(
    nexilb_model_t model,
    nexilb_step_result_t *out_result,
    nexilb_error_t *out_error);

/**
 * Synchronously advance a READY model to an exact integer target or stop.
 * @param model Model handle to advance.
 * @param options Size/version-initialized run target and bounds.
 * @param[out] out_result Size/version-initialized terminal step result.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_model_run_until_fn)(
    nexilb_model_t model,
    const nexilb_run_options_t *options,
    nexilb_step_result_t *out_result,
    nexilb_error_t *out_error);

/**
 * Capture an immutable snapshot of a model at its current exact tick.
 * @param model Model handle to snapshot.
 * @param[out] out_snapshot Receives the owned snapshot handle.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_snapshot_create_fn)(
    nexilb_model_t model,
    nexilb_snapshot_t *out_snapshot,
    nexilb_error_t *out_error);

/**
 * Read immutable snapshot identity and progress metadata.
 * @param snapshot Snapshot handle to inspect.
 * @param[out] out_info Size/version-initialized snapshot information.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_snapshot_info_fn)(
    nexilb_snapshot_t snapshot,
    nexilb_snapshot_info_t *out_info,
    nexilb_error_t *out_error);

/**
 * Read one named public array view from an immutable snapshot.
 * @param snapshot Snapshot handle to read.
 * @param array_id Exact field, particle, contact, or diagnostic identifier.
 * @param[in,out] inout_view Size/version-initialized destination array view.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 * @note The function-table member selects the array association namespace.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_snapshot_read_fn)(
    nexilb_snapshot_t snapshot,
    nexilb_string_view_t array_id,
    nexilb_array_view_t *inout_view,
    nexilb_error_t *out_error);

/**
 * Destroy a snapshot and set its handle to NULL.
 * @param[in,out] inout_snapshot Snapshot handle to release.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_snapshot_destroy_fn)(
    nexilb_snapshot_t *inout_snapshot,
    nexilb_error_t *out_error);

/**
 * Inspect bounded checkpoint metadata without restoring a model.
 * @param path UTF-8 checkpoint bundle path.
 * @param[out] out_info Size/version-initialized checkpoint information.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_checkpoint_info_fn)(
    nexilb_string_view_t path,
    nexilb_checkpoint_info_t *out_info,
    nexilb_error_t *out_error);

/**
 * Write or restore a model checkpoint bundle.
 * @param model Model handle to persist or EMPTY model handle to restore.
 * @param path UTF-8 checkpoint bundle path.
 * @param flags Operation flags; ABI v1 accepts only its documented zero value.
 * @param[out] out_error Error-handle destination; NULL on entry.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 * @note The function-table member selects checkpoint write or read semantics.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_checkpoint_model_fn)(
    nexilb_model_t model,
    nexilb_string_view_t path,
    uint64_t flags,
    nexilb_error_t *out_error);

/**
 * Read machine-readable fields from an owned error handle.
 * @param error Error handle to inspect.
 * @param[out] out_info Size/version-initialized error information.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_error_info_fn)(
    nexilb_error_t error,
    nexilb_error_info_t *out_info);

/**
 * Format an owned error handle through the two-call buffer protocol.
 * @param error Error handle to format.
 * @param[out] out_text Caller-owned destination buffer.
 * @return NEXILB_STATUS_OK on success; otherwise a stable status value.
 */
typedef nexilb_status_t(NEXILB_CALL *nexilb_error_format_fn)(
    nexilb_error_t error,
    nexilb_buffer_t *out_text);

/**
 * Destroy an owned error handle and set it to NULL.
 * @param[in,out] inout_error Error handle to release.
 */
typedef void(NEXILB_CALL *nexilb_error_destroy_fn)(
    nexilb_error_t *inout_error);

/**
 * Size-negotiated API v1 function table.
 *
 * `nexilb_get_api` zero-initializes the runtime table and copies no more than
 * the caller's `table_size`.  A NULL slot means that capability is unavailable
 * in that runtime and must not be called.  A non-NULL slot is still usable only
 * when the effective catalog accepts the exact model and required capability
 * set.  Successful bootstrap, a static descriptor, or availability of another
 * model does not enable a missing slot or a non-creatable model.
 */
typedef struct nexilb_api_v1 {
  uint32_t struct_size;     /**< Full table size known to the runtime. */
  uint32_t struct_version;  /**< Table structure version. */
  uint32_t api_version;     /**< Negotiated API version. */
  uint32_t reserved_u32;    /**< Reserved; zero in API v1. */
  uint64_t api_flags;       /**< Capability flags for this table. */

  nexilb_status_name_fn status_name; /**< Stable status-name formatting. */
  nexilb_library_info_fn library_info; /**< Runtime identity discovery. */

  nexilb_library_catalog_create_fn library_catalog_create; /**< Static catalog. */
  nexilb_context_catalog_create_fn context_catalog_create; /**< Effective catalog, nullable. */
  nexilb_catalog_info_fn catalog_info; /**< Catalog identity. */
  nexilb_catalog_count_fn catalog_count; /**< Entity count by kind. */
  nexilb_catalog_id_fn catalog_id; /**< Stable entity ID by index. */
  nexilb_catalog_json_fn catalog_descriptor_json; /**< Descriptor JSON. */
  nexilb_catalog_json_fn catalog_schema_json; /**< Descriptor schema JSON. */
  nexilb_catalog_destroy_fn catalog_destroy; /**< Release a catalog handle. */

  nexilb_device_count_fn device_count; /**< Discoverable device count. */
  nexilb_device_info_json_fn device_info_json; /**< Device descriptor JSON. */
  nexilb_context_create_fn context_create; /**< Context creation, nullable. */
  nexilb_context_info_json_fn context_info_json; /**< Effective context JSON. */
  nexilb_context_destroy_fn context_destroy; /**< Release an idle context. */

  nexilb_model_create_fn model_create; /**< Model creation, nullable. */
  nexilb_model_state_fn model_state; /**< Model state query. */
  nexilb_model_reset_fn model_reset; /**< Reset to the documented state. */
  nexilb_model_destroy_fn model_destroy; /**< Release a model handle. */
  nexilb_model_set_config_json_fn model_set_config_json; /**< Set validated JSON config. */

  nexilb_input_begin_fn input_begin; /**< Begin an atomic input transaction. */
  nexilb_input_write_initial_field_fn input_write_initial_field; /**< Write a field region. */
  nexilb_input_add_particles_fn input_add_particles; /**< Add a particle batch. */
  nexilb_input_finish_fn input_commit; /**< Atomically commit input. */
  nexilb_input_finish_fn input_abort; /**< Abort input without partial commit. */

  nexilb_model_report_fn model_validate; /**< Validate configuration and input. */
  nexilb_model_report_fn model_geometry_check; /**< Validate public geometry. */
  nexilb_validation_report_count_fn validation_report_count; /**< Report item count. */
  nexilb_validation_report_get_fn validation_report_get; /**< Read one report item. */
  nexilb_validation_report_destroy_fn validation_report_destroy; /**< Release report. */
  nexilb_model_json_fn model_limits_evaluate_json; /**< Evaluate hard limits. */
  nexilb_model_json_fn model_estimate_resources_json; /**< Estimate resources. */

  nexilb_model_initialize_fn model_initialize; /**< Initialize a validated model. */
  nexilb_model_step_fn model_step; /**< Advance one macro step. */
  nexilb_model_run_until_fn model_run_until; /**< Synchronously advance to an integer target. */

  nexilb_snapshot_create_fn snapshot_create; /**< Capture an immutable snapshot. */
  nexilb_snapshot_info_fn snapshot_info; /**< Read snapshot identity. */
  nexilb_snapshot_read_fn field_read; /**< Read a public field. */
  nexilb_snapshot_read_fn particle_read; /**< Read a particle attribute. */
  nexilb_snapshot_read_fn contact_read; /**< Read a contact attribute. */
  nexilb_snapshot_read_fn diagnostics_read; /**< Read a diagnostic array. */
  nexilb_snapshot_destroy_fn snapshot_destroy; /**< Release a snapshot. */

  nexilb_checkpoint_info_fn checkpoint_info; /**< Inspect bounded checkpoint metadata. */
  nexilb_checkpoint_model_fn checkpoint_write; /**< Persist model state. */
  nexilb_checkpoint_model_fn checkpoint_read; /**< Restore an EMPTY model; the bundle's exact model, dimension, precision and effective configuration digest must match. */

  nexilb_error_info_fn error_info; /**< Read machine-readable error fields. */
  nexilb_error_format_fn error_format; /**< Format error text via two-call buffer. */
  nexilb_error_destroy_fn error_destroy; /**< Release and NULL an owned error. */

  void *reserved[16]; /**< Reserved NULL slots for append-only evolution. */
} nexilb_api_v1;

/** Minimum API v1 table prefix required for successful negotiation. */
#define NEXILB_API_V1_MANDATORY_PREFIX_SIZE \
  ((uint64_t)offsetof(nexilb_api_v1, library_catalog_create))

/**
 * Query the runtime ABI and supported API interval.
 * @param[out] out_version Receives ABI/API values on success.
 * @return `NEXILB_STATUS_OK` or `NEXILB_STATUS_INVALID_ARGUMENT`.
 * @note This bootstrap call does not prove that any solver capability exists.
 */
NEXILB_PUBLIC nexilb_status_t NEXILB_CALL
nexilb_get_abi_version(nexilb_abi_version_t *out_version);

/**
 * Negotiate and copy API v1 without writing beyond `table_size` bytes.
 * @param[in] requested_api Requested table version; API v1 is
 * `NEXILB_API_V1`.
 * @param[in] table_size Writable bytes at `table`; an accepted historical
 * prefix is preserved without overflow.
 * @param[out] table Receives a zero-filled table prefix with supported slots.
 * @return `NEXILB_STATUS_OK`, `NEXILB_STATUS_INVALID_ARGUMENT`,
 * `NEXILB_STATUS_STRUCT_TOO_SMALL`, or `NEXILB_STATUS_UNSUPPORTED_VERSION`.
 * @warning A successful return does not make NULL slots callable and does not
 * override an unavailable/non-creatable effective-catalog entry.
 */
NEXILB_PUBLIC nexilb_status_t NEXILB_CALL
nexilb_get_api(uint32_t requested_api, uint64_t table_size, nexilb_api_v1 *table);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
