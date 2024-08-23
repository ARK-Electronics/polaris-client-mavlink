cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(polaris-client-mavlink VERSION 0.9 LANGUAGES CXX)

add_compile_options(-Wall -Wunused)

# Download polaris repo (or make it a submodule)
include(FetchContent)
FetchContent_Declare(
    polaris
    GIT_REPOSITORY https://github.com/PointOneNav/polaris.git
    GIT_TAG v1.3.3)
set(POLARIS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(polaris)

# Assumes MAVSDK system wide install
list(APPEND CMAKE_PREFIX_PATH "/usr/local/MAVSDK/install")
find_package(MAVSDK REQUIRED)

include_directories(third_party/tomlplusplus/)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/PolarisClientMavlink.cpp)

target_link_libraries(${PROJECT_NAME}
    pthread
    polaris_cpp_client
    MAVSDK::mavsdk)
