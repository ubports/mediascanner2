add_library(mediascanner SHARED
  MediaFile.cc
  MediaFileBuilder.cc
  MediaFilePrivate.cc
  Filter.cc
  Album.cc
  MediaStore.cc
  MediaStoreBase.cc
  utils.cc
  mozilla/fts3_porter.c
  mozilla/Normalize.c
)

set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/mediascanner-2.0.map")
set_target_properties(mediascanner PROPERTIES
    LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(mediascanner PROPERTIES LINK_DEPENDS ${symbol_map})

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
  MediaStoreBase.hh
  scannercore.hh
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/mediascanner-2.0/mediascanner"
)
