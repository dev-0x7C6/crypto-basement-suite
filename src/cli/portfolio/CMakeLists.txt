add_executable(cbs-portfolio "main.cpp")

target_link_libraries(cbs-portfolio
    cbs-networking
    libcoingecko-v3
    libblockfrost-v0
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    cbs-common
)
