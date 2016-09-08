include_directories(..)
add_definitions(${MEDIASCANNER_CFLAGS} ${DBUSCPP_CFLAGS} ${APPARMOR_CFLAGS})

add_library(ms-dbus STATIC
  dbus-codec.cc
  service-skeleton.cc
  service-stub.cc
  )

# gcc 6 is miscompiling something in dbus-cpp, and disabling
# optimisation makes the problem go away.
#  https://bugs.launchpad.net/ubuntu/+source/mediascanner2/+bug/1621002
set_source_files_properties(dbus-codec.cc PROPERTIES COMPILE_FLAGS "-O0")

target_link_libraries(ms-dbus
  mediascanner
  ${DBUSCPP_LDFLAGS}
  ${APPARMOR_LDFLAGS})
# Compile with -fPIC, since this code is linked into the QML plugin.
set_target_properties(ms-dbus PROPERTIES COMPILE_FLAGS "-fPIC ")

add_executable(mediascanner-dbus
  main.cc
)
target_link_libraries(mediascanner-dbus ms-dbus)
set_target_properties(mediascanner-dbus
  PROPERTIES OUTPUT_NAME "mediascanner-dbus-2.0")

install(
  TARGETS mediascanner-dbus
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/mediascanner-2.0
)

configure_file(
  com.canonical.MediaScanner2.service.in
  com.canonical.MediaScanner2.service)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.MediaScanner2.service
  DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
