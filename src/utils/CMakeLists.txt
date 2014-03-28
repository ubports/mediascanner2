add_definitions(${MEDIASCANNER_CFLAGS})
include_directories(..)

add_executable(watcher watchertest.cc)

add_executable(query query.cc)
target_link_libraries(query mediascanner
${MEDIASCANNER_LDFLAGS})

add_executable(scaletest scaletest.cc)
target_link_libraries(scaletest mediascanner
${MEDIASCANNER_LDFLAGS})
