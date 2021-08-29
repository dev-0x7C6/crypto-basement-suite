add_subdirectory(model)

add_executable(cbs "main.cpp")
target_link_libraries(cbs cbs-model)
