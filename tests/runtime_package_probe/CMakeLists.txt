cmake_minimum_required(VERSION 3.26)
project(NexiLBRuntimePackageMustBeAbsent LANGUAGES C)

find_package(NexiLB 1.0.0 EXACT CONFIG REQUIRED
  PATHS "${NexiLB_DIR}" NO_DEFAULT_PATH)

if(NOT TARGET NexiLB::Runtime OR NOT TARGET NexiLB::CaseVerify OR
   NOT TARGET NexiLB::CaseTest)
  message(FATAL_ERROR "an installed runtime package must provide all imported targets")
endif()
