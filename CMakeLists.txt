cmake_minimum_required(VERSION 3.26)
project(NexiLBPublicSdk VERSION 1.0.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(nexilb_public_headers INTERFACE)
add_library(NexiLB::Headers ALIAS nexilb_public_headers)
set_target_properties(nexilb_public_headers PROPERTIES EXPORT_NAME Headers)
target_include_directories(nexilb_public_headers INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_compile_features(nexilb_public_headers INTERFACE c_std_11 cxx_std_17)

add_library(nexilb_c_header_check OBJECT tests/c_abi_layout.c)
target_link_libraries(nexilb_c_header_check PRIVATE NexiLB::Headers)
add_library(nexilb_cpp_header_check OBJECT tests/abi_contract_compile.cpp)
target_link_libraries(nexilb_cpp_header_check PRIVATE NexiLB::Headers)
target_compile_features(nexilb_cpp_header_check PRIVATE cxx_std_17)
add_library(nexilb_loader_contract OBJECT tests/dynamic_loader_contract.c)
target_link_libraries(nexilb_loader_contract PRIVATE NexiLB::Headers)
add_executable(nexilb_case_test tests/case_test.cpp)
target_link_libraries(nexilb_case_test PRIVATE NexiLB::Headers)
target_compile_features(nexilb_case_test PRIVATE cxx_std_17)

add_subdirectory(examples/secondary-development/nphase-wetting-minimal)
add_subdirectory(examples/secondary-development/imb-coupling-minimal)
add_subdirectory(examples/secondary-development/dem-contact-restart)
add_subdirectory(examples/secondary-development/pure-coupled-degeneration)

set(_package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/NexiLBHeaders")
configure_package_config_file(cmake/NexiLBHeadersConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/NexiLBHeadersConfig.cmake"
  INSTALL_DESTINATION "${_package_dir}")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/NexiLBHeadersConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)
install(TARGETS nexilb_public_headers EXPORT NexiLBHeadersTargets)
install(EXPORT NexiLBHeadersTargets NAMESPACE NexiLB::
  DESTINATION "${_package_dir}")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/NexiLBHeadersConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/NexiLBHeadersConfigVersion.cmake"
  DESTINATION "${_package_dir}")
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY contracts catalog examples
  DESTINATION "${CMAKE_INSTALL_DATADIR}/nexilb-sdk")
set(_sdk_documentation_dir "${CMAKE_INSTALL_DATADIR}/doc/NexiLB")
install(FILES LICENSE.md NOTICE.md SECURITY.md README.md
  DESTINATION "${_sdk_documentation_dir}")
install(DIRECTORY licensing/
  DESTINATION "${_sdk_documentation_dir}/licensing"
  FILES_MATCHING PATTERN "*.md" PATTERN "*.json")

find_package(Python3 3.10 QUIET COMPONENTS Interpreter)
include(CTest)
if(BUILD_TESTING)
  if(NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR
      "Python 3.10 or newer is required for SDK contract tests. "
      "If the shell's python command is a .cmd/.bat wrapper, pass the real "
      "interpreter explicitly as -DPython3_EXECUTABLE=<absolute-python.exe>.")
  endif()
  add_test(NAME nexilb.sdk-boundary COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_sdk_boundary.py")
  add_test(NAME nexilb.catalog-contract COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_catalog_schema.py"
    --schema "${CMAKE_CURRENT_SOURCE_DIR}/catalog/schema/nexilb-catalog-descriptor.schema.json"
    --descriptor-list "${CMAKE_CURRENT_SOURCE_DIR}/catalog/descriptors/catalog.list"
    --descriptor-directory "${CMAKE_CURRENT_SOURCE_DIR}/catalog/descriptors" --self-test)
  add_test(NAME nexilb.secondary-case-contracts COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/validate_secondary_examples.py")
  add_test(NAME nexilb.case-verify COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/case_verify.py")
  set(_chain_entries
    "native|${NEXILB_UNUSED}chain.nexilb.native|model.nexilb.NSAllen"
    "nphase|chain.NPhaseContactAngle|model.NPhaseContactAngle"
    "coupled|chain.NPhaseImbDemContactAngle|model.NPhaseImbDemContactAngle")
  foreach(_entry IN LISTS _chain_entries)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _label)
    list(GET _parts 1 _chain)
    list(GET _parts 2 _model)
    foreach(_dimension IN ITEMS 2 3)
      foreach(_precision IN ITEMS f32 f64)
        add_test(NAME "nexilb.case-test.${_label}.${_dimension}d.${_precision}"
          COMMAND nexilb_case_test "${_chain}" "${_model}"
            "${_dimension}" "${_precision}")
      endforeach()
    endforeach()
  endforeach()
  add_test(NAME nexilb.case-test.native-prescribed-motion
    COMMAND nexilb_case_test "chain.nexilb.native"
      "model.nexilb.NSAllenImbPrescribedMotion" 2 f64)
  add_test(NAME nexilb.case-test.reject-dimension
    COMMAND nexilb_case_test "chain.NPhaseContactAngle"
      "model.NPhaseContactAngle" 4 f64)
  add_test(NAME nexilb.case-test.reject-chain-model
    COMMAND nexilb_case_test "chain.NPhaseContactAngle"
      "model.NPhaseImbDemContactAngle" 3 f64)
  set_tests_properties(nexilb.case-test.reject-dimension
    nexilb.case-test.reject-chain-model PROPERTIES WILL_FAIL TRUE)
  add_test(NAME nexilb.headers-install-consumer COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_headers_install.py"
    --build-dir "${CMAKE_BINARY_DIR}")
  foreach(_consumer IN ITEMS
      nexilb_nphase_wetting_consumer
      nexilb_imb_coupling_consumer
      nexilb_dem_contact_consumer
      nexilb_degeneration_consumer)
    add_test(NAME "nexilb.consumer-fail-closed.${_consumer}"
      COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_consumer_blocked.py"
        "$<TARGET_FILE:${_consumer}>")
  endforeach()
  set_tests_properties(nexilb.sdk-boundary nexilb.catalog-contract
    nexilb.secondary-case-contracts nexilb.case-verify
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
