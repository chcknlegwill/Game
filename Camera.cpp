#include "Camera.hpp"
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float posX, float posY, float posZ, float initialYaw, float initialPitch)
    : cameraPos(posX, posY, posZ), yaw(initialYaw), pitch(initialPitch), 
      cameraSpeed(0.15f), scrollSpeed(0.5f), panSpeed(0.01f), 
      rotating(false), panning(false) {
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Camera::update(SDL_Event& event) {
    glm::vec3 direction(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch))
    );
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 0, 1)));
    glm::vec3 moveDirection = glm::normalize(glm::vec3(direction.x, direction.y, 0.0f));

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_r) {
            reset();
        }
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rotating = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            panning = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
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
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }
    if (event.type == SDL_MOUSEMOTION) {
        if (rotating) {
            yaw -= event.motion.xrel * 0.02f;
            pitch -= event.motion.yrel * 0.02f;
            pitch = glm::clamp(pitch, -89.0f, 89.0f);
        }
        if (panning) {
            cameraPos -= right * (event.motion.xrel * panSpeed);
            cameraPos += glm::vec3(0, 0, 1) * (event.motion.yrel * panSpeed);
        }
    }
    if (event.type == SDL_MOUSEWHEEL) {
        cameraPos.z -= event.wheel.y * scrollSpeed;
    }

    const Uint8* keystate = SDL_GetKeyboardState(nullptr);

    if (keystate[SDL_SCANCODE_W]) cameraPos += cameraSpeed * moveDirection;
    if (keystate[SDL_SCANCODE_S]) cameraPos -= cameraSpeed * moveDirection;
    if (keystate[SDL_SCANCODE_A]) cameraPos -= cameraSpeed * right;
    if (keystate[SDL_SCANCODE_D]) cameraPos += cameraSpeed * right;
    if (keystate[SDL_SCANCODE_SPACE]) cameraPos += cameraSpeed * glm::vec3(0, 0, 1);
    if (keystate[SDL_SCANCODE_LSHIFT]) cameraPos -= cameraSpeed * glm::vec3(0, 0, 1);

    if (keystate[SDL_SCANCODE_LEFT]) yaw += 1.0f;
    if (keystate[SDL_SCANCODE_RIGHT]) yaw -= 1.0f;
    if (keystate[SDL_SCANCODE_UP]) pitch += 1.0f;
    if (keystate[SDL_SCANCODE_DOWN]) pitch -= 1.0f;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    cameraPos.x = glm::clamp(cameraPos.x, -50.0f, 50.0f); // Increased for skybox
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
    panning = false;
    SDL_SetRelativeMouseMode(SDL_FALSE);
}