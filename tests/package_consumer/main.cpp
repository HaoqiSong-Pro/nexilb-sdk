#include "nexilb/nexilb.hpp"

#include <cstdint>
#include <utility>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      return __LINE__;                                                         \
    }                                                                          \
  } while (false)

int main() {
  auto loaded = nexilb::bootstrap();
  CHECK(loaded);
  auto library = std::move(loaded).value();
  auto info = library.info();
  CHECK(info);
  CHECK(info.value().abi_major == NEXILB_ABI_MAJOR);
  CHECK((info.value().api_flags & NEXILB_API_FLAG_CONTRACT_PROTOTYPE) != 0u);
  auto devices = library.device_count();
  CHECK(devices && devices.value() == std::uint32_t{0});
  auto catalog = library.create_catalog();
  CHECK(catalog);
  return 0;
}
