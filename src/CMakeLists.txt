add_subdirectory(indicators)
add_subdirectory(model)

find_package(range-v3)
find_package(spdlog)
find_package(benchmark)

add_executable(cbs "main.cpp")
add_executable(cbs-benchmark "benchmark.cpp")

target_link_libraries(cbs-benchmark
    cbs-indicators
    cbs-model
    benchmark::benchmark
    range-v3::concepts
    range-v3::meta
    range-v3::range-v3
    spdlog::spdlog
)

target_link_libraries(cbs
    cbs-indicators
    cbs-model
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
)
