include("${CMAKE_SOURCE_DIR}/cmake/compiler/enable_cxx14.cmake")

# Enable full warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
