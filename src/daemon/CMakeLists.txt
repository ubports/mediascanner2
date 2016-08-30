add_definitions(${MEDIASCANNER_DEPS_CFLAGS} ${UDISKS_CFLAGS})
include_directories(..)

add_library(scannerstuff STATIC
  InvalidationSender.cc
  MountWatcher.cc
  VolumeManager.cc
  SubtreeWatcher.cc
  Scanner.cc
  ../mediascanner/utils.cc
)

target_link_libraries(scannerstuff extractor-client ${UDISKS_LDFLAGS})

add_executable(scannerdaemon
  scannerdaemon.cc
)

set_target_properties(scannerdaemon
  PROPERTIES OUTPUT_NAME "mediascanner-service-2.0")

target_link_libraries(scannerdaemon
  mediascanner
  scannerstuff
)

install(
  TARGETS scannerdaemon
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
