
file(GLOB_RECURSE libcoingecko-v3-public includes/*.hpp)
file(GLOB_RECURSE libcoingecko-v3-private-headers src/*.hpp)
file(GLOB_RECURSE libcoingecko-v3-private-sources src/*.cpp)

add_library(libcoingecko-v3 SHARED
    ${libcoingecko-v3-public}
    ${libcoingecko-v3-private-headers}
    ${libcoingecko-v3-private-sources}
)
    
target_include_directories(libcoingecko-v3 PUBLIC includes)
target_include_directories(libcoingecko-v3 PRIVATE src)

target_link_libraries(libcoingecko-v3 PRIVATE
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    fmt::fmt
    nlohmann_json::nlohmann_json
    curlpp
    curl
)

add_subdirectory(examples)
