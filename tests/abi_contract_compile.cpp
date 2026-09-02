#include "nexilb/nexilb.h"
#include "nexilb/nexilb.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

static_assert(sizeof(nexilb_status_t) == 4u, "status width is ABI-significant");
static_assert(sizeof(nexilb_bool_t) == 4u, "boolean width is ABI-significant");
static_assert(std::is_standard_layout<nexilb_api_v1>::value,
              "the function table must remain standard-layout");
static_assert(offsetof(nexilb_api_v1, status_name) == 24u,
              "the v1 bootstrap prefix moved");
static_assert(offsetof(nexilb_api_v1, library_info) == 32u,
              "the v1 bootstrap prefix moved");
static_assert(NEXILB_API_V1_MANDATORY_PREFIX_SIZE == 40u,
              "the accepted historical prefix changed");
static_assert(!std::is_copy_constructible<nexilb::error>::value,
              "owned error handles must remain move-only");
static_assert(!std::is_copy_constructible<nexilb::catalog>::value,
              "owned catalog handles must remain move-only");

using get_api_signature = nexilb_status_t(NEXILB_CALL *)(
    std::uint32_t, std::uint64_t, nexilb_api_v1 *);
using get_abi_signature = nexilb_status_t(NEXILB_CALL *)(
    nexilb_abi_version_t *);
static_assert(std::is_same<decltype(&nexilb_get_api), get_api_signature>::value,
              "nexilb_get_api calling convention or signature changed");
static_assert(
    std::is_same<decltype(&nexilb_get_abi_version), get_abi_signature>::value,
    "nexilb_get_abi_version calling convention or signature changed");

int nexilb_abi_contract_compile_anchor() { return 0; }

