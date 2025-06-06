#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class Camera {
public:
    Camera(float posX, float posY, float posZ, float yaw, float pitch);
    void update(SDL_Event& event, float deltaTime);
    glm::mat4 getViewMatrix();
    glm::vec3 getPosition() const { return cameraPos; }
    void reset();

private:
    glm::vec3 cameraPos;
    float yaw, pitch;
    float moveSpeed, mouseSensitivity, zoomSpeed;
    bool rotating;
};