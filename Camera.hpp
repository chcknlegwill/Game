#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>

class Camera {
private:
    glm::vec3 cameraPos;
    float yaw;
    float pitch;
    float cameraSpeed;
    float scrollSpeed;
    float panSpeed;
    bool rotating; // Right-click rotation
    bool panning;  // Middle-click panning

public:
    Camera(float posX, float posY, float posZ, float yaw, float pitch);
    void update(SDL_Event& event);
    glm::mat4 getViewMatrix();
    void reset();
};

#endif