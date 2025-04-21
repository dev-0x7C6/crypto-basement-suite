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
    benchmark::benchmark_main
    spdlog::spdlog
)

target_link_libraries(cbs
    cbs-indicators
    cbs-model
    thin-curl-requests
    spdlog::spdlog
)
