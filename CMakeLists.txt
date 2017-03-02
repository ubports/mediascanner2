cmake_minimum_required(VERSION 3.0.2)
project(mediascanner2 CXX C)

set(MEDIASCANNER_VERSION "0.112")

execute_process(
  COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/get-soversion.sh
  OUTPUT_VARIABLE MEDIASCANNER_SOVERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Error running get-soversion.sh script")
endif()

set(MEDIASCANNER_LIBVERSION "${MEDIASCANNER_SOVERSION}.${MEDIASCANNER_VERSION}")

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

option(full_warnings "All possible compiler warnings." OFF)

include(FindPkgConfig)
pkg_check_modules(MEDIASCANNER_DEPS REQUIRED
  gio-2.0
  gio-unix-2.0
  sqlite3>=3.8.5
)
pkg_check_modules(GST gstreamer-1.0 gstreamer-pbutils-1.0 REQUIRED)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
pkg_check_modules(PIXBUF gdk-pixbuf-2.0 REQUIRED)
pkg_check_modules(EXIF libexif REQUIRED)
pkg_check_modules(TAGLIB taglib REQUIRED)
pkg_check_modules(DBUSCPP dbus-cpp REQUIRED)
pkg_check_modules(APPARMOR libapparmor REQUIRED)
pkg_check_modules(UDISKS udisks2 REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")
if(${full_warnings})
# C does not have any more warning flags.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
endif()
find_package(CoverageReport)

include(GNUInstallDirs)
set(LIBDIR $CMAKE_INSTALL_LIBDIR)
enable_testing()

add_subdirectory(src/mediascanner)
add_subdirectory(src/extractor)
add_subdirectory(src/daemon)
add_subdirectory(src/ms-dbus)
add_subdirectory(src/qml/Ubuntu/MediaScanner.0.1)
add_subdirectory(src/utils)
add_subdirectory(test)

# Install pkg-config file
configure_file(mediascanner-2.0.pc.in mediascanner-2.0.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-2.0.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Install Upstart user session job
configure_file(mediascanner-2.0.conf.in mediascanner-2.0.conf)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-2.0.conf
  DESTINATION ${CMAKE_INSTALL_DATADIR}/upstart/sessions
)

enable_coverage_report(
  TARGETS
    mediascanner
    extractor-client
    extractor-backend
    mediascanner-extractor
    scannerstuff
    scannerdaemon
    ms-dbus
    mediascanner-dbus
    mediascanner-qml
    query
    mountwatcher
  FILTER
    ${CMAKE_SOURCE_DIR}/tests/*
    ${CMAKE_BINARY_DIR}/*
  TESTS
    basic
    test_mediastore
    test_metadataextractor
    test_extractorbackend
    test_sqliteutils
    test_mfbuilder
    test_subtreewatcher
    test_volumemanager
    test_dbus
    test_qml
    test_util
)

