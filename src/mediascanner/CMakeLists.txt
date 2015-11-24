add_library(mediascanner SHARED
  MediaFile.cc
  MediaFileBuilder.cc
  MediaFilePrivate.cc
  Filter.cc
  Album.cc
  MediaStore.cc
  MediaStoreBase.cc
  FolderArtCache.cc
  utils.cc
  mozilla/fts3_porter.c
  mozilla/Normalize.c
)

set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/mediascanner-2.0.map")
set_target_properties(mediascanner PROPERTIES
    LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map} ")
set_target_properties(mediascanner PROPERTIES LINK_DEPENDS ${symbol_map})

add_definitions(${MEDIASCANNER_DEPS_CFLAGS})
target_link_libraries(mediascanner ${MEDIASCANNER_DEPS_LDFLAGS})

set_target_properties(mediascanner PROPERTIES
  OUTPUT_NAME "mediascanner-2.0"
  SOVERSION ${MEDIASCANNER_SOVERSION}
  VERSION ${MEDIASCANNER_LIBVERSION}
)

install(
  TARGETS mediascanner
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  Album.hh
  Filter.hh
  MediaFile.hh
  MediaFileBuilder.hh
  MediaStore.hh
  MediaStoreBase.hh
  scannercore.hh
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/mediascanner-2.0/mediascanner"
)
