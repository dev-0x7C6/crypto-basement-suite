file(GLOB_RECURSE cbs-portfolio-headers *.hpp)
file(GLOB_RECURSE cbs-portfolio-sources *.cpp)

add_executable(cbs-portfolio
    ${cbs-portfolio-headers}
    ${cbs-portfolio-sources}
)

target_link_libraries(cbs-portfolio
    thin-curl-requests
    libcoingecko-v3
    libblockfrost-v0
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    cbs-common
)

install(TARGETS cbs-portfolio RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
