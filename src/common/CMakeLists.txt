add_library(cbs-common INTERFACE "common.hpp" "csv.hpp")

target_include_directories(cbs-common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
