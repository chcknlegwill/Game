#include "Camera.hpp" // Use quotes for relative include
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float posX, float posY, float posZ, float initialYaw, float initialPitch)
    : cameraPos(posX, posY, posZ), yaw(initialYaw), pitch(initialPitch),
      moveSpeed(5.0f), mouseSensitivity(0.1f), zoomSpeed(2.0f),
      rotating(false) {
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Camera::update(SDL_Event& event, float deltaTime) {
    // Compute direction and right vector
    glm::vec3 direction(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch))
    );
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 0, 1)));
    glm::vec3 moveDirection = glm::normalize(glm::vec3(direction.x, direction.y, 0.0f));

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
        reset();
    }
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        rotating = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
        rotating = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    if (event.type == SDL_MOUSEMOTION && rotating) {
        yaw -= event.motion.xrel * mouseSensitivity * deltaTime * 60.0f;
        pitch -= event.motion.yrel * mouseSensitivity * deltaTime * 60.0f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }
    if (event.type == SDL_MOUSEWHEEL) {
        cameraPos.z -= event.wheel.y * zoomSpeed * deltaTime * 60.0f;
    }

    // Keyboard movement
    const Uint8* keystate = SDL_GetKeyboardState(nullptr);
    if (keystate[SDL_SCANCODE_W]) cameraPos += moveSpeed * moveDirection * deltaTime;
    if (keystate[SDL_SCANCODE_S]) cameraPos -= moveSpeed * moveDirection * deltaTime;
    if (keystate[SDL_SCANCODE_A]) cameraPos -= moveSpeed * right * deltaTime;
    if (keystate[SDL_SCANCODE_D]) cameraPos += moveSpeed * right * deltaTime;

    // Constraints
    cameraPos.x = glm::clamp(cameraPos.x, -50.0f, 50.0f);
    cameraPos.y = glm::clamp(cameraPos.y, -50.0f, 50.0f);
    cameraPos.z = glm::clamp(cameraPos.z, 1.0f, 50.0f);
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 direction(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch))
    );
    return glm::lookAt(cameraPos, cameraPos + direction, glm::vec3(0, 0, 1));
}

void Camera::reset() {
    cameraPos = glm::vec3(0.0f, -15.0f, 15.0f);
    yaw = 0.0f;
    pitch = -45.0f;
    rotating = false;
    SDL_SetRelativeMouseMode(SDL_FALSE);
}