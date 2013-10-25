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
enable_testing()

add_subdirectory(src)
add_subdirectory(test)
