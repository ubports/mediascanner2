include_directories(../../..)
add_definitions(${DBUSCPP_CFLAGS} -DQT_NO_KEYWORDS)

set(QML_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner.0.1")

add_library(mediascanner-qml MODULE
  plugin.cc
  MediaFileWrapper.cc
  MediaStoreWrapper.cc
  StreamingModel.cc
  MediaFileModelBase.cc
  AlbumModelBase.cc
  AlbumsModel.cc
  ArtistsModel.cc
  GenresModel.cc
  SongsModel.cc
  SongsSearchModel.cc
)

set_target_properties(mediascanner-qml PROPERTIES AUTOMOC TRUE)
qt5_use_modules(mediascanner-qml Qml Concurrent DBus)
target_link_libraries(mediascanner-qml mediascanner ms-dbus)

install(
  TARGETS mediascanner-qml
  LIBRARY DESTINATION ${QML_PLUGIN_DIR}
)

file(GLOB QMLFILES
  qmldir
)

add_custom_target(mediascanner-qmlfiles ALL
  COMMAND cp ${QMLFILES} ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${QMLFILES}
)

install(
  FILES ${QMLFILES}
  DESTINATION ${QML_PLUGIN_DIR}
)

if(NOT CMAKE_CROSSCOMPILING)
  find_program(qmlplugindump_exe qmlplugindump)
  if(NOT qmlplugindump_exe)
    msg(FATAL_ERROR "Could not locate qmlplugindump.")
  endif()

  # qmlplugindump doesn't run reliably in the CI environment (it seems
  # to be instantiating the types, which fails when there is no media
  # database).  So add a 
  add_custom_target(update-qmltypes
    COMMAND QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/src/qml ${qmlplugindump_exe} -notrelocatable Ubuntu.MediaScanner 0.1 > ${CMAKE_CURRENT_SOURCE_DIR}/plugin.qmltypes
    DEPENDS mediascanner-qml mediascanner-qmlfiles
  )
endif()

install(
  FILES plugin.qmltypes
  DESTINATION ${QML_PLUGIN_DIR}
)
