cmake_minimum_required(VERSION 3.10)
project(StrategyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Homebrew paths for macOS
set(CMAKE_PREFIX_PATH "/opt/homebrew;/opt/homebrew/Cellar/assimp/5.4.3" ${CMAKE_PREFIX_PATH})

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Find OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# Find GLM
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# Find Assimp
find_package(assimp REQUIRED)
include_directories(${assimp_INCLUDE_DIRS})

# Manually set zlib
set(ZLIB_LIBRARY "/opt/homebrew/Cellar/zlib/1.3.1/lib/libz.dylib")
set(ZLIB_INCLUDE_DIR "/opt/homebrew/Cellar/zlib/1.3.1/include")
include_directories(${ZLIB_INCLUDE_DIR})

# Define the executable
add_executable(StrategyGame main.cpp Camera.cpp)

# Link libraries
target_link_libraries(StrategyGame PRIVATE ${SDL2_LIBRARIES} GLEW::GLEW OpenGL::GL assimp::assimp ${ZLIB_LIBRARY})