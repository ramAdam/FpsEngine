# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(qodot)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/map
    ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer
)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "include/*.cpp"
)

# Add raylib
find_package(raylib REQUIRED)

# Create the target BEFORE linking
add_executable(qodot ${SOURCES})

# Now we can link libraries
target_link_libraries(qodot PRIVATE raylib)

