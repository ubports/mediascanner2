add_definitions(${MEDIASCANNER_CFLAGS} ${GST_CFLAGS})
include_directories(..)

add_executable(scannerdaemon
  scannerdaemon.cc
  )

target_link_libraries(scannerdaemon mediascanner2 ${MEDIASCANNER_LDFLAGS} ${GST_LDFLAGS})

install(TARGETS scannerdaemon
RUNTIME DESTINATION usr/bin
)
