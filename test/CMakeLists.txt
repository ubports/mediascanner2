find_package(GMock REQUIRED)

set(TEST_LIBS
  ${GTEST_BOTH_LIBRARIES}
)

add_definitions(${MEDIASCANNER_DEPS_CFLAGS} ${GST_CFLAGS} ${DBUSCPP_CFLAGS} -DQT_NO_KEYWORDS)
include_directories(../src)

configure_file(test_config.h.in test_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(
  services/com.canonical.MediaScanner2.service.in
  services/com.canonical.MediaScanner2.service)

configure_file(
  services/com.canonical.MediaScanner2.Extractor.service.in
  services/com.canonical.MediaScanner2.Extractor.service)

add_executable(basic basic.cc)
target_link_libraries(basic mediascanner scannerstuff ${TEST_LIBS})
add_test(basic basic)
# The gvfs modules interfere with the private D-Bus test fixtures
set_tests_properties(basic PROPERTIES
  ENVIRONMENT "GIO_MODULE_DIR=${CMAKE_CURRENT_BINARY_DIR}/modules")

add_executable(test_mediastore test_mediastore.cc ../src/mediascanner/utils.cc)
target_link_libraries(test_mediastore mediascanner ${TEST_LIBS})
add_test(test_mediastore test_mediastore)

add_executable(test_extractorbackend test_extractorbackend.cc)
target_link_libraries(test_extractorbackend extractor-backend ${TEST_LIBS})
add_test(test_extractorbackend test_extractorbackend)

add_executable(test_metadataextractor test_metadataextractor.cc)
target_link_libraries(test_metadataextractor extractor-client ${TEST_LIBS})
add_test(test_metadataextractor test_metadataextractor)
# The gvfs modules interfere with the private D-Bus test fixtures
set_tests_properties(test_metadataextractor PROPERTIES
  ENVIRONMENT "GIO_MODULE_DIR=${CMAKE_CURRENT_BINARY_DIR}/modules")

add_executable(test_subtreewatcher test_subtreewatcher.cc)
target_link_libraries(test_subtreewatcher mediascanner scannerstuff ${TEST_LIBS})
add_test(test_subtreewatcher test_subtreewatcher)
# The gvfs modules interfere with the private D-Bus test fixtures
set_tests_properties(test_subtreewatcher PROPERTIES
  ENVIRONMENT "GIO_MODULE_DIR=${CMAKE_CURRENT_BINARY_DIR}/modules")

add_executable(test_volumemanager test_volumemanager.cc)
target_link_libraries(test_volumemanager mediascanner scannerstuff ${TEST_LIBS})
add_test(test_volumemanager test_volumemanager)
# The gvfs modules interfere with the private D-Bus test fixtures
set_tests_properties(test_volumemanager PROPERTIES
  ENVIRONMENT "GIO_MODULE_DIR=${CMAKE_CURRENT_BINARY_DIR}/modules")

add_executable(test_sqliteutils test_sqliteutils.cc)
target_link_libraries(test_sqliteutils ${TEST_LIBS} ${MEDIASCANNER_DEPS_LDFLAGS})
add_test(test_sqliteutils test_sqliteutils)

add_executable(test_mfbuilder test_mfbuilder.cc)
target_link_libraries(test_mfbuilder ${TEST_LIBS} mediascanner)
add_test(test_mfbuilder test_mfbuilder)

add_executable(test_dbus test_dbus.cc)
target_link_libraries(test_dbus ${TEST_LIBS} mediascanner ms-dbus)
add_test(test_dbus test_dbus)

add_executable(test_qml test_qml.cc)
qt5_use_modules(test_qml QuickTest)
target_link_libraries(test_qml mediascanner)
add_test(test_qml test_qml -import ${CMAKE_BINARY_DIR}/src/qml)
set_tests_properties(test_qml PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=minimal;QT_SELECT=qt5"
  TIMEOUT 600)

add_test(test_qml_dbus test_qml -import ${CMAKE_BINARY_DIR}/src/qml)
set_tests_properties(test_qml_dbus PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=minimal;MEDIASCANNER_USE_DBUS=1;QT_SELECT=qt5"
  TIMEOUT 600)

add_test(test_qml_nodb qmltestrunner -import ${CMAKE_BINARY_DIR}/src/qml -input ${CMAKE_CURRENT_SOURCE_DIR}/test_qml_nodb.qml)
set_tests_properties(test_qml_nodb PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=minimal;QT_SELECT=qt5;MEDIASCANNER_CACHEDIR=${CMAKE_CURRENT_BINARY_DIR}/qml-nodb-cachedir;DBUS_SESSION_BUS_ADDRESS="
  TIMEOUT 600)


add_executable(test_util test_util.cc ../src/mediascanner/utils.cc)
target_link_libraries(test_util ${TEST_LIBS} ${GLIB_LDFLAGS})
add_test(test_util test_util)
