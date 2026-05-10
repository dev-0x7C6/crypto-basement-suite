file(GLOB_RECURSE cbs-midnight-tracker-headers *.hpp)
file(GLOB_RECURSE cbs-midnight-tracker-sources *.cpp)

add_executable(cbs-midnight-tracker
    ${cbs-midnight-tracker-headers}
    ${cbs-midnight-tracker-sources}
)

target_compile_definitions(cbs-midnight-tracker PUBLIC -DSIMDJSON_STATIC_REFLECTION=1)

target_link_libraries(cbs-midnight-tracker
    thin-curl-requests
    CLI11::CLI11
    simdjson::simdjson
    spdlog::spdlog
    indicators::indicators
    cbs-common
)

install(TARGETS cbs-midnight-tracker RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
