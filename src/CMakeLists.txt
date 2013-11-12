add_subdirectory(mediascanner)
add_subdirectory(daemon)

add_executable(watcher watchertest.cc)

add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS})

add_executable(query query.cc)
target_link_libraries(query mediascanner2
${MEDIASCANNER_LDFLAGS})

include_directories(${UNITY_INCLUDE_DIRS})
add_executable(scanscope scanscope.cc)
target_link_libraries(scanscope
${MEDIASCANNER_LDFLAGS} ${UNITY_LDFLAGS})

add_executable(scaletest scaletest.cc)
target_link_libraries(scaletest mediascanner2
${MEDIASCANNER_LDFLAGS})
