#include "nexilb/nexilb.h"

#include <string.h>

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      return __LINE__;                                                          \
    }                                                                           \
  } while (0)

typedef struct nexilb_api_v1_bootstrap_prefix {
  uint32_t struct_size;
  uint32_t struct_version;
  uint32_t api_version;
  uint32_t reserved_u32;
  uint64_t api_flags;
  nexilb_status_name_fn status_name;
  nexilb_library_info_fn library_info;
} nexilb_api_v1_bootstrap_prefix;

typedef struct guarded_prefix {
  nexilb_api_v1_bootstrap_prefix api;
  uint64_t sentinel;
} guarded_prefix;

int main(void) {
  guarded_prefix guarded;
  nexilb_buffer_t buffer;
  char text[64];

  (void)memset(&guarded, 0, sizeof(guarded));
  guarded.sentinel = UINT64_C(0x6e6578696c625631);
  CHECK(sizeof(guarded.api) == NEXILB_API_V1_MANDATORY_PREFIX_SIZE);
  CHECK(nexilb_get_api(NEXILB_API_V1, sizeof(guarded.api),
                       (nexilb_api_v1 *)&guarded.api) == NEXILB_STATUS_OK);
  CHECK(guarded.sentinel == UINT64_C(0x6e6578696c625631));
  CHECK(guarded.api.status_name != NULL);
  CHECK(guarded.api.library_info != NULL);

  (void)memset(&buffer, 0, sizeof(buffer));
  buffer.struct_size = (uint32_t)sizeof(buffer);
  buffer.struct_version = NEXILB_STRUCT_VERSION_1;
  buffer.data = text;
  buffer.capacity_bytes = sizeof(text);
  CHECK(guarded.api.status_name(NEXILB_STATUS_UNSUPPORTED_CAPABILITY,
                                &buffer) == NEXILB_STATUS_OK);
  CHECK(strcmp(text, "NEXILB_STATUS_UNSUPPORTED_CAPABILITY") == 0);
  return 0;
}
