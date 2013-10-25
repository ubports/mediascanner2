project(scantest CXX C)
cmake_minimum_required(VERSION 2.8.9)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

option(full_warnings "All possible compiler warnings." OFF)

include(FindPkgConfig)
pkg_check_modules(GST gstreamer-1.0 gstreamer-pbutils-1.0 REQUIRED)
pkg_search_module(SQLITE sqlite3 REQUIRED)
pkg_search_module(UNITY unity REQUIRED)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")
if(${full_warnings})
# C does not have any more warning flags.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
endif()

include(GNUInstallDirs)
set(LIBDIR $CMAKE_INSTALL_LIBDIR)

#add_executable(scanner Scanner.cc FileTypeDetector.cc MediaFile.cc MediaStore.cc scannermain.cc)
add_executable(watcher watchertest.cc)

add_definitions(${GST_CFLAGS})
add_executable(scannerdaemon scannerdaemon.cc SubtreeWatcher.cc MediaFile.cc MediaStore.cc
FileTypeDetector.cc metadataextractor.cc Scanner.cc utils.cc mozilla/fts3_porter.c mozilla/Normalize.c)
target_link_libraries(scannerdaemon ${GST_LDFLAGS} ${SQLITE_LDFLAGS})

add_executable(query query.cpp utils.cc)
target_link_libraries(query ${SQLITE_LDFLAGS})

include_directories(${UNITY_INCLUDE_DIRS})
add_executable(scanscope scanscope.cpp)
target_link_libraries(scanscope ${SQLITE_LDFLAGS} ${UNITY_LDFLAGS})

add_executable(scaletest scaletest.cc MediaStore.cc MediaFile.cc utils.cc
 metadataextractor.cc FileTypeDetector.cc mozilla/fts3_porter.c mozilla/Normalize.c)
target_link_libraries(scaletest ${SQLITE_LDFLAGS} ${GST_LDFLAGS})
