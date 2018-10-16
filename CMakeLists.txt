cmake_minimum_required(VERSION 3.1.0)
project(libgeoip LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadAdishavitArgh()
MkDownloadCatchorgCatch2()
MkDownloadMeasurementKitLibcurlx()
MkDownloadGeoIP()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND LIBGEOIP_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
endif()

# Checks
# ------

include(CheckIncludeFiles)

check_include_files(maxminddb.h LIBGEOIP_HAVE_MAXMINDDB_H)
find_library(LIBMAXMINDDB_LIBRARY maxminddb)
if (("${LIBGEOIP_HAVE_MAXMINDDB_H}" STREQUAL "") OR
    ("${LIBMAXMINDDB_LIBRARY}" STREQUAL "LIBMAXMINDDB_LIBRARY-NOTFOUND"))
  message(FATAL_ERROR "Cannot find libmaxminddb")
endif()
list(APPEND LIBGEOIP_LIBS "${LIBMAXMINDDB_LIBRARY}")

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(LIBGEOIP_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})

add_executable(libgeoip-client libgeoip-client.cpp)
target_include_directories(libgeoip-client PUBLIC ${LIBGEOIP_INCLUDES})
list (APPEND LIBGEOIP_LIBS ${CURL_LIBRARIES})
install(FILES libgeoip.h DESTINATION include/measurement-kit/libgeoip)
install(TARGETS libgeoip-client DESTINATION bin)
if("${WIN32}" OR "${MINGW}")
  list(APPEND LIBGEOIP_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND LIBGEOIP_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND LIBGEOIP_LIBS Threads::Threads)
target_link_libraries(libgeoip-client "${LIBGEOIP_LIBS}")

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  enable_testing()
  add_test(NAME basic_test COMMAND libgeoip-client)
endif()
