add_executable(cbs-fear-and-greed "main.cpp")

target_link_libraries(cbs-fear-and-greed
    thin-curl-requests
    spdlog::spdlog
)
