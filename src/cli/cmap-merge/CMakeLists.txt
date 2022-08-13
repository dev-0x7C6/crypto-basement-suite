add_executable(cbs-cmap-merge "main.cpp")

target_link_libraries(cbs-cmap-merge
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    cbs-common
)
