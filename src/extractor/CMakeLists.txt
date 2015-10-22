
add_definitions(${MEDIASCANNER_DEPS_CFLAGS} ${GST_CFLAGS} ${EXIF_CFLAGS} ${PIXBUF_CFLAGS})
include_directories(.. ${CMAKE_CURRENT_BINARY_DIR})

# Build stubs/skeletons for D-Bus interface
find_program(gdbus_codegen gdbus-codegen)
if(NOT gdbus_codegen)
  msg(FATAL_ERROR "Could not locate gdbus-codegen")
endif()

add_custom_command(
  OUTPUT dbus-generated.c dbus-generated.h
  COMMAND ${gdbus_codegen} --interface-prefix=com.canonical.MediaScanner2 --generate-c-code dbus-generated --c-namespace MS ${CMAKE_CURRENT_SOURCE_DIR}/dbus-interface.xml
  MAIN_DEPENDENCY dbus-interface.xml
)

# The client code for the extractor daemon
add_library(extractor-client STATIC
  MetadataExtractor.cc
  dbus-generated.c
  dbus-marshal.cc
  ../mediascanner/utils.cc
)
target_link_libraries(extractor-client
  mediascanner
)

# The backend code for the extractor daemon, as a library for use by tests
add_library(extractor-backend STATIC
  ExtractorBackend.cc
)
target_link_libraries(extractor-backend
  extractor-client
  ${GST_LDFLAGS}
  ${EXIF_LDFLAGS}
  ${PIXBUF_LDFLAGS}
)

add_executable(mediascanner-extractor
  main.cc
)
target_link_libraries(mediascanner-extractor extractor-backend)

install(
  TARGETS mediascanner-extractor
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/mediascanner-2.0
)

configure_file(
  com.canonical.MediaScanner2.Extractor.service.in
  com.canonical.MediaScanner2.Extractor.service)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.MediaScanner2.Extractor.service
  DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
