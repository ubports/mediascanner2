include_directories(..)
add_definitions(${MEDIASCANNER_CFLAGS} ${DBUSCPP_CFLAGS})

add_library(ms-dbus STATIC
  dbus-codec.cc
  service-skeleton.cc
  service-stub.cc
)

target_link_libraries(ms-dbus mediascanner ${DBUSCPP_LDFLAGS})
set_target_properties(ms-dbus PROPERTIES COMPILE_FLAGS "-fPIC")

add_executable(mediascanner-dbus
  main.cc
)
target_link_libraries(mediascanner-dbus ms-dbus)
set_target_properties(mediascanner-dbus
  PROPERTIES OUTPUT_NAME "mediascanner-dbus-2.0")
