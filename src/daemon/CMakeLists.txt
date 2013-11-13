add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS})
include_directories(..)

add_library(scannerstuff STATIC
  FileTypeDetector.cc
  MetadataExtractor.cc
  SubtreeWatcher.cc
  Scanner.cc
)

add_executable(scannerdaemon
  scannerdaemon.cc
)

target_link_libraries(scannerdaemon
  mediascanner
  scannerstuff
  ${MEDIASCANNER_LDFLAGS}
  ${GST_LDFLAGS}
)

install(
  TARGETS scannerdaemon
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
