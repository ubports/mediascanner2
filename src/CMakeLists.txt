add_executable(watcher watchertest.cc)

add_library(scannerstuff STATIC SubtreeWatcher.cc MediaFile.cc MediaStore.cc
FileTypeDetector.cc metadataextractor.cc Scanner.cc utils.cc
 ../mozilla/fts3_porter.c ../mozilla/Normalize.c)

add_definitions(${GST_CFLAGS})
add_executable(scannerdaemon scannerdaemon.cc)
target_link_libraries(scannerdaemon scannerstuff ${GST_LDFLAGS} ${SQLITE_LDFLAGS})

add_executable(query query.cc)
target_link_libraries(query scannerstuff ${GST_LDFLAGS} ${SQLITE_LDFLAGS})

include_directories(${UNITY_INCLUDE_DIRS})
add_executable(scanscope scanscope.cc)
target_link_libraries(scanscope ${SQLITE_LDFLAGS} ${UNITY_LDFLAGS})

add_executable(scaletest scaletest.cc)
target_link_libraries(scaletest scannerstuff ${SQLITE_LDFLAGS} ${GST_LDFLAGS})
