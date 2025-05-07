#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Camera.hpp"
#include "GameLogic.h"
#include "Renderer.h"

// Define global gridVertices (matches extern in Renderer.h)
std::vector<float> gridVertices;

static SDL_Window* window = nullptr;
static SDL_GLContext glContext = nullptr;
static GLuint oceanVao, oceanVbo, gridVao, gridVbo, islandVao, islandVbo, islandEbo;
static GLuint cloudVao, cloudVbo, skyboxVao, skyboxVbo;
static Camera* camera = nullptr;
static glm::mat4 projection;
static GLuint carrierTexture, islandTexture, cloudTexture;
static float cloudTime = 0.0f;

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool initSDLAndOpenGL(int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL/Image init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("3D Strategy Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext || glewInit() != GLEW_OK) {
        std::cerr << "GL context/GLEW init failed" << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    camera = new Camera(0.0f, -15.0f, 15.0f, 0.0f, -45.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 200.0f);

    return true;
}

GLuint setupShaders() {
    std::string vertexSource = readFile("shaders/vertex.glsl");
    std::string fragmentSource = readFile("shaders/fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return 0;

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
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void setupIslandGeometry(std::vector<float>& vertices, std::vector<unsigned int>& indices, float baseX, float baseY, int gridSize, float islandSize) {
    const float cellSize = islandSize / gridSize;
    auto heightFunc = [](float x, float y) { return 0.5f * (1.0f - (x*x + y*y)); };
    int vertexOffset = vertices.size() / 8;

    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            float x = baseX - 1.0f + j * cellSize;
            float y = baseY - 1.0f + i * cellSize;
            float u = j / (float)gridSize;
            float v = i / (float)gridSize;
            float h = std::max(heightFunc(u * 2.0f - 1.0f, v * 2.0f - 1.0f), 0.1f);

            float h_dx = heightFunc((u + 0.01f) * 2.0f - 1.0f, v * 2.0f - 1.0f);
            float h_dy = heightFunc(u * 2.0f - 1.0f, (v + 0.01f) * 2.0f - 1.0f);
            glm::vec3 tangent = glm::normalize(glm::vec3(0.01f, 0.0f, h_dx - h));
            glm::vec3 bitangent = glm::normalize(glm::vec3(0.0f, 0.01f, h_dy - h));
            glm::vec3 normal = glm::normalize(glm::cross(bitangent, tangent));

            vertices.push_back(x); vertices.push_back(y); vertices.push_back(h);
            vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
            vertices.push_back(u); vertices.push_back(v);
        }
    }

    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            int topLeft = vertexOffset + i * (gridSize + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + (gridSize + 1);
            int bottomRight = bottomLeft + 1;
            indices.push_back(topLeft); indices.push_back(bottomLeft); indices.push_back(topRight);
            indices.push_back(topRight); indices.push_back(bottomLeft); indices.push_back(bottomRight);
        }
    }
}

