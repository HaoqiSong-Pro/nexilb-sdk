#ifndef NEXILB_BOOTSTRAP_FIXTURE_H
#define NEXILB_BOOTSTRAP_FIXTURE_H

#include <stddef.h>
#include <stdint.h>

#if UINTPTR_MAX != UINT64_MAX
#error "The frozen NexiLB bootstrap fixture requires a 64-bit process."
#endif

#if defined(_WIN32)
#define NEXILB_FIXTURE_CALL __cdecl
#define NEXILB_FIXTURE_PUBLIC __declspec(dllimport)
#else
#define NEXILB_FIXTURE_CALL
#define NEXILB_FIXTURE_PUBLIC
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NEXILB_FIXTURE_API_V1 1u
#define NEXILB_FIXTURE_STRUCT_VERSION_1 1u
#define NEXILB_FIXTURE_STATUS_OK ((int32_t)0)
#define NEXILB_FIXTURE_STATUS_INVALID_ARGUMENT ((int32_t)-1)
#define NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION ((int32_t)-3)
#define NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL ((int32_t)-2)
#define NEXILB_FIXTURE_STATUS_BUFFER_TOO_SMALL ((int32_t)-8)
#define NEXILB_FIXTURE_API_FLAG_CONTRACT_PROTOTYPE UINT64_C(1)

typedef struct nexilb_fixture_error *nexilb_fixture_error_t;

typedef struct nexilb_fixture_buffer_t {
  uint32_t struct_size;
  uint32_t struct_version;
  void *data;
  uint64_t capacity_bytes;
  uint64_t required_bytes;
} nexilb_fixture_buffer_t;

typedef struct nexilb_fixture_abi_version_t {
  uint32_t abi_major;
  uint32_t abi_minor;
  uint32_t api_min;
  uint32_t api_max;
} nexilb_fixture_abi_version_t;

typedef struct nexilb_fixture_library_info_t {
  uint32_t struct_size;
  uint32_t struct_version;
  uint32_t sdk_major;
  uint32_t sdk_minor;
  uint32_t sdk_patch;
  uint32_t abi_major;
  uint32_t abi_minor;
  uint32_t api_min;
  uint32_t api_max;
  uint32_t reserved_u32;
  uint64_t api_flags;
  char build_id[64];
} nexilb_fixture_library_info_t;

typedef int32_t(NEXILB_FIXTURE_CALL *nexilb_fixture_status_name_fn)(
    int32_t, nexilb_fixture_buffer_t *);
typedef int32_t(NEXILB_FIXTURE_CALL *nexilb_fixture_library_info_fn)(
    nexilb_fixture_library_info_t *, nexilb_fixture_error_t *);

typedef struct nexilb_fixture_api_v1_prefix {
  uint32_t struct_size;
  uint32_t struct_version;
  uint32_t api_version;
  uint32_t reserved_u32;
  uint64_t api_flags;
  nexilb_fixture_status_name_fn status_name;
  nexilb_fixture_library_info_fn library_info;
} nexilb_fixture_api_v1_prefix;

#define NEXILB_FIXTURE_API_V1_PREFIX_SIZE UINT64_C(40)

NEXILB_FIXTURE_PUBLIC int32_t NEXILB_FIXTURE_CALL
nexilb_get_abi_version(nexilb_fixture_abi_version_t *out_version);

NEXILB_FIXTURE_PUBLIC int32_t NEXILB_FIXTURE_CALL
nexilb_get_api(uint32_t requested_api, uint64_t table_size, void *table);

#ifdef __cplusplus
}
#endif

#endif
