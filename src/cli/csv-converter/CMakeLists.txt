add_executable(cbs-csv-converter "main.cpp")

target_link_libraries(cbs-csv-converter
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    cbs-common
)
