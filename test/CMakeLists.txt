if (NOT DEFINED GTEST_ROOT)
    set(GTEST_ROOT /usr/src/gtest)
endif()

set(GTEST_SRC_DIR "${GTEST_ROOT}/src")
set(GTEST_INCLUDE_DIR ${GTEST_ROOT})

add_library(gtest STATIC
${GTEST_SRC_DIR}/gtest-all.cc
)

set_target_properties(gtest PROPERTIES INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})

# Clang complains about unused private field 'pretty_' in gtest-internal-inl.h.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-Wno-unused-private-field")
endif()

add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS})
include_directories(../src)

configure_file(test_config.h.in test_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(basic basic.cc)
target_link_libraries(basic mediascanner scannerstuff gtest)
add_test(basic basic)

add_executable(test_mediastore test_mediastore.cc)
target_link_libraries(test_mediastore mediascanner gtest)
add_test(test_mediastore test_mediastore)

add_executable(test_metadataextractor test_metadataextractor.cc)
target_link_libraries(test_metadataextractor mediascanner scannerstuff gtest)
add_test(test_metadataextractor test_metadataextractor)

add_executable(test_sqliteutils test_sqliteutils.cc)
target_link_libraries(test_sqliteutils gtest ${MEDIASCANNER_LIBRARIES})
add_test(test_sqliteutils test_sqliteutils)

add_executable(test_mfbuilder test_mfbuilder.cc)
target_link_libraries(test_mfbuilder gtest mediascanner)
add_test(test_mfbuilder test_mfbuilder)

add_executable(test_qml test_qml.cc)
qt5_use_modules(test_qml QuickTest)
target_link_libraries(test_qml mediascanner)
add_test(test_qml test_qml -input ${CMAKE_CURRENT_SOURCE_DIR}/qml -import ${CMAKE_BINARY_DIR}/src/qml)
set_tests_properties(test_qml
  PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=minimal")
