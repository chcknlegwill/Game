#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Starting program (initialising variables and so on)..." << std::endl;

    int SCREEN_WIDTH = 800;
    int SCREEN_HEIGHT = 600;
    //screen variables need to be here for window creation (SDL_CreateWindow)

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL initialized." << std::endl;

    SDL_Window* window = SDL_CreateWindow("Strategy Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "Window created." << std::endl;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "Renderer created." << std::endl;

    //main code starts here

    int GRID_SIZE = 10;

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); //render a black screen
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //set render colour for white lines

        for (int x =0; x <= SCREEN_WIDTH; x += GRID_SIZE) {
          SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
        } //horizontal lines (x-axis)

        for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE) {
          SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
        } //vertical lines (y-axis)

        SDL_RenderPresent(renderer);

      }

    std::cout << "Cleaning up..." << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
