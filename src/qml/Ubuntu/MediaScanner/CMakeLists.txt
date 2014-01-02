include_directories(../../..)

set(QML_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml/Ubuntu/MediaScanner.0.1")

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

  add_custom_target(mediascanner-qmltypes ALL
    COMMAND QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/src/qml ${qmlplugindump_exe} -notrelocatable Ubuntu.MediaScanner 0.1 > ${CMAKE_CURRENT_BINARY_DIR}/plugin.qmltypes
  )
  add_dependencies(mediascanner-qmltypes mediascanner-qml mediascanner-qmlfiles)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin.qmltypes
    DESTINATION ${QML_PLUGIN_DIR}
  )
endif()
