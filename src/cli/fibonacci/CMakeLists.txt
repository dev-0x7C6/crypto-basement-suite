file(GLOB_RECURSE cbs-fib-experiments-headers *.hpp)
file(GLOB_RECURSE cbs-fib-experiments-sources *.cpp)

add_executable(cbs-fib-experiments
    ${cbs-fib-experiments-headers}
    ${cbs-fib-experiments-sources}
)

target_link_libraries(cbs-fib-experiments
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
)

install(TARGETS cbs-fib-experiments RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
 
