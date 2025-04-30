#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "Camera.hpp"

struct Unit { int gridX, gridY; }; // To be adapted for 3D later

int main(int argc, char* argv[]) {
    std::cout << "Starting 3D Strategy Game..." << std::endl;

    int SCREEN_WIDTH = 810;
    int SCREEN_HEIGHT = 610;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set up OpenGL 3.3 core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("3D Strategy Game", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "GL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "OpenGL initialized: " << glGetString(GL_VERSION) << std::endl;

    // Set viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    // Fragment shader with color uniform
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main() {
            FragColor = color;
        }
    )";

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Ocean quad (15x15 units)
    float oceanVertices[] = {
        -7.5f, -7.5f, 0.0f, // Bottom-left
         7.5f, -7.5f, 0.0f, // Bottom-right
         7.5f,  7.5f, 0.0f, // Top-right
         7.5f,  7.5f, 0.0f, // Top-right
        -7.5f,  7.5f, 0.0f, // Top-left
        -7.5f, -7.5f, 0.0f  // Bottom-left
    };
    GLuint oceanVao, oceanVbo;
    glGenVertexArrays(1, &oceanVao);
    glGenBuffers(1, &oceanVbo);
    glBindVertexArray(oceanVao);
    glBindBuffer(GL_ARRAY_BUFFER, oceanVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oceanVertices), oceanVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Grid lines (15x15 grid, 16 vertical + 16 horizontal lines)
    std::vector<float> gridVertices;
    // Vertical lines: x = -7.5 to 7.5, every 1 unit
    for (float x = -7.5f; x <= 7.5f; x += 1.0f) {
        gridVertices.push_back(x); gridVertices.push_back(-7.5f); gridVertices.push_back(0.01f);
        gridVertices.push_back(x); gridVertices.push_back( 7.5f); gridVertices.push_back(0.01f);
    }
    // Horizontal lines: y = -7.5 to 7.5, every 1 unit
    for (float y = -7.5f; y <= 7.5f; y += 1.0f) {
        gridVertices.push_back(-7.5f); gridVertices.push_back(y); gridVertices.push_back(0.01f);
        gridVertices.push_back( 7.5f); gridVertices.push_back(y); gridVertices.push_back(0.01f);
    }
    GLuint gridVao, gridVbo;
    glGenVertexArrays(1, &gridVao);
    glGenBuffers(1, &gridVbo);
    glBindVertexArray(gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Gray background

    // Camera setup
    Camera camera(0.0f, -10.0f, 10.0f, 0.0f, -45.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f); // Identity for ocean and grid

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            camera.update(event);
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Render ocean quad (blue)
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f); // Blue
        glBindVertexArray(oceanVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render grid lines (white)
        glUniform4f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 1.0f, 1.0f); // White
        glLineWidth(1.0f);
        glBindVertexArray(gridVao);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &oceanVao);
    glDeleteBuffers(1, &oceanVbo);
    glDeleteVertexArrays(1, &gridVao);
    glDeleteBuffers(1, &gridVbo);
    glDeleteProgram(shaderProgram);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Cleaning up..." << std::endl;
    return 0;
}