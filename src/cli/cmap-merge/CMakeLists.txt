add_executable(cbs-cmap-merge "main.cpp")

target_link_libraries(cbs-cmap-merge
    CLI11::CLI11
    spdlog::spdlog
    cbs-common
)
