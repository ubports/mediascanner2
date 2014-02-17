add_library(mediascanner SHARED
  MediaFile.cc
  MediaFileBuilder.cc
  Album.cc
  MediaStore.cc
  utils.cc
  mozilla/fts3_porter.c
  mozilla/Normalize.c
)

add_definitions(${MEDIASCANNER_CFLAGS})
target_link_libraries(mediascanner ${MEDIASCANNER_LIBRARIES})

set_target_properties(mediascanner PROPERTIES
  OUTPUT_NAME "mediascanner-2.0"
  VERSION 0.0.0
  SOVERSION 0
)

install(
  TARGETS mediascanner
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  Album.hh
  MediaFile.hh
  MediaFileBuilder.hh
  MediaStore.hh
  scannercore.hh
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/mediascanner-2.0/mediascanner"
)
