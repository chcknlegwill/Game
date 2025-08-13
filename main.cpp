#include <iostream>
#include "Game.cpp" // Include implementation directly

int main(int argc, char* argv[]) {
    std::cout << "Starting Simple Strategy Game..." << std::endl;

    Game game;
    if (!game.init()) {
        return 1;
    }

    game.run();
    game.cleanup();
    return 0;
}