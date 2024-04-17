cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(polaris-rtk-client VERSION 0.9 LANGUAGES CXX)

# Fetch polaris repo
include(FetchContent)
FetchContent_Declare(
    polaris
    GIT_REPOSITORY https://github.com/PointOneNav/polaris.git
    GIT_TAG v1.3.3
)
set(POLARIS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(polaris)

# Assumes MAVSDK system wide install
list(APPEND CMAKE_PREFIX_PATH "/usr/local/MAVSDK/install")
find_package(MAVSDK REQUIRED)

# Add header only libraries
include_directories(third_party/tomlplusplus/)

# Create executable
add_executable(polaris-rtk-client
    src/main.cpp
    src/PolarisRTKClient.cpp
)

target_link_libraries(polaris-rtk-client
    pthread
    polaris_cpp_client
    MAVSDK::mavsdk
)

# Binary installation
install(TARGETS polaris-rtk-client RUNTIME DESTINATION bin)
