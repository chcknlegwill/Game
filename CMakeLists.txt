#avoids legacy fallbacks - defaults to the GLVND libraries
set(OpenGL_GL_PREFERENCE GLVND)


cmake_minimum_required(VERSION 3.10)
project(StrategyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find SDL2_image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

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

# Find ZLIB
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# Define the executable
add_executable(StrategyGame main.cpp ModelLoader.cpp Renderer.cpp GameLogic.cpp Camera.cpp)

# Link libraries
target_link_libraries(StrategyGame PRIVATE 
    ${SDL2_LIBRARIES} 
    SDL2_image::SDL2_image  # Use the target provided by SDL2_image
    GLEW::GLEW 
    OpenGL::GL 
    assimp::assimp 
    ${ZLIB_LIBRARIES}
)

#removed hard-coded paths as also developing on linux
