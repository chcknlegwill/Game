#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <GL/glew.h>
#include "ModelLoader.h"

bool initSDLAndOpenGL(int width, int height);
GLuint setupShaders();
bool setupRendering(GLuint shaderProgram, std::vector<Unit3D>& units);
void runGameLoop(GLuint shaderProgram, std::vector<Unit3D>& units);
void cleanupSDLAndOpenGL();

extern std::vector<float> gridVertices; //external as its tied to the grid on game setup


#endif