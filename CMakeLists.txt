cmake_minimum_required(VERSION 3.10)
project(StrategyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(assimp REQUIRED)
include_directories(${assimp_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

add_executable(StrategyGame main.cpp ModelLoader.cpp Renderer.cpp GameLogic.cpp Camera.cpp)

target_link_libraries(StrategyGame PRIVATE ${SDL2_LIBRARIES} GLEW::GLEW OpenGL::GL assimp::assimp ${ZLIB_LIBRARIES})


#removed hard-coded paths as also developing on linux
