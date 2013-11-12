add_library(mediascanner2 STATIC
  MediaFile.cc
  Album.cc
  MediaStore.cc
  utils.cc
  mozilla/fts3_porter.c
  mozilla/Normalize.c)

add_definitions(${MEDIASCANNER_CFLAGS})
