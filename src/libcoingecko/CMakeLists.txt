add_library(libcoingecko-v3 SHARED
    src/api.hpp
    src/api.cpp
    src/categories/list.cpp
    src/simple/price.cpp
    src/ping.cpp
    src/simple/supported_vs_currencies.cpp
    src/list.cpp
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
