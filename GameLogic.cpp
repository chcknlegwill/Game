#include "GameLogic.h"
#include <glm/glm.hpp>
#include <iostream>
#include "ModelLoader.h"

bool isIslandCell(int gridX, int gridY) {
    // Island 1 at (-10, -10) to (-8, -8)
    if (gridX >= 5 && gridX <= 7 && gridY >= 5 && gridY <= 7) {
        return true;
    }
    // Island 2 at (10, 10) to (12, 12)
    if (gridX >= 25 && gridX <= 27 && gridY >= 25 && gridY <= 27) {
        return true;
    }
    return false;
}

void handleCombat(const SDL_Event& event, std::vector<Unit3D>& units) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && !units.empty()) {
        for (size_t i = 1; i < units.size(); ++i) {
            float dist = glm::distance(units[0].position, units[i].position);
            if (dist <= 2.0f) {
                std::cout << "Carrier attacks plane at gridX=" << units[i].gridX << ", gridY=" << units[i].gridY << "!" << std::endl;
                units.erase(units.begin() + i);
                --i;
            }
        }
    }
}