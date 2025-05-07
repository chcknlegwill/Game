#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h> // Added for GLuint

struct Unit3D {
    GLuint vao, vbo, ebo;
    int vertexCount, indexCount;
    int gridX, gridY;
    glm::vec3 position;
    float health = 100.0f;
    float attackPower = 20.0f;
    bool isSelected = false;
};

bool isIslandCell(int gridX, int gridY);
void selectUnit(const SDL_Event& event, std::vector<Unit3D>& units, int gridX, int gridY);
void handleCombat(const SDL_Event& event, std::vector<Unit3D>& units);