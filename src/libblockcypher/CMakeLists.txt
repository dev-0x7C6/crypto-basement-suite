
file(GLOB_RECURSE libblockcypher-v1-public public/*.hpp)
file(GLOB_RECURSE libblockcypher-v1-private-headers private/*.hpp)
file(GLOB_RECURSE libblockcypher-v1-private-sources private/*.cpp)

add_library(libblockcypher-v1 SHARED
    ${libblockcypher-v1-public}
    ${libblockcypher-v1-private-headers}
    ${libblockcypher-v1-private-sources}
)

target_include_directories(libblockcypher-v1 PUBLIC public/includes)
target_include_directories(libblockcypher-v1 PRIVATE private/src)

target_link_libraries(libblockcypher-v1 PRIVATE
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    fmt::fmt
    nlohmann_json::nlohmann_json
    curlpp
    curl
)

add_subdirectory(examples)
