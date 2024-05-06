cmake_minimum_required(VERSION 3.28)
project(thread_plus)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

add_library(thread_plus)
file (
  GLOB thread_plus_ccm
  "${CMAKE_CURRENT_LIST_DIR}/src/*.ccm"
)
target_sources(thread_plus
  PUBLIC 
    FILE_SET CXX_MODULES FILES ${thread_plus_ccm}
)

# Add Tests Conditionally
if (EXIST ${CMAKE_CURRENT_LIST_DIR}/lib/test-lib/CMakeLists.txt)
  add_subdirectory(lib/test-lib EXCLUDE_FROM_ALL)
  add_executable(
    thread_plus_tests
    EXCLUDE_FROM_ALL 
    ${CMAKE_CURRENT_LIST_DIR}/tests/thread_tests.cpp
  )
  target_link_libraries(thread_plus_tests
    test_lib 
    thread_plus
  )
  target_compile_options(thread_plus_tests PRIVATE "-Wall" "-Werror")
endif()