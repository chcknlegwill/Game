#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <vector>
#include "Unit3D.h"

extern std::vector<float> gridVertices; // External as it's tied to the grid on game setup

bool initSDLAndOpenGL(int width, int height);
GLuint setupShaders();
bool setupRendering(GLuint shaderProgram, std::vector<Unit3D>& units);
void runGameLoop(GLuint shaderProgram, std::vector<Unit3D>& units);
void cleanupSDLAndOpenGL();

// Function from GameLogic to check island cells
bool isIslandCell(int gridX, int gridY);

#endif