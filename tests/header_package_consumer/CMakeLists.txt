cmake_minimum_required(VERSION 3.26)
project(NexiLBHeadersConsumer LANGUAGES CXX)
find_package(NexiLBHeaders 1.0.0 EXACT CONFIG REQUIRED)
add_executable(nexilb_headers_consumer main.cpp)
target_link_libraries(nexilb_headers_consumer PRIVATE NexiLB::Headers)
target_compile_features(nexilb_headers_consumer PRIVATE cxx_std_17)

