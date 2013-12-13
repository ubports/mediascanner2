include_directories(..)

add_library(UbuntuMediaScanner MODULE plugin.cc)
qt5_use_modules(UbuntuMediaScanner Qml)
target_link_libraries(UbuntuMediaScanner mediascanner)
