include_directories(..)

add_library(mediascanner-qml MODULE
  plugin.cc
  MediaFileWrapper.cc
  MediaStoreWrapper.cc
  moc_MediaFileWrapper.cc
  moc_MediaStoreWrapper.cc
  moc_plugin.cc
)
qt5_generate_moc(MediaFileWrapper.hh moc_MediaFileWrapper.cc)
qt5_generate_moc(MediaStoreWrapper.hh moc_MediaStoreWrapper.cc)
qt5_generate_moc(plugin.hh moc_plugin.cc)

qt5_use_modules(mediascanner-qml Qml)
target_link_libraries(mediascanner-qml mediascanner)

install(
  TARGETS mediascanner-qml
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner"
)
