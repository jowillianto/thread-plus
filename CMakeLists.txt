cmake_minimum_required(VERSION 3.28)
project(moderna_thread_plus)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

include (CTest)
add_library(moderna_thread_plus)
file (
  GLOB moderna_thread_plus_src
  "${CMAKE_CURRENT_LIST_DIR}/src/*.ccm"
)
target_sources(moderna_thread_plus
  PUBLIC 
    FILE_SET CXX_MODULES FILES ${moderna_thread_plus_src}
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeLists.txt AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/../test-lib)
  include (${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeLists.txt)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../test-lib test-lib)
  add_test_asan_tsan_usan_ubsan(thread_plus_tests ${CMAKE_CURRENT_LIST_DIR}/tests/tests.cpp "moderna_test_lib;moderna_thread_plus")
else()
  message("Clone Moderna Library to Test")
endif()