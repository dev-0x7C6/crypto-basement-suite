add_executable(cbs-cmap-squeeze "main.cpp")

target_link_libraries(cbs-cmap-squeeze
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    cbs-common
)
