include_directories(../src)

add_executable(basic basic.cc)
target_link_libraries(basic scannerstuff ${SQLITE_LDFLAGS} ${GST_LDFLAGS})
add_test(basic basic)
