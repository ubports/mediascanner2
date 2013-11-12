add_subdirectory(mediascanner)

add_executable(watcher watchertest.cc)

add_definitions(${MEDIASCANNER_CFLAGS})

add_executable(scannerdaemon scannerdaemon.cc)
target_link_libraries(scannerdaemon mediascanner2 ${MEDIASCANNER_LDFLAGS})

install(TARGETS scannerdaemon
RUNTIME DESTINATION usr/bin
)

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
