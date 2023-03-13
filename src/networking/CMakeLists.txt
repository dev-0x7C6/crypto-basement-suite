add_library(cbs-networking STATIC
    downloader.cpp
    downloader.hpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(curlpp REQUIRED)
pkg_check_modules(curl REQUIRED)

target_include_directories(cbs-networking PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(cbs-networking PUBLIC
    curlpp
    curl
)
