include_directories(..)

add_library(mediascanner-qml MODULE
  plugin.cc
  MediaFileWrapper.cc
  MediaStoreWrapper.cc
)

set_target_properties(mediascanner-qml PROPERTIES AUTOMOC TRUE)
qt5_use_modules(mediascanner-qml Qml)
target_link_libraries(mediascanner-qml mediascanner)

set(QML_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner.0.1")

install(
  TARGETS mediascanner-qml
  LIBRARY DESTINATION ${QML_PLUGIN_DIR}
)

install(
  FILES qmldir
  DESTINATION ${QML_PLUGIN_DIR}
)
