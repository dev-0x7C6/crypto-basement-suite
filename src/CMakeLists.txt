add_subdirectory(common)
add_subdirectory(cli)
add_subdirectory(indicators)
add_subdirectory(model)
add_subdirectory(networking)

add_subdirectory(libcoingecko)
add_subdirectory(libblockfrost)

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
    thin-curl-requests
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
)
