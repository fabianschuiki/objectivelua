cmake_minimum_required(VERSION 2.6)
project(ObjectiveLua)

# Add ObjectiveLua's include directory
include_directories(../include/)

# Find the required libraries.
find_package(Lua51)
include_directories(${LUA_INCLUDE_DIR})

# Debug executable to develop the whole project.
add_executable(debug main.cpp)
target_link_libraries(debug ${LUA_LIBRARIES})
