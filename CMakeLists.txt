cmake_minimum_required(VERSION 3.1.0)
project(mkgeoip LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadCatchorgCatch2()
MkDownloadMeasurementKitMkData()
MkDownloadMeasurementKitMkCurl()
MkDownloadMeasurementKitMkIPLookup()
MkDownloadMeasurementKitMkMMDB()
MkDownloadMeasurementKitMkMock()
MkDownloadMMDBDatabases()
MkDownloadCABundle()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND MKGEOIP_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
  MkDownloadMeasurementKitPrebuiltWindowsLibmaxminddb()
  list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_WINDOWS_LIBMAXMINDDB_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_LIBMAXMINDDB_LIBRARY_PATH}")
endif()

# Checks
# ------

include(CheckIncludeFiles)

check_include_files(maxminddb.h MKGEOIP_HAVE_MAXMINDDB_H)
find_library(LIBMAXMINDDB_LIBRARY maxminddb)
if (("${MKGEOIP_HAVE_MAXMINDDB_H}" STREQUAL "") OR
    ("${LIBMAXMINDDB_LIBRARY}" STREQUAL "LIBMAXMINDDB_LIBRARY-NOTFOUND"))
  message(FATAL_ERROR "Cannot find libmaxminddb")
endif()
list(APPEND MKGEOIP_LIBS "${LIBMAXMINDDB_LIBRARY}")

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(MKGEOIP_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})

add_library(mkgeoip mkgeoip.cpp)
add_executable(mkgeoip-client mkgeoip-client.cpp)
target_include_directories(mkgeoip-client PUBLIC ${MKGEOIP_INCLUDES})
list (APPEND MKGEOIP_LIBS ${CURL_LIBRARIES})
install(FILES mkgeoip.h DESTINATION include/measurement-kit)
install(TARGETS mkgeoip-client DESTINATION bin)
if("${WIN32}" OR "${MINGW}")
  list(APPEND MKGEOIP_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND MKGEOIP_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND MKGEOIP_LIBS Threads::Threads)
target_link_libraries(mkgeoip-client "${MKGEOIP_LIBS}" mkgeoip)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  add_executable(tests tests.cpp)
  target_link_libraries(tests "${MKGEOIP_LIBS}")
  enable_testing()
  add_test(NAME mocked_tests COMMAND tests)
  add_test(NAME basic_test COMMAND mkgeoip-client)
endif()
