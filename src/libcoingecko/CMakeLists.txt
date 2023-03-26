add_library(libcoingecko-v3 SHARED
    price.cpp)

target_include_directories(libcoingecko-v3 PRIVATE includes)

target_link_libraries(libcoingecko-v3 PRIVATE
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    fmt::fmt
    nlohmann_json::nlohmann_json
    curlpp
    curl
)

