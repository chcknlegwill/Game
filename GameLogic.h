#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <SDL2/SDL.h>
#include <vector>
#include "ModelLoader.h"

void handleCombat(const SDL_Event& event, std::vector<Unit3D>& units);

#endif