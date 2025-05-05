#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <vector>
#include <string>
#include <GL/glew.h> //for GLuint variable
#include <glm/glm.hpp>

struct Unit3D {
  int gridX, gridY;
  glm::vec3 position;
  GLuint vao, vbo, ebo;
  int vertexCount;
  int indexCount;
};

std::vector<Unit3D> loadModels(const std::string& path);

#endif