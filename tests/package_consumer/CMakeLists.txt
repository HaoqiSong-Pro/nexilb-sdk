cmake_minimum_required(VERSION 3.26)

project(NexiLBPackageConsumer LANGUAGES C CXX)

if(NOT DEFINED NexiLB_DIR OR NexiLB_DIR STREQUAL "")
  message(FATAL_ERROR "NexiLB_DIR must name the verified SDK config directory")
endif()
find_package(NexiLB 1.0.0 EXACT CONFIG REQUIRED
  PATHS "${NexiLB_DIR}" NO_DEFAULT_PATH)

if(NOT NexiLB_PACKAGE_STATE STREQUAL "release")
  message(FATAL_ERROR
    "This consumer test requires a verified v4 release package state.")
endif()

if(NOT EXISTS "${NexiLB_CATALOG_BUNDLE}")
  message(FATAL_ERROR "Installed NexiLB catalog bundle is missing.")
endif()
file(READ "${NexiLB_CATALOG_BUNDLE}" catalog_bundle_json)
string(JSON expected_catalog_sha256 ERROR_VARIABLE catalog_json_error
  GET "${catalog_bundle_json}" catalog_sha256)
string(LENGTH "${expected_catalog_sha256}" expected_catalog_sha256_length)
if(catalog_json_error OR NOT expected_catalog_sha256_length EQUAL 64 OR
   expected_catalog_sha256 MATCHES "[^0-9a-f]")
  message(FATAL_ERROR "Installed NexiLB catalog bundle has an invalid hash.")
endif()

add_executable(nexilb_package_consumer main.c)
add_executable(nexilb_package_cpp_consumer main.cpp)
target_link_libraries(nexilb_package_consumer PRIVATE NexiLB::Runtime)
target_link_libraries(nexilb_package_cpp_consumer PRIVATE NexiLB::Runtime)
target_compile_definitions(nexilb_package_consumer PRIVATE
  "NEXILB_EXPECTED_CATALOG_SHA256=\"${expected_catalog_sha256}\"")
set_target_properties(nexilb_package_consumer PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED YES
  C_EXTENSIONS NO
)
set_target_properties(nexilb_package_cpp_consumer PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(nexilb_package_consumer PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(nexilb_package_cpp_consumer PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
endif()
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(nexilb_package_cpp_consumer PRIVATE
    -static -static-libgcc -static-libstdc++)
endif()
if(NOT WIN32)
  set_target_properties(
    nexilb_package_consumer nexilb_package_cpp_consumer PROPERTIES
    INSTALL_RPATH "$ORIGIN")
endif()

enable_testing()
add_test(NAME nexilb.package-consumer COMMAND nexilb_package_consumer)
add_test(NAME nexilb.package-cpp-consumer COMMAND nexilb_package_cpp_consumer)
if(WIN32)
  set_tests_properties(
    nexilb.package-consumer nexilb.package-cpp-consumer PROPERTIES
    ENVIRONMENT_MODIFICATION
      "PATH=path_list_prepend:$<TARGET_FILE_DIR:NexiLB::Runtime>")
endif()

install(TARGETS nexilb_package_consumer nexilb_package_cpp_consumer
  RUNTIME DESTINATION bin)
nexilb_stage_runtime(
  TARGETS nexilb_package_consumer nexilb_package_cpp_consumer
  DESTINATION bin)
