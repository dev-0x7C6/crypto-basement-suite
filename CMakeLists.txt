cmake_minimum_required(VERSION 3.20...3.26)

project(
  crypto-basement-suite
  VERSION 0.0.2
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_definitions("-D__cpp_concepts=202002L")

find_package(CLI11 REQUIRED)
find_package(benchmark REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Curl++ REQUIRED IMPORTED_TARGET curlpp)


add_subdirectory("src")
