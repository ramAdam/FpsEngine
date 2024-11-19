# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(BSPEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/map
    ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/physics
    ${CMAKE_CURRENT_SOURCE_DIR}/include/game
)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "include/*.cpp"
)

# Add raylib
find_package(raylib REQUIRED)
find_package(Bullet REQUIRED)

# Create the target BEFORE linking
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Now we can link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
    raylib
    ${BULLET_LIBRARIES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${BULLET_INCLUDE_DIRS}
)

