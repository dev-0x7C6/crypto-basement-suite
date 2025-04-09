add_executable(cbs-cmap-squeeze "main.cpp")

target_link_libraries(cbs-cmap-squeeze
    CLI11::CLI11
    spdlog::spdlog
    cbs-common
)
