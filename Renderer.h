#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <GL/glew.h>
#include "GameLogic.h"

extern std::vector<float> gridVertices;

bool initSDLAndOpenGL(int width, int height);
GLuint setupShaders();
bool setupRendering(GLuint shaderProgram, std::vector<Unit3D>& units);
void runGameLoop(GLuint shaderProgram, std::vector<Unit3D>& units);
void cleanupSDLAndOpenGL(std::vector<Unit3D>& units); // Matches signature