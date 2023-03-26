cmake_minimum_required(VERSION 3.18...3.26)

project(
  crypto-basement-suite
  VERSION 0.0.2
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(CLI11 REQUIRED)
find_package(benchmark REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(curlpp REQUIRED curlpp)

add_subdirectory("src")
