add_subdirectory(indicators)
add_subdirectory(model)

find_package(range-v3)

add_executable(cbs "main.cpp")

target_link_libraries(cbs
    cbs-indicators
    cbs-model
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
)
