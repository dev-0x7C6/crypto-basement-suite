add_executable(cbs-csv-converter "main.cpp")

target_link_libraries(cbs-csv-converter
    CLI11::CLI11
    spdlog::spdlog
    cbs-common
)
