include_directories(..)
add_definitions(${MEDIASCANNER_CFLAGS} ${DBUSCPP_CFLAGS})

add_executable(ms-dbus
  main.cc
  dbus-codec.cc
  service-skeleton.cc
)
target_link_libraries(ms-dbus mediascanner ${DBUSCPP_LDFLAGS})
