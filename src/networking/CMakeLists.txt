add_library(cbs-networking STATIC
    downloader.cpp
    downloader.hpp
)

target_include_directories(cbs-networking PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(cbs-networking PUBLIC
    curlpp
    curl
)
