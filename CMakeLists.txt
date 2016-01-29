project(awaitify CXX)
cmake_minimum_required(VERSION 2.8)

# Only interpret if() arguments as variables or
# keywords when unquoted.
cmake_policy(SET CMP0054 NEW)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

include(cmake/configure_platform.cmake)
include(cmake/configure_compiler.cmake)

include(dep/configure_dependencies.cmake)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(LIBRARY_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/awaitify/awaitify.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/awaitify.cpp
)

add_library(awaitify STATIC
  ${LIBRARY_SOURCES}
)

set(AWAITIFY_LINK_LIBRARIES
  ${Boost_LIBRARIES}
)

if (WITH_TESTS)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/dep
  )

  # Build the tests
  file(GLOB TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp
  )

  add_executable(awaitify_tests
    ${TEST_SOURCES}
  )

  target_link_libraries(awaitify_tests
    awaitify
    ${AWAITIFY_LINK_LIBRARIES}
  )
endif()
