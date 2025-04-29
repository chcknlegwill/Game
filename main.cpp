#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Starting program (initialising variables and so on)..." << std::endl;

    int SCREEN_WIDTH = 810;
    int SCREEN_HEIGHT = 610;
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

    const int grid_size = 10; //not px, but numb of cells you want
    const int GRID_ROWS = grid_size, GRID_COLS = grid_size; //changes the size of the grid, no matter the window size
    const int PADDING = 10; //in px, padding around the edges 

    //below makes sure that the grid is equal no matter window size (height or width) 
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    float CELL_WIDTH = (window_width - 2 * PADDING)  / GRID_COLS; 
    float CELL_HEIGHT = (window_height -2 * PADDING) / GRID_ROWS;
    std::cout << "Grid: " << GRID_ROWS << "x" << GRID_COLS << "| Cell(px): " << CELL_WIDTH << "x" << CELL_HEIGHT << std::endl;

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

        //vertical lines
        for (int i = 0; i <= GRID_COLS; i++) {
            int x = PADDING + i * CELL_WIDTH;
            SDL_RenderDrawLine(renderer, x, PADDING, x, PADDING + GRID_ROWS * CELL_HEIGHT);
        }

        //horizontal lines
        for (int i = 0; i <= GRID_ROWS; i++) {
          int y = PADDING + i * CELL_HEIGHT;
          SDL_RenderDrawLine(renderer, PADDING, y, PADDING + GRID_COLS * CELL_WIDTH, y);
        }

        SDL_RenderPresent(renderer);

      }

    std::cout << "Cleaning up..." << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
