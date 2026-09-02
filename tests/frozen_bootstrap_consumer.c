#include "nexilb_bootstrap_fixture.h"

#include <string.h>

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      return __LINE__;                                                          \
    }                                                                           \
  } while (0)

_Static_assert(sizeof(nexilb_fixture_api_v1_prefix) == 40u,
               "frozen bootstrap prefix changed");
_Static_assert(offsetof(nexilb_fixture_api_v1_prefix, status_name) == 24u,
               "frozen status_name slot changed");
_Static_assert(offsetof(nexilb_fixture_api_v1_prefix, library_info) == 32u,
               "frozen library_info slot changed");

int main(void) {
  struct guarded_prefix {
    uint64_t before;
    nexilb_fixture_api_v1_prefix api;
    uint64_t after;
  } guarded;
  struct extended_prefix {
    nexilb_fixture_api_v1_prefix api;
    unsigned char known_runtime_tail[496];
    void *future_slots[4];
    uint64_t guard;
  } extended;
  nexilb_fixture_api_v1_prefix rejected;
  unsigned char rejected_before[sizeof(rejected)];
  struct extended_info {
    nexilb_fixture_library_info_t info;
    uint64_t future[2];
    uint64_t guard;
  } extended_info;
  nexilb_fixture_abi_version_t version = {0};
  nexilb_fixture_buffer_t buffer;
  char status_text[64];
  char small_text[4] = {'x', 'x', 'x', 'x'};

  CHECK(nexilb_get_abi_version(NULL) ==
        NEXILB_FIXTURE_STATUS_INVALID_ARGUMENT);
  CHECK(nexilb_get_abi_version(&version) == NEXILB_FIXTURE_STATUS_OK);
  CHECK(version.abi_major == 1u);
  CHECK(version.api_min == NEXILB_FIXTURE_API_V1);
  CHECK(version.api_max == NEXILB_FIXTURE_API_V1);

  (void)memset(&rejected, 0xa5, sizeof(rejected));
  (void)memcpy(rejected_before, &rejected, sizeof(rejected));
  CHECK(nexilb_get_api(0u, sizeof(rejected), &rejected) ==
        NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION);
  CHECK(memcmp(&rejected, rejected_before, sizeof(rejected)) == 0);
  CHECK(nexilb_get_api(UINT32_MAX, sizeof(rejected), &rejected) ==
        NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION);
  CHECK(memcmp(&rejected, rejected_before, sizeof(rejected)) == 0);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1, sizeof(rejected), NULL) ==
        NEXILB_FIXTURE_STATUS_INVALID_ARGUMENT);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1, 0u, &rejected) ==
        NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1, 1u, &rejected) ==
        NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1,
                       NEXILB_FIXTURE_API_V1_PREFIX_SIZE - 1u,
                       &rejected) == NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL);
  CHECK(memcmp(&rejected, rejected_before, sizeof(rejected)) == 0);

  (void)memset(&guarded, 0, sizeof(guarded));
  guarded.before = UINT64_C(0x1122334455667788);
  guarded.after = UINT64_C(0x8877665544332211);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1, sizeof(guarded.api),
                       &guarded.api) ==
        NEXILB_FIXTURE_STATUS_OK);
  CHECK(guarded.before == UINT64_C(0x1122334455667788));
  CHECK(guarded.after == UINT64_C(0x8877665544332211));
  CHECK(guarded.api.struct_size >= sizeof(guarded.api));
  CHECK(guarded.api.api_version == NEXILB_FIXTURE_API_V1);
  CHECK((guarded.api.api_flags & NEXILB_FIXTURE_API_FLAG_CONTRACT_PROTOTYPE) !=
        0u);
  CHECK(guarded.api.status_name != NULL);
  CHECK(guarded.api.library_info != NULL);

  (void)memset(&extended, 0, sizeof(extended));
  extended.guard = UINT64_C(0xa1b2c3d4e5f60718);
  CHECK(nexilb_get_api(NEXILB_FIXTURE_API_V1,
                       sizeof(extended) - sizeof(extended.guard),
                       &extended.api) == NEXILB_FIXTURE_STATUS_OK);
  CHECK(extended.future_slots[0] == NULL);
  CHECK(extended.future_slots[3] == NULL);
  CHECK(extended.guard == UINT64_C(0xa1b2c3d4e5f60718));

  CHECK(guarded.api.library_info(NULL, NULL) ==
        NEXILB_FIXTURE_STATUS_INVALID_ARGUMENT);
  (void)memset(&extended_info, 0, sizeof(extended_info));
  extended_info.info.struct_size = 1u;
  extended_info.info.struct_version = NEXILB_FIXTURE_STRUCT_VERSION_1;
  extended_info.guard = UINT64_C(0xfedcba9876543210);
  CHECK(guarded.api.library_info(&extended_info.info, NULL) ==
        NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL);
  CHECK(extended_info.guard == UINT64_C(0xfedcba9876543210));
  extended_info.info.struct_size = (uint32_t)sizeof(extended_info.info);
  extended_info.info.struct_version = NEXILB_FIXTURE_STRUCT_VERSION_1 + 1u;
  CHECK(guarded.api.library_info(&extended_info.info, NULL) ==
        NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION);
  CHECK(extended_info.guard == UINT64_C(0xfedcba9876543210));

  (void)memset(&extended_info, 0, sizeof(extended_info));
  extended_info.info.struct_size =
      (uint32_t)(sizeof(extended_info) - sizeof(extended_info.guard));
  extended_info.info.struct_version = NEXILB_FIXTURE_STRUCT_VERSION_1;
  extended_info.guard = UINT64_C(0xfedcba9876543210);
  CHECK(guarded.api.library_info(&extended_info.info, NULL) ==
        NEXILB_FIXTURE_STATUS_OK);
  CHECK(extended_info.info.abi_major == 1u);
  CHECK(strcmp(extended_info.info.build_id, "06-contract-prototype") == 0);
  CHECK(extended_info.future[0] == 0u && extended_info.future[1] == 0u);
  CHECK(extended_info.guard == UINT64_C(0xfedcba9876543210));

  (void)memset(&buffer, 0, sizeof(buffer));
  buffer.struct_size = (uint32_t)sizeof(buffer);
  buffer.struct_version = NEXILB_FIXTURE_STRUCT_VERSION_1;
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_OK, NULL) ==
        NEXILB_FIXTURE_STATUS_INVALID_ARGUMENT);
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION,
                                &buffer) == NEXILB_FIXTURE_STATUS_OK);
  CHECK(buffer.required_bytes == sizeof("NEXILB_STATUS_UNSUPPORTED_VERSION"));
  buffer.data = small_text;
  buffer.capacity_bytes = sizeof(small_text);
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION,
                                &buffer) ==
        NEXILB_FIXTURE_STATUS_BUFFER_TOO_SMALL);
  CHECK(small_text[0] == 'x' && small_text[3] == 'x');
  buffer.data = status_text;
  buffer.capacity_bytes = sizeof(status_text);
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION,
                                &buffer) ==
        NEXILB_FIXTURE_STATUS_OK);
  CHECK(strcmp(status_text, "NEXILB_STATUS_UNSUPPORTED_VERSION") == 0);
  buffer.struct_size = (uint32_t)sizeof(buffer) - 1u;
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_OK, &buffer) ==
        NEXILB_FIXTURE_STATUS_STRUCT_TOO_SMALL);
  buffer.struct_size = (uint32_t)sizeof(buffer);
  buffer.struct_version = NEXILB_FIXTURE_STRUCT_VERSION_1 + 1u;
  CHECK(guarded.api.status_name(NEXILB_FIXTURE_STATUS_OK, &buffer) ==
        NEXILB_FIXTURE_STATUS_UNSUPPORTED_VERSION);

  return 0;
}
