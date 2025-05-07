#include "GameLogic.h"
#include <glm/glm.hpp>
#include <iostream>
#include "ModelLoader.h"

bool isIslandCell(int gridX, int gridY) {
    if (gridX >= 5 && gridX <= 7 && gridY >= 5 && gridY <= 7) {
        return true;
    }
    if (gridX >= 25 && gridX <= 27 && gridY >= 25 && gridY <= 27) {
        return true;
    }
    return false;
}

void selectUnit(const SDL_Event& event, std::vector<Unit3D>& units, int gridX, int gridY) {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) return;
    for (auto& unit : units) {
        unit.isSelected = (unit.gridX == gridX && unit.gridY == gridY);
    }
}

void handleCombat(const SDL_Event& event, std::vector<Unit3D>& units) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
        Unit3D* selectedUnit = nullptr;
        for (auto& unit : units) {
            if (unit.isSelected) {
                selectedUnit = &unit;
                break;
            }
        }
        if (!selectedUnit) return;

        for (size_t i = 0; i < units.size(); ++i) {
            if (&units[i] == selectedUnit) continue;
            float dist = glm::distance(selectedUnit->position, units[i].position);
            if (dist <= 2.0f) {
                units[i].health -= selectedUnit->attackPower;
                std::cout << "Unit at gridX=" << units[i].gridX << ", gridY=" << units[i].gridY 
                          << " takes " << selectedUnit->attackPower << " damage, health now " 
                          << units[i].health << std::endl;
                if (units[i].health <= 0) {
                    std::cout << "Unit at gridX=" << units[i].gridX << ", gridY=" << units[i].gridY 
                              << " destroyed!" << std::endl;
                    units.erase(units.begin() + i);
                    --i;
                }
            }
        }
    }
}
