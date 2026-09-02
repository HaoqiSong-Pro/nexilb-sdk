include(CMakeParseArguments)

function(nexilb_stage_runtime)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "DESTINATION" "TARGETS")
  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "nexilb_stage_runtime received unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()
  if(NOT ARG_TARGETS)
    message(FATAL_ERROR "nexilb_stage_runtime requires TARGETS")
  endif()
  if(NOT ARG_DESTINATION)
    message(FATAL_ERROR "nexilb_stage_runtime requires DESTINATION")
  endif()
  if(IS_ABSOLUTE "${ARG_DESTINATION}" OR
     ARG_DESTINATION MATCHES "(^|[/\\])\\.\\.([/\\]|$)")
    message(FATAL_ERROR
      "nexilb_stage_runtime DESTINATION must be a normalized relative path")
  endif()
  foreach(target IN LISTS ARG_TARGETS)
    if(NOT TARGET "${target}")
      message(FATAL_ERROR "nexilb_stage_runtime target does not exist: ${target}")
    endif()
  endforeach()

  if(NOT NexiLB_PACKAGE_STATE STREQUAL "release")
    message(FATAL_ERROR
      "NexiLB package verification did not establish the release state")
  endif()
  foreach(required_variable IN ITEMS
      NexiLB_RUNTIME_FILE NexiLB_RUNTIME_SIZE NexiLB_RUNTIME_SHA256
      NexiLB_RUNTIME_LOAD_NAME
      NexiLB_CASE_VERIFY_FILE NexiLB_CASE_VERIFY_SIZE NexiLB_CASE_VERIFY_SHA256
      NexiLB_CASE_TEST_FILE NexiLB_CASE_TEST_SIZE NexiLB_CASE_TEST_SHA256)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
      message(FATAL_ERROR
        "NexiLB package verification did not provide ${required_variable}")
    endif()
  endforeach()
  set(_nexilb_source "${NexiLB_RUNTIME_FILE}")
  set(_nexilb_expected_size "${NexiLB_RUNTIME_SIZE}")
  set(_nexilb_expected_sha256 "${NexiLB_RUNTIME_SHA256}")
  file(SIZE "${_nexilb_source}" _nexilb_actual_size)
  file(SHA256 "${_nexilb_source}" _nexilb_actual_sha256)
  if(NOT _nexilb_actual_size EQUAL _nexilb_expected_size OR
     NOT _nexilb_actual_sha256 STREQUAL _nexilb_expected_sha256)
    message(FATAL_ERROR "Installed NexiLB runtime does not match its manifest")
  endif()

  get_filename_component(_nexilb_runtime_name "${_nexilb_source}" NAME)
  set(_nexilb_runtime_load_name "${NexiLB_RUNTIME_LOAD_NAME}")
  get_filename_component(_nexilb_case_verify_name
    "${NexiLB_CASE_VERIFY_FILE}" NAME)
  get_filename_component(_nexilb_case_test_name
    "${NexiLB_CASE_TEST_FILE}" NAME)
  set(_nexilb_install_code [=[
set(_nexilb_source [==[@_nexilb_source@]==])
set(_nexilb_destination_dir
  "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/@ARG_DESTINATION@")
set(_nexilb_destination
  "${_nexilb_destination_dir}/@_nexilb_runtime_name@")
set(_nexilb_load_destination
  "${_nexilb_destination_dir}/@_nexilb_runtime_load_name@")
if(EXISTS "${_nexilb_destination}")
  message(FATAL_ERROR
    "Refusing to overwrite an existing staged runtime: ${_nexilb_destination}")
endif()
if(NOT "@_nexilb_runtime_load_name@" STREQUAL "@_nexilb_runtime_name@" AND
   (EXISTS "${_nexilb_load_destination}" OR
    IS_SYMLINK "${_nexilb_load_destination}"))
  message(FATAL_ERROR
    "Refusing to overwrite an existing staged runtime load name: ${_nexilb_load_destination}")
endif()
file(MAKE_DIRECTORY "${_nexilb_destination_dir}")
file(INSTALL DESTINATION "${_nexilb_destination_dir}"
  TYPE SHARED_LIBRARY FILES "${_nexilb_source}")
file(SIZE "${_nexilb_destination}" _nexilb_staged_size)
file(SHA256 "${_nexilb_destination}" _nexilb_staged_sha256)
if(NOT _nexilb_staged_size EQUAL @_nexilb_expected_size@ OR
   NOT _nexilb_staged_sha256 STREQUAL "@_nexilb_expected_sha256@")
  message(FATAL_ERROR "Staged NexiLB runtime failed manifest verification")
endif()
if(NOT "@_nexilb_runtime_load_name@" STREQUAL "@_nexilb_runtime_name@")
  file(CREATE_LINK "@_nexilb_runtime_name@" "${_nexilb_load_destination}"
    SYMBOLIC RESULT _nexilb_link_result)
  if(NOT _nexilb_link_result STREQUAL "0")
    file(COPY_FILE "${_nexilb_destination}" "${_nexilb_load_destination}")
  endif()
  if(NOT EXISTS "${_nexilb_load_destination}")
    message(FATAL_ERROR "Staged NexiLB SONAME was not created")
  endif()
  file(SIZE "${_nexilb_load_destination}" _nexilb_load_size)
  file(SHA256 "${_nexilb_load_destination}" _nexilb_load_sha256)
  if(NOT _nexilb_load_size EQUAL @_nexilb_expected_size@ OR
     NOT _nexilb_load_sha256 STREQUAL "@_nexilb_expected_sha256@")
    message(FATAL_ERROR "Staged NexiLB SONAME failed manifest verification")
  endif()
endif()
foreach(_nexilb_tool IN ITEMS case_verify case_test)
  if(_nexilb_tool STREQUAL "case_verify")
    set(_nexilb_tool_name "@_nexilb_case_verify_name@")
    set(_nexilb_tool_size "@NexiLB_CASE_VERIFY_SIZE@")
    set(_nexilb_tool_sha256 "@NexiLB_CASE_VERIFY_SHA256@")
  else()
    set(_nexilb_tool_name "@_nexilb_case_test_name@")
    set(_nexilb_tool_size "@NexiLB_CASE_TEST_SIZE@")
    set(_nexilb_tool_sha256 "@NexiLB_CASE_TEST_SHA256@")
  endif()
  set(_nexilb_tool_destination
    "${_nexilb_destination_dir}/${_nexilb_tool_name}")
  if(NOT EXISTS "${_nexilb_tool_destination}" OR
     IS_DIRECTORY "${_nexilb_tool_destination}" OR
     IS_SYMLINK "${_nexilb_tool_destination}")
    message(FATAL_ERROR
      "Installed NexiLB tool is missing or unsafe: ${_nexilb_tool_name}")
  endif()
  file(SIZE "${_nexilb_tool_destination}" _nexilb_tool_actual_size)
  file(SHA256 "${_nexilb_tool_destination}" _nexilb_tool_actual_sha256)
  if(NOT _nexilb_tool_actual_size EQUAL _nexilb_tool_size OR
     NOT _nexilb_tool_actual_sha256 STREQUAL _nexilb_tool_sha256)
    message(FATAL_ERROR
      "Installed NexiLB tool failed manifest verification: ${_nexilb_tool_name}")
  endif()
endforeach()
]=])
  string(CONFIGURE "${_nexilb_install_code}" _nexilb_install_code @ONLY)
  install(CODE "${_nexilb_install_code}")
endfunction()
