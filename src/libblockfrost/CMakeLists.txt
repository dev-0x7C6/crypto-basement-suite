
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
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    fmt::fmt
    nlohmann_json::nlohmann_json
    PkgConfig::Curl++
    CURL::libcurl
)

add_subdirectory(examples)
