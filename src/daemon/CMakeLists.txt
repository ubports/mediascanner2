add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS} ${EXIF_CFLAGS} ${UDISKS_CFLAGS}
${PIXBUF_CFLAGS})
include_directories(..)

add_library(scannerstuff STATIC
  InvalidationSender.cc
  MetadataExtractor.cc
  MountWatcher.cc
  SubtreeWatcher.cc
  Scanner.cc
  ../mediascanner/utils.cc
)

target_link_libraries(scannerstuff ${GST_LIBRARIES} ${EXIF_LDFLAGS} ${UDISKS_LDFLAGS}
${PIXBUF_LDFLAGS})

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
