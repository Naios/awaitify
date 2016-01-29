if (${MSVC_VERSION} LESS 1900)
  message(FATAL_ERROR "You are using an unsupported version of Visual Studio "
                      "which doesn't support all required C++14 features. "
                      "(Visual Studio 2015 (version >= 1900) is required!)")
endif()

if (PLATFORM EQUAL 64)
  add_definitions("-D_WIN64")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
