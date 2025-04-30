#include "Camera.hpp"
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float posX, float posY, float posZ, float initialYaw, float initialPitch)
    : cameraPos(posX, posY, posZ), yaw(initialYaw), pitch(initialPitch), 
      cameraSpeed(0.15f), scrollSpeed(0.5f), panSpeed(0.01f), 
      rotating(false), panning(false) {
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Camera::update(SDL_Event& event) {
    // Compute direction and right vectors
    glm::vec3 direction(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch))
    );
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 0, 1)));
    // Normalize direction for WASD movement (horizontal plane)
    glm::vec3 moveDirection = glm::normalize(glm::vec3(direction.x, direction.y, 0.0f));

    // Handle discrete events
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_r) {
            reset();
        }
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rotating = true;
            SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            panning = true;
            SDL_SetRelativeMouseMode(SDL_TRUE); // Lock cursor
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rotating = false;
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            panning = false;
        }
        if (!rotating && !panning) {
            SDL_SetRelativeMouseMode(SDL_FALSE); // Unlock cursor
        }
    }
    if (event.type == SDL_MOUSEMOTION) {
        if (rotating) {
            yaw -= event.motion.xrel * 0.02f; // Low sensitivity for trackpad
            pitch -= event.motion.yrel * 0.02f;
            pitch = glm::clamp(pitch, -89.0f, 89.0f);
        }
        if (panning) {
            cameraPos -= right * (event.motion.xrel * panSpeed);
            cameraPos += glm::vec3(0, 0, 1) * (event.motion.yrel * panSpeed);
        }
    }
    if (event.type == SDL_MOUSEWHEEL) {
        cameraPos.z -= event.wheel.y * scrollSpeed; // Scroll up to zoom in, down to zoom out
    }

    // Handle continuous input
    const Uint8* keystate = SDL_GetKeyboardState(nullptr);

    // Movement (WASD, space, shift) with normalized direction
    if (keystate[SDL_SCANCODE_W]) cameraPos += cameraSpeed * moveDirection;
    if (keystate[SDL_SCANCODE_S]) cameraPos -= cameraSpeed * moveDirection;
    if (keystate[SDL_SCANCODE_A]) cameraPos -= cameraSpeed * right;
    if (keystate[SDL_SCANCODE_D]) cameraPos += cameraSpeed * right;
    if (keystate[SDL_SCANCODE_SPACE]) cameraPos += cameraSpeed * glm::vec3(0, 0, 1);
    if (keystate[SDL_SCANCODE_LSHIFT]) cameraPos -= cameraSpeed * glm::vec3(0, 0, 1);

    // Rotation (arrow keys)
    if (keystate[SDL_SCANCODE_LEFT]) yaw += 1.0f; // Rotate left
    if (keystate[SDL_SCANCODE_RIGHT]) yaw -= 1.0f; // Rotate right
    if (keystate[SDL_SCANCODE_UP]) pitch += 1.0f; // Look up
    if (keystate[SDL_SCANCODE_DOWN]) pitch -= 1.0f; // Look down
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Enforce bounds to keep quad visible
    cameraPos.x = glm::clamp(cameraPos.x, -20.0f, 20.0f);
    cameraPos.y = glm::clamp(cameraPos.y, -20.0f, 20.0f);
    cameraPos.z = glm::clamp(cameraPos.z, 1.0f, 20.0f);
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
    cameraPos = glm::vec3(0.0f, -10.0f, 10.0f);
    yaw = 0.0f;
    pitch = -45.0f;
    rotating = false;
    panning = false;
    SDL_SetRelativeMouseMode(SDL_FALSE);
}