#include "nexilb/nexilb.h"

#include <cstring>
#include <type_traits>

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                         \
      return __LINE__;                                                          \
    }                                                                           \
  } while (false)

static_assert(std::is_standard_layout<nexilb_api_v1>::value,
              "the API table must remain standard-layout");
static_assert(sizeof(nexilb_status_t) == 4u, "status width changed");

int main() {
  nexilb_api_v1 api{};
  nexilb_library_info_t info{};

  CHECK(nexilb_get_api(NEXILB_API_V1, sizeof(api), &api) ==
        NEXILB_STATUS_OK);
  CHECK(api.status_name != nullptr);
  CHECK(api.library_info != nullptr);

  info.struct_size = static_cast<uint32_t>(sizeof(info));
  info.struct_version = NEXILB_STRUCT_VERSION_1;
  CHECK(api.library_info(&info, nullptr) == NEXILB_STATUS_OK);
  CHECK(std::strcmp(info.build_id, "06-contract-prototype") == 0);
  CHECK((info.api_flags & NEXILB_API_FLAG_CONTRACT_PROTOTYPE) != 0u);
  return 0;
}
