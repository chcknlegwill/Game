#include "GameLogic.h"
#include <glm/glm.hpp>
#include <iostream>
#include "ModelLoader.h"

void handleCombat(const SDL_Event& event, std::vector<Unit3D>& units) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && !units.empty()) {
        for (size_t i = 1; i < units.size(); ++i) { // Start at 1 to skip carrier
            float dist = glm::distance(units[0].position, units[i].position);
            if (dist <= 2.0f) {
                std::cout << "Carrier attacks plane at gridX=" << units[i].gridX << ", gridY=" << units[i].gridY << "!" << std::endl;
                units.erase(units.begin() + i);
                --i; // Adjust index after erase
            }
        }
    }
}