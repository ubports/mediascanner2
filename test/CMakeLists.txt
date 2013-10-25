include_directories(../src)

add_executable(basic basic.cc)
target_link_libraries(basic scannerstuff ${SQLITE_LDFLAGS} ${GST_LDFLAGS})
add_test(basic basic)
set_property(TEST basic PROPERTY ENVIRONMENT "TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}")
set_property(TEST basic APPEND PROPERTY ENVIRONMENT "SOURCE_DIR=${CMAKE_SOURCE_DIR}")
