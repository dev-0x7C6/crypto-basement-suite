
file(GLOB_RECURSE libcoingecko-v3-public public/*.hpp)
file(GLOB_RECURSE libcoingecko-v3-private-headers private/*.hpp)
file(GLOB_RECURSE libcoingecko-v3-private-sources private/*.cpp)

add_library(libcoingecko-v3 SHARED
    ${libcoingecko-v3-public}
    ${libcoingecko-v3-private-headers}
    ${libcoingecko-v3-private-sources}
)
    
target_include_directories(libcoingecko-v3 PUBLIC public/includes)
target_include_directories(libcoingecko-v3 PRIVATE private/src)

target_link_libraries(libcoingecko-v3 PRIVATE
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    fmt::fmt
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

set_target_properties(libcoingecko-v3 PROPERTIES PREFIX "")
install(TARGETS libcoingecko-v3 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory(examples)
