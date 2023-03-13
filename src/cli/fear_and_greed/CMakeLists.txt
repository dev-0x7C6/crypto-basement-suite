add_executable(cbs-fear-and-greed "main.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(curlpp REQUIRED)
pkg_check_modules(curl REQUIRED)


target_link_libraries(cbs-fear-and-greed
    cbs-networking
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    curlpp
    curl
)
