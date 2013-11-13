if (NOT DEFINED GTEST_ROOT)
    set(GTEST_ROOT /usr/src/gtest)
endif()

set(GTEST_SRC_DIR "${GTEST_ROOT}/src")
set(GTEST_INCLUDE_DIR ${GTEST_ROOT})

add_library(gtest STATIC
${GTEST_SRC_DIR}/gtest-all.cc
)

set_target_properties(gtest PROPERTIES INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})

# Clang complains about unused private field 'pretty_' in gtest-internal-inl.h.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-Wno-unused-private-field")
endif()

add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS})
include_directories(../src)

add_definitions(-DTEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_executable(basic basic.cc)
target_link_libraries(basic mediascanner scannerstuff gtest)
add_test(basic basic)

add_executable(sqliteutilstest sqliteutilstest.cc)
target_link_libraries(sqliteutilstest gtest ${MEDIASCANNER_LIBRARIES})
add_test(sqliteutilstest sqliteutilstest)
