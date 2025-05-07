#include <iostream>

//local files
#include "Renderer.h"
#include "GameLogic.h"
#include "ModelLoader.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting 3D Strategy Game..." << std::endl;

    if (!initSDLAndOpenGL(810, 610)) {
        std::cerr << "Failed to initialize SDL and OpenGL" << std::endl;
        return 1;
    }

    GLuint shaderProgram = setupShaders();
    if (!shaderProgram) {
        std::cerr << "Failed to setup shaders" << std::endl;
        cleanupSDLAndOpenGL();
        return 1;
    }

    std::vector<Unit3D> units = loadModels("../Models/Aircraft_Carrier(low_poly).obj");
    if (units.empty()) {
        std::cerr << "Failed to load models" << std::endl;
        glDeleteProgram(shaderProgram);
        cleanupSDLAndOpenGL();
        return 1;
    }

    if (!setupRendering(shaderProgram, units)) {
        std::cerr << "Failed to setup rendering" << std::endl;
        glDeleteProgram(shaderProgram);
        cleanupSDLAndOpenGL();
        return 1;
    }

    runGameLoop(shaderProgram, units);

    glDeleteProgram(shaderProgram);
    cleanupSDLAndOpenGL();
    std::cout << "Cleaning up..." << std::endl;
    return 0;
}