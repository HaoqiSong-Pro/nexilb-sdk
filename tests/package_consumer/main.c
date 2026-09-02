#include "nexilb/nexilb.h"

#include <stddef.h>

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      return __LINE__;                                                          \
    }                                                                           \
  } while (0)

static int hex_nibble(char value) {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  return -1;
}

static int hash_matches(const uint8_t hash[32], const char *hex) {
  size_t index;
  for (index = 0u; index < 32u; ++index) {
    const int high = hex_nibble(hex[index * 2u]);
    const int low = hex_nibble(hex[index * 2u + 1u]);
    if (high < 0 || low < 0 ||
        hash[index] != (uint8_t)((high << 4) | low)) {
      return 0;
    }
  }
  return hex[64] == '\0';
}

int main(void) {
  nexilb_abi_version_t version = {0};
  nexilb_api_v1 api = {0};
  nexilb_catalog_t catalog = NULL;
  nexilb_catalog_info_t catalog_info = {0};
  uint64_t count = 0u;

  CHECK(nexilb_get_abi_version(&version) == NEXILB_STATUS_OK);
  CHECK(version.abi_major == 1u);
  CHECK(nexilb_get_api(NEXILB_API_V1, sizeof(api), &api) ==
        NEXILB_STATUS_OK);
  CHECK((api.api_flags & NEXILB_API_FLAG_CONTRACT_PROTOTYPE) != 0u);
  CHECK((api.api_flags & NEXILB_API_FLAG_LIBRARY_CATALOG_AVAILABLE) != 0u);
  CHECK(api.library_catalog_create != NULL);
  CHECK(api.catalog_info != NULL);
  CHECK(api.catalog_count != NULL);
  CHECK(api.catalog_destroy != NULL);
  CHECK(api.context_catalog_create == NULL);
  CHECK(api.model_create == NULL);
  CHECK(api.library_catalog_create(&catalog, NULL) == NEXILB_STATUS_OK);
  catalog_info.struct_size = (uint32_t)sizeof(catalog_info);
  catalog_info.struct_version = NEXILB_STRUCT_VERSION_1;
  CHECK(api.catalog_info(catalog, &catalog_info, NULL) == NEXILB_STATUS_OK);
  CHECK(hash_matches(catalog_info.catalog_sha256,
                     NEXILB_EXPECTED_CATALOG_SHA256));
  CHECK(api.catalog_count(catalog, NEXILB_CATALOG_MODEL, &count, NULL) ==
        NEXILB_STATUS_OK);
  CHECK(count == 4u);
  CHECK(api.catalog_count(catalog, NEXILB_CATALOG_MODEL_CHAIN, &count, NULL) ==
        NEXILB_STATUS_OK);
  CHECK(count == 3u);
  CHECK(api.catalog_count(catalog, NEXILB_CATALOG_CASE, &count, NULL) ==
        NEXILB_STATUS_OK);
  CHECK(count == 19u);
  CHECK(api.catalog_destroy(&catalog, NULL) == NEXILB_STATUS_OK);
  CHECK(catalog == NULL);
  return 0;
}