bool setupRendering(GLuint shaderProgram, std::vector<Unit3D>& units) {
    // Ocean
    float oceanVertices[] = {
        -15.0f, -15.0f, 0.0f, 15.0f, -15.0f, 0.0f, 15.0f, 15.0f, 0.0f,
        15.0f, 15.0f, 0.0f, -15.0f, 15.0f, 0.0f, -15.0f, -15.0f, 0.0f
    };
    glGenVertexArrays(1, &oceanVao);
    glGenBuffers(1, &oceanVbo);
    glBindVertexArray(oceanVao);
    glBindBuffer(GL_ARRAY_BUFFER, oceanVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oceanVertices), oceanVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Grid
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

    // Islands
    std::vector<float> islandVertices;
    std::vector<unsigned int> islandIndices;
    setupIslandGeometry(islandVertices, islandIndices, -10.0f, -10.0f, 10, 2.0f);
    setupIslandGeometry(islandVertices, islandIndices, 10.0f, 10.0f, 10, 2.0f);
    glGenVertexArrays(1, &islandVao);
    glGenBuffers(1, &islandVbo);
    glGenBuffers(1, &islandEbo);
    glBindVertexArray(islandVao);
    glBindBuffer(GL_ARRAY_BUFFER, islandVbo);
    glBufferData(GL_ARRAY_BUFFER, islandVertices.size() * sizeof(float), islandVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, islandEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, islandIndices.size() * sizeof(unsigned int), islandIndices.data(), GL_STATIC_DRAW);

    // Clouds
    float cloudVertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    glGenVertexArrays(1, &cloudVao);
    glGenBuffers(1, &cloudVbo);
    glBindVertexArray(cloudVao);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cloudVertices), cloudVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Skybox
    float skyboxVertices[] = {
        -100.0f, 100.0f, -100.0f, -100.0f, -100.0f, -100.0f, 100.0f, -100.0f, -100.0f,
        100.0f, -100.0f, -100.0f, 100.0f, 100.0f, -100.0f, -100.0f, 100.0f, -100.0f,
        -100.0f, -100.0f, 100.0f, -100.0f, -100.0f, -100.0f, -100.0f, 100.0f, -100.0f,
        -100.0f, 100.0f, -100.0f, -100.0f, 100.0f, 100.0f, -100.0f, -100.0f, 100.0f,
        100.0f, -100.0f, -100.0f, 100.0f, -100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
        100.0f, 100.0f, 100.0f, 100.0f, 100.0f, -100.0f, 100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f, 100.0f, -100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
        100.0f, 100.0f, 100.0f, 100.0f, -100.0f, 100.0f, -100.0f, -100.0f, 100.0f,
        -100.0f, 100.0f, -100.0f, 100.0f, 100.0f, -100.0f, 100.0f, 100.0f, 100.0f,
        100.0f, 100.0f, 100.0f, -100.0f, 100.0f, 100.0f, -100.0f, 100.0f, -100.0f,
        -100.0f, -100.0f, -100.0f, -100.0f, -100.0f, 100.0f, 100.0f, -100.0f, -100.0f,
        100.0f, -100.0f, -100.0f, -100.0f, -100.0f, 100.0f, 100.0f, -100.0f, 100.0f
    };
    glGenVertexArrays(1, &skyboxVao);
    glGenBuffers(1, &skyboxVbo);
    glBindVertexArray(skyboxVao);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Textures
    std::string texturePath = getResourcePath("textures/carrier_texture.png");
    std::string getResourcePath(const std::string& relativePath) {
      // Base path could be set via config or environment variable
      std::string basePath = "../Models/";
      return basePath + relativePath;
  }

    SDL_Surface* surface = IMG_Load(texturePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load carrier texture: " << IMG_GetError() << std::endl;
        return false;
    }
    std::cout << "Carrier texture loaded: " << surface->w << "x" << surface->h << std::endl;
    glGenTextures(1, &carrierTexture);
    glBindTexture(GL_TEXTURE_2D, carrierTexture);
    GLenum format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);

    glGenTextures(1, &islandTexture);
    glBindTexture(GL_TEXTURE_2D, islandTexture);
    unsigned char islandPixel[] = {139, 69, 19, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, islandPixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &cloudTexture);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    unsigned char cloudPixel[] = {255, 255, 255, 200};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, cloudPixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void runGameLoop(GLuint shaderProgram, std::vector<Unit3D>& units) {
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            camera->update(event);
            handleCombat(event, units);

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float x = (2.0f * mouseX) / 810 - 1.0f;
                float y = 1.0f - (2.0f * mouseY) / 610;
                glm::vec4 rayClip = glm::vec4(x, y, -1.0, 1.0);
                glm::vec4 rayEye = glm::inverse(projection) * rayClip;
                rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
                glm::vec3 rayWorld = glm::vec3(glm::inverse(camera->getViewMatrix()) * rayEye);
                rayWorld = glm::normalize(rayWorld);
                glm::mat4 view = camera->getViewMatrix();
                glm::mat4 invView = glm::inverse(view);
                glm::vec3 rayOrigin = glm::vec3(invView[3]);
                float t = -rayOrigin.z / rayWorld.z;
                glm::vec3 intersection = rayOrigin + t * rayWorld;
                int gridX = std::floor(intersection.x + 15.0f);
                int gridY = std::floor(intersection.y + 15.0f);
                if (!units.empty() && !isIslandCell(gridX, gridY)) {
                    for (auto& unit : units) {
                        unit.gridX = gridX;
                        unit.gridY = gridY;
                    }
                    std::cout << "Moved carrier to gridX=" << gridX << ", gridY=" << gridY << std::endl;
                } else if (isIslandCell(gridX, gridY)) {
                    std::cout << "Cannot move to island at gridX=" << gridX << ", gridY=" << gridY << std::endl;
                }
            }
        }

        cloudTime += 0.01f;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Skybox
        glDepthMask(GL_FALSE);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(camera->getViewMatrix()))));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 0.3f, 0.5f, 0.8f, 1.0f);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "isSkybox"), 1);
        glBindVertexArray(skyboxVao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
        glUniform1i(glGetUniformLocation(shaderProgram, "isSkybox"), 0);

        // Ocean
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 0.0f, 0.0f, 1.0f, 1.0f);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
        glBindVertexArray(oceanVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Grid
        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(gridVao);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

        // Islands
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, islandTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
        glBindVertexArray(islandVao);
        glDrawElements(GL_TRIANGLES, 2 * 10 * 10 * 6, GL_UNSIGNED_INT, 0);

        // Clouds
        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, cloudTexture);
        glm::vec3 cloudPositions[] = {
            glm::vec3(-5.0f + 0.5f * sin(cloudTime), -5.0f, 12.0f),
            glm::vec3(5.0f + 0.5f * cos(cloudTime), 5.0f, 14.0f)
        };
        for (const auto& pos : cloudPositions) {
            glm::vec3 camPos = glm::vec3(camera->getViewMatrix()[3]);
            glm::vec3 toCamera = glm::normalize(camPos - pos);
            glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 right = glm::normalize(glm::cross(toCamera, up));
            up = glm::normalize(glm::cross(right, toCamera));
            model = glm::mat4(1.0f);
            model[0] = glm::vec4(right, 0.0f);
            model[1] = glm::vec4(up, 0.0f);
            model[2] = glm::vec4(-toCamera, 0.0f);
            model = glm::translate(model, pos);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cloudVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glDepthMask(GL_TRUE);

        // Carrier
        glBindTexture(GL_TEXTURE_2D, carrierTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
        for (auto& unit : units) {
            model = glm::mat4(1.0f);
            float worldX = unit.gridX - 15.0f + 0.5f;
            float worldY = unit.gridY - 15.0f + 0.5f;
            unit.position = glm::vec3(worldX, worldY, 0.0f);
            model = glm::translate(model, unit.position);
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            std::cout << "Rendering unit at position: (" << worldX << ", " << worldY << ", 0.0)" << std::endl;
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(unit.vao);
            glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_INT, 0);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        SDL_GL_SwapWindow(window);
    }
}

void cleanupSDLAndOpenGL() {
    glDeleteTextures(1, &carrierTexture);
    glDeleteTextures(1, &islandTexture);
    glDeleteTextures(1, &cloudTexture);
    glDeleteVertexArrays(1, &oceanVao); glDeleteBuffers(1, &oceanVbo);
    glDeleteVertexArrays(1, &gridVao); glDeleteBuffers(1, &gridVbo);
    glDeleteVertexArrays(1, &islandVao); glDeleteBuffers(1, &islandVbo); glDeleteBuffers(1, &islandEbo);
    glDeleteVertexArrays(1, &cloudVao); glDeleteBuffers(1, &cloudVbo);
    glDeleteVertexArrays(1, &skyboxVao); glDeleteBuffers(1, &skyboxVbo);
    delete camera;
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}