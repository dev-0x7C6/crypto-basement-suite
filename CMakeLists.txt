cmake_minimum_required(VERSION 3.25...3.31)

project(
  crypto-basement-suite
  VERSION 0.0.2
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

find_package(CLI11 REQUIRED)
find_package(benchmark REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory("src")
