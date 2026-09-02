#include "nexilb/nexilb.hpp"

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      return __LINE__;                                                         \
    }                                                                          \
  } while (false)

static_assert(!std::is_copy_constructible<nexilb::error>::value,
              "error ownership must be move-only");
static_assert(std::is_nothrow_move_constructible<nexilb::error>::value,
              "moving an error must not fail");
static_assert(!std::is_copy_constructible<nexilb::catalog>::value,
              "catalog ownership must be move-only");
static_assert(std::is_nothrow_move_constructible<nexilb::catalog>::value,
              "moving a catalog must not fail");

int main() {
  auto loaded = nexilb::bootstrap();
  CHECK(loaded);
  auto library = std::move(loaded).value();
  const auto *const api = library.api_table();
  CHECK(api != nullptr);

  CHECK(library.function_available(&nexilb_api_v1::library_catalog_create));
  CHECK(library.function_available(&nexilb_api_v1::device_count));
  CHECK(library.function_available(&nexilb_api_v1::error_format));
  CHECK(!library.function_available(&nexilb_api_v1::context_create));
  CHECK(!library.function_available(&nexilb_api_v1::model_create));

  auto library_info = library.info();
  CHECK(library_info);
  CHECK(library_info.value().abi_major == NEXILB_ABI_MAJOR);
  CHECK(library_info.value().api_min <= NEXILB_API_V1);
  CHECK(library_info.value().api_max >= NEXILB_API_V1);
  CHECK((library_info.value().api_flags &
         NEXILB_API_FLAG_CONTRACT_PROTOTYPE) != 0u);

  auto created = library.create_catalog();
  CHECK(created);
  nexilb::catalog original = std::move(created).value();
  nexilb::catalog catalog = std::move(original);
  CHECK(!original);
  CHECK(catalog);
  CHECK(catalog.api_table() == api);

  auto model_count = catalog.count(NEXILB_CATALOG_MODEL);
  auto chain_count = catalog.count(NEXILB_CATALOG_MODEL_CHAIN);
  auto case_count = catalog.count(NEXILB_CATALOG_CASE);
  CHECK(model_count && model_count.value() == UINT64_C(4));
  CHECK(chain_count && chain_count.value() == UINT64_C(3));
  CHECK(case_count && case_count.value() == UINT64_C(19));

  auto first_model = catalog.id(NEXILB_CATALOG_MODEL, 0u);
  CHECK(first_model && first_model.value() == "model.NPhaseContactAngle");

  auto descriptor = catalog.descriptor_json(first_model.value());
  CHECK(descriptor);
  CHECK(descriptor.value().find("\"public_name\": \"NPhaseContactAngle\"") !=
        std::string::npos);

  auto catalog_info = catalog.info();
  CHECK(catalog_info);
  CHECK(catalog_info.value().descriptor_schema_major == 0u);
  CHECK(catalog_info.value().descriptor_schema_minor == 2u);

  auto schema = catalog.schema_json(
      "urn:nexilb:schema:catalog-descriptor:0.2");
  CHECK(schema);
  CHECK(schema.value().find(
            "\"$schema\": \"https://json-schema.org/draft/2020-12/schema\"") !=
        std::string::npos);

  auto devices = library.device_count();
  CHECK(devices && devices.value() == 0u);

  auto missing_device = library.device_info_json(0u);
  CHECK(!missing_device);
  CHECK(missing_device.status() == NEXILB_STATUS_NOT_FOUND);
  CHECK(missing_device.diagnostic());
  CHECK(missing_device.diagnostic().api_table() == api);

  auto diagnostic = missing_device.take_diagnostic();
  CHECK(!missing_device.diagnostic());
  nexilb::error moved_diagnostic = std::move(diagnostic);
  CHECK(!diagnostic);
  CHECK(moved_diagnostic);

  auto info = moved_diagnostic.info();
  CHECK(info);
  CHECK(info.value().status == NEXILB_STATUS_NOT_FOUND);
  CHECK(info.value().subsystem == NEXILB_ERROR_SUBSYSTEM_DEVICE);

  auto message = moved_diagnostic.format();
  CHECK(message);
  CHECK(message.value() == "no device exists for the requested ordinal");

  nexilb_error_t raw_error = moved_diagnostic.release();
  CHECK(raw_error != nullptr);
  CHECK(!moved_diagnostic);
  CHECK(api->error_destroy != nullptr);
  api->error_destroy(&raw_error);
  CHECK(raw_error == nullptr);

  auto closed = catalog.close();
  CHECK(closed);
  CHECK(!catalog);
  return 0;
}
