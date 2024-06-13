cmake_minimum_required(VERSION 3.28)
project(moderna_thread_plus)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

add_library(moderna_thread_plus)
file (
  GLOB moderna_thread_plus_src
  "${CMAKE_CURRENT_LIST_DIR}/src/thread_plus/*.ccm"
)
target_sources(moderna_thread_plus
  PUBLIC 
    FILE_SET CXX_MODULES FILES ${moderna_thread_plus_src}
)

# add_library(moderna_lock_free)
# file (
#   GLOB moderna_lock_free_src
#   "${CMAKE_CURRENT_LIST_DIR}/src/lock_free/*.ccm"
# )
# target_sources(moderna_lock_free
#   PUBLIC
#     FILE_SET CXX_MODULES FILES ${moderna_lock_free_src}
# )

# add_library(moderna_fine_grained)
# file (
#   GLOB moderna_fine_grained_src
#   "${CMAKE_CURRENT_LIST_DIR}/src/fine_grained/*.ccm"
# )
# target_sources(moderna_fine_grained
#   PUBLIC
#     FILE_SET CXX_MODULES FILES ${moderna_fine_grained_src}
# )

if (
  EXISTS ${CMAKE_CURRENT_LIST_DIR}/../test-lib/CMakeLists.txt 
  AND PROJECT_IS_TOP_LEVEL
)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../test-lib test-lib)
  moderna_add_test(
    thread_plus_tests 
    ${CMAKE_CURRENT_LIST_DIR}/tests/thread_plus_tests.cpp 
    LIBRARIES moderna_thread_plus moderna_test_lib
    SANITIZERS all
  )
  moderna_add_test(
    lock_free_tests
    ${CMAKE_CURRENT_LIST_DIR}/tests/lock_free_tests.cpp
    LIBRARIES moderna_lock_free moderna_thread_plus moderna_test_lib
    SANITIZERS all
  )
  moderna_add_test(
    bench_lockfree
    ${CMAKE_CURRENT_LIST_DIR}/tests/bench_lockfree.cpp
    LIBRARIES moderna_thread_plus moderna_test_lib moderna_lock_free
    SANITIZERS all
  )
elseif (PROJECT_IS_TOP_LEVEL)
  message("Clone Moderna Library to Test")
endif()