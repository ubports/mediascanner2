add_definitions(${MEDIASCANNER_DEPS_CFLAGS})
include_directories(..)

add_executable(watcher watchertest.cc)

add_executable(query query.cc)
target_link_libraries(query mediascanner
${MEDIASCANNER_DEPS_LDFLAGS})

add_executable(mountwatcher mountwatcher.cc)
target_link_libraries(mountwatcher scannerstuff)
