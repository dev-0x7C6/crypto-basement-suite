
file(GLOB_RECURSE libblockfrost-v0-public public/*.hpp)
file(GLOB_RECURSE libblockfrost-v0-private-headers private/*.hpp)
file(GLOB_RECURSE libblockfrost-v0-private-sources private/*.cpp)

add_library(libblockfrost-v0 SHARED
    ${libblockfrost-v0-public}
    ${libblockfrost-v0-private-headers}
    ${libblockfrost-v0-private-sources}
)

target_include_directories(libblockfrost-v0 PUBLIC public/includes)
target_include_directories(libblockfrost-v0 PRIVATE private/src)

target_link_libraries(libblockfrost-v0 PRIVATE
    thin-curl-requests
    fmt::fmt
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

set_target_properties(libblockfrost-v0 PROPERTIES PREFIX "")
install(TARGETS libblockfrost-v0 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory(examples)
