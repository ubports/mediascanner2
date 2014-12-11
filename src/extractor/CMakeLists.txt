
add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS} ${EXIF_CFLAGS})
include_directories(.. ${CMAKE_CURRENT_BINARY_DIR})

add_library(extractor-client STATIC
  MetadataExtractor.cc
  dbus-generated.c
  dbus-marshal.cc
  ../mediascanner/utils.cc
)

add_executable(mediascanner-extractor
  main.cc
  ExtractorBackend.cc
)

target_link_libraries(mediascanner-extractor
  mediascanner
  extractor-client
  ${GST_LDFLAGS}
  ${EXIF_LDFLAGS}
)

install(
  TARGETS mediascanner-extractor
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/mediascanner-2.0
)

find_program(gdbus_codegen gdbus-codegen)
if(NOT gdbus_codegen)
  msg(FATAL_ERROR "Could not locate gdbus-codegen")
endif()

add_custom_command(
  OUTPUT dbus-generated.c dbus-generated.h
  COMMAND ${gdbus_codegen} --interface-prefix=com.canonical.MediaScanner2 --generate-c-code dbus-generated --c-namespace MS ${CMAKE_CURRENT_SOURCE_DIR}/dbus-interface.xml
  MAIN_DEPENDENCY dbus-interface.xml
)
