#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Camera.hpp"
#include <vector>

static SDL_Window* window = nullptr;
static SDL_GLContext glContext = nullptr;
static GLuint shaderProgram = 0, gridVao = 0, gridVbo = 0;
static std::vector<float> gridVertices;
static int windowWidth = 810, windowHeight = 610;

std::string getResourcePath(const std::string& relativePath) {
#ifdef RESOURCE_PATH
    return std::string(RESOURCE_PATH) + relativePath;
#else
    return "assets/" + relativePath; // Fallback
#endif
}

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
      std::cerr << "Failed to open " << path << std::endl;
      return "";
  }
  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  //std::cout << "Raw content length: " << content.length() << " bytes\n";
  //std::cout << "Raw content:\n" << content << "\n";
  if (content.empty()) {
      std::cerr << "File " << path << " is empty or unreadable\n";
      return "";
  }
  return content;
}

GLuint loadShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
  std::string vertexSource = readFile(getResourcePath(vertexPath));
  std::string fragmentSource = readFile(getResourcePath(fragmentPath));
  if (vertexSource.empty() || fragmentSource.empty()) return 0;

  //std::cout << "Vertex Shader Source:\n" << vertexSource << "\n";
  //std::cout << "Fragment Shader Source:\n" << fragmentSource << "\n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const char* vSrc = vertexSource.c_str();
  glShaderSource(vertexShader, 1, &vSrc, nullptr);
  glCompileShader(vertexShader);
  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
      GLchar infoLog[512];
      glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
      std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
      glDeleteShader(vertexShader);
      return 0;
  }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fSrc, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glBindAttribLocation(program, 0, "aPos");
glLinkProgram(program); // Relink after binding
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

bool isRestrictedCell(int gridX, int gridY) {
    // Define restricted areas (like islands in your original code)
    if (gridX >= 5 && gridX <= 7 && gridY >= 5 && gridY <= 7) return true; // Restricted area 1
    if (gridX >= 25 && gridX <= 27 && gridY >= 25 && gridY <= 27) return true; // Restricted area 2
    return false;
}

glm::vec2 worldToGrid(const glm::vec3& worldPos) {
    return glm::vec2(std::floor(worldPos.x + 15.0f), std::floor(worldPos.y + 15.0f));
}

bool initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Game Engine Base", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                             windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext || glewInit() != GLEW_OK) {
        std::cerr << "GL context/GLEW init failed" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!glContext || glewInit() != GLEW_OK) {
      std::cerr << "GL context/GLEW init failed" << std::endl;
      SDL_DestroyWindow(window);
      SDL_Quit();
      return false;
  }

  // Print OpenGL version
  const GLubyte* glVersion = glGetString(GL_VERSION);
  const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "OpenGL Version: " << (glVersion ? reinterpret_cast<const char*>(glVersion) : "Unknown") << std::endl;
  std::cout << "GLSL Version: " << (glslVersion ? reinterpret_cast<const char*>(glslVersion) : "Unknown") << std::endl;

  if (!GLEW_VERSION_3_3) {
      std::cerr << "OpenGL 3.3 not supported by your system!" << std::endl;
      SDL_GL_DeleteContext(glContext);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return false;
  }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    shaderProgram = loadShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to load shaders" << std::endl;
        return false;
    }

    // Grid setup (30x30 grid, from -15 to 15)
    for (float x = -15.0f; x <= 15.0f; x += 1.0f) {
        gridVertices.push_back(x); gridVertices.push_back(-15.0f); gridVertices.push_back(0.01f);
        gridVertices.push_back(x); gridVertices.push_back(15.0f); gridVertices.push_back(0.01f);
    }
    for (float y = -15.0f; y <= 15.0f; y += 1.0f) {
        gridVertices.push_back(-15.0f); gridVertices.push_back(y); gridVertices.push_back(0.01f);
        gridVertices.push_back(15.0f); gridVertices.push_back(y); gridVertices.push_back(0.01f);
    }
    glGenVertexArrays(1, &gridVao);
    glGenBuffers(1, &gridVbo);
    glBindVertexArray(gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return true;
}

void cleanup() {
    if (gridVao) glDeleteVertexArrays(1, &gridVao);
    if (gridVbo) glDeleteBuffers(1, &gridVbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (glContext) SDL_GL_DeleteContext(glContext);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    std::cout << "Starting Game Engine Base..." << std::endl;

    if (!initialize()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }

    Camera camera(0.0f, -15.0f, 15.0f, 0.0f, -45.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 200.0f);

    bool running = true;
    Uint32 lastTime = SDL_GetTicks64();
    SDL_Event event;
    while (running) {
        Uint32 currentTime = SDL_GetTicks64();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            camera.update(event, deltaTime);

            // Test constraints with raycasting (click to check grid position)
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float x = (2.0f * mouseX) / windowWidth - 1.0f;
                float y = 1.0f - (2.0f * mouseY) / windowHeight;
                glm::vec4 rayClip = glm::vec4(x, y, -1.0, 1.0);
                glm::vec4 rayEye = glm::inverse(projection) * rayClip;
                rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
                glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.getViewMatrix()) * rayEye);
                rayWorld = glm::normalize(rayWorld);
                glm::vec3 rayOrigin = camera.getPosition();

                float t = -rayOrigin.z / rayWorld.z;
                glm::vec3 intersection = rayOrigin + t * rayWorld;
                glm::vec2 gridPos = worldToGrid(intersection);
                int gridX = static_cast<int>(gridPos.x);
                int gridY = static_cast<int>(gridPos.y);

                if (isRestrictedCell(gridX, gridY)) {
                    std::cout << "Clicked on restricted cell at gridX=" << gridX << ", gridY=" << gridY << std::endl;
                } else {
                    std::cout << "Clicked on free cell at gridX=" << gridX << ", gridY=" << gridY << std::endl;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glm::mat4 view = camera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

        // Render grid
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(gridVao);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

        SDL_GL_SwapWindow(window);
    }

    cleanup();
    std::cout << "Cleaning up..." << std::endl;
    return 0;
}