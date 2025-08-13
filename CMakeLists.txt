#avoids legacy fallbacks - defaults to the GLVND libraries
set(OpenGL_GL_PREFERENCE GLVND)


cmake_minimum_required(VERSION 3.10)
project(StrategyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Define the executable
add_executable(StrategyGame main.cpp Game.cpp)

# Link libraries
target_link_libraries(StrategyGame PRIVATE ${SDL2_LIBRARIES})

#removed hard-coded paths as also developing on linux
