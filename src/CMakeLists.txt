add_subdirectory(indicators)
add_subdirectory(model)

add_executable(cbs "main.cpp")

target_link_libraries(cbs
    cbs-indicators
    cbs-model
)
