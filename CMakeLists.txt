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
    )
    
# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/map/*.cpp"
)

# Create library target (assuming this is meant to be a library)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

