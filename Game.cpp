#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

struct Unit {
    int gridX, gridY;
    float health = 100.0f;
    float attackPower = 20.0f;
    bool isSelected = false;
};

struct Camera {
    float x, y, zoom;
    Camera() : x(0.0f), y(0.0f), zoom(50.0f) {} // Pixels per grid unit
    void update(const SDL_Event& event) {
        if (event.type == SDL_MOUSEWHEEL) {
            zoom += event.wheel.y * 5.0f;
            if (zoom < 10.0f) zoom = 10.0f; // Limit zoom
        }
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        float speed = 5.0f / zoom; // Adjust for zoom level
        if (keys[SDL_SCANCODE_W]) y -= speed;
        if (keys[SDL_SCANCODE_S]) y += speed;
        if (keys[SDL_SCANCODE_A]) x -= speed;
        if (keys[SDL_SCANCODE_D]) x += speed;
    }
};

class Game {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::vector<Unit> units;
    Camera camera;
    const int GRID_SIZE = 10;
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

public:
    Game() {
        units.push_back({5, 5, 100.0f, 20.0f, false});
        units.push_back({6, 6, 100.0f, 20.0f, false});
    }

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            return false;
        }
        window = SDL_CreateWindow("Simple Strategy Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }
        return true;
    }

    void handleInput(SDL_Event& event) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            SDL_Quit();
            exit(0);
        }
        camera.update(event);

        // Unit selection
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            // Convert screen to world coords
            float worldX = (mouseX - WINDOW_WIDTH / 2.0f) / camera.zoom + camera.x;
            float worldY = (mouseY - WINDOW_HEIGHT / 2.0f) / camera.zoom + camera.y;
            int gridX = static_cast<int>(worldX + GRID_SIZE / 2.0f);
            int gridY = static_cast<int>(worldY + GRID_SIZE / 2.0f);
            for (auto& unit : units) {
                unit.isSelected = (unit.gridX == gridX && unit.gridY == gridY);
            }
        }

        // Combat
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
            Unit* selectedUnit = nullptr;
            for (auto& unit : units) {
                if (unit.isSelected) {
                    selectedUnit = &unit;
                    break;
                }
            }
            if (!selectedUnit) return;

            for (size_t i = 0; i < units.size(); ++i) {
                if (&units[i] == selectedUnit) continue;
                float dist = sqrt(pow(units[i].gridX - selectedUnit->gridX, 2) + pow(units[i].gridY - selectedUnit->gridY, 2));
                if (dist <= 2.0f) {
                    units[i].health -= selectedUnit->attackPower;
                    std::cout << "Unit at (" << units[i].gridX << ", " << units[i].gridY 
                              << ") takes " << selectedUnit->attackPower << " damage, health now " 
                              << units[i].health << std::endl;
                    if (units[i].health <= 0) {
                        std::cout << "Unit at (" << units[i].gridX << ", " << units[i].gridY 
                                  << ") destroyed!" << std::endl;
                        units.erase(units.begin() + i);
                        --i;
                    }
                }
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray background
        SDL_RenderClear(renderer);

        // Draw grid
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White lines
        for (int i = 0; i <= GRID_SIZE; ++i) {
            float worldPos = i - GRID_SIZE / 2.0f;
            int screenX = (worldPos - camera.x) * camera.zoom + WINDOW_WIDTH / 2;
            int screenY = (worldPos - camera.y) * camera.zoom + WINDOW_HEIGHT / 2;
            SDL_RenderDrawLine(renderer, screenX, 0, screenX, WINDOW_HEIGHT); // Vertical
            SDL_RenderDrawLine(renderer, 0, screenY, WINDOW_WIDTH, screenY);   // Horizontal
        }

        // Draw units
        for (const auto& unit : units) {
            int screenX = (unit.gridX - GRID_SIZE / 2.0f + 0.5f - camera.x) * camera.zoom + WINDOW_WIDTH / 2;
            int screenY = (unit.gridY - GRID_SIZE / 2.0f + 0.5f - camera.y) * camera.zoom + WINDOW_HEIGHT / 2;
            SDL_Rect rect = {screenX - 20, screenY - 20, 40, 40}; // 40x40 pixel unit
            if (unit.isSelected) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
            }
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    void run() {
        SDL_Event event;
        while (true) {
            while (SDL_PollEvent(&event)) {
                handleInput(event);
            }
            render();
        }
    }

    void cleanup() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};