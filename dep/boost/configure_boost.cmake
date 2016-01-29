if(WIN32)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
  else()
    set(PLATFORM 32)
  endif()

  set(BOOST_DEBUG ON)
  if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
      set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-12.0)
    else()
      set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
    endif()
  else()
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found!"
                        "Please make sure it is set and the points to your Boost installation.")
  endif()

  set(Boost_USE_STATIC_LIBS     ON)
  set(Boost_USE_MULTITHREADED   ON)
  set(Boost_USE_STATIC_RUNTIME  OFF)

  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(Boost_ADDITIONAL_VERSIONS
  "1.59" "1.59.0"
  "1.60" "1.60.0"
)
find_package(Boost 1.58 REQUIRED
  system
  context
  coroutine
  thread
)

add_definitions(-DBOOST_DATE_TIME_NO_LIB)
add_definitions(-DBOOST_REGEX_NO_LIB)
add_definitions(-DBOOST_CHRONO_NO_LIB)

include_directories(
  ${Boost_INCLUDE_DIRS}
)
