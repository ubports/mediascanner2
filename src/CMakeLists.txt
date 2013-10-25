
#add_executable(scanner Scanner.cc FileTypeDetector.cc MediaFile.cc MediaStore.cc scannermain.cc)
add_executable(watcher watchertest.cc)

add_definitions(${GST_CFLAGS})
add_executable(scannerdaemon scannerdaemon.cc SubtreeWatcher.cc MediaFile.cc MediaStore.cc
FileTypeDetector.cc metadataextractor.cc Scanner.cc utils.cc
 ../mozilla/fts3_porter.c ../mozilla/Normalize.c)
target_link_libraries(scannerdaemon ${GST_LDFLAGS} ${SQLITE_LDFLAGS})

add_executable(query query.cc utils.cc)
target_link_libraries(query ${SQLITE_LDFLAGS})

include_directories(${UNITY_INCLUDE_DIRS})
add_executable(scanscope scanscope.cc)
target_link_libraries(scanscope ${SQLITE_LDFLAGS} ${UNITY_LDFLAGS})

add_executable(scaletest scaletest.cc MediaStore.cc MediaFile.cc utils.cc
 metadataextractor.cc FileTypeDetector.cc ../mozilla/fts3_porter.c ../mozilla/Normalize.c)
target_link_libraries(scaletest ${SQLITE_LDFLAGS} ${GST_LDFLAGS})
