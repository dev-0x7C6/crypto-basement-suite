set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Core Charts)

file(GLOB_RECURSE cbs-portfolio-headers *.hpp)
file(GLOB_RECURSE cbs-portfolio-sources *.cpp)
file(GLOB_RECURSE cbs-portfolio-uis *.ui)

add_executable(cbs-portfolio
    ${cbs-portfolio-headers}
    ${cbs-portfolio-sources}
    ${cbs-portfolio-uis}
)

target_link_libraries(cbs-portfolio
    thin-curl-requests
    libcoingecko-v3
    libblockfrost-v0
    CLI11::CLI11
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    cbs-common
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
)

install(TARGETS cbs-portfolio RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
