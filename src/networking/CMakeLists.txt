add_library(thin-curl-requests SHARED
    requests.cpp
    public/rest/requests.hpp
)

target_include_directories(thin-curl-requests PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/public
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(thin-curl-requests PRIVATE CURL::libcurl)
