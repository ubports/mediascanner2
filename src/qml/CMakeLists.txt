include_directories(..)

add_library(mediascanner-qml MODULE
  plugin.cc
  MediaFileWrapper.cc
  MediaStoreWrapper.cc
)

set_target_properties(mediascanner-qml PROPERTIES AUTOMOC TRUE)
qt5_use_modules(mediascanner-qml Qml)
target_link_libraries(mediascanner-qml mediascanner)

install(
  TARGETS mediascanner-qml
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner"
)

install(
  FILES qmldir
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner"
)
