#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <vector>
#include "Camera.hpp"

struct Unit3D {
    int gridX, gridY;
    glm::vec3 position;
    GLuint vao, vbo, ebo;
    int vertexCount;
    int indexCount;
};

std::vector<Unit3D> loadModel(const std::string& path) {
    std::cout << "Attempting to load model: " << path << std::endl;
    std::vector<Unit3D> units;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return units;
    }
    std::cout << "Loaded model: " << path << ", meshes: " << scene->mNumMeshes << std::endl;

    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        aiMesh* mesh = scene->mMeshes[meshIdx];
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        Unit3D unit;
        unit.gridX = 0;
        unit.gridY = 0;
        unit.position = glm::vec3(0.0f, 0.0f, 0.0f);
        unit.vertexCount = mesh->mNumVertices;
        unit.indexCount = indices.size();

        glGenVertexArrays(1, &unit.vao);
        glGenBuffers(1, &unit.vbo);
        glGenBuffers(1, &unit.ebo);

        glBindVertexArray(unit.vao);

        glBindBuffer(GL_ARRAY_BUFFER, unit.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unit.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        units.push_back(unit);
        std::cout << "Loaded mesh " << meshIdx << ": " << mesh->mNumVertices << " vertices, " << indices.size() << " indices" << std::endl;
    }

    return units;
}

int main(int argc, char* argv[]) {
    std::cout << "Starting 3D Strategy Game..." << std::endl;

    int SCREEN_WIDTH = 810;
    int SCREEN_HEIGHT = 610;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

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

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        out vec3 Normal;
        out vec3 FragPos;
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 Normal;
        in vec3 FragPos;
        out vec4 FragColor;
        uniform vec4 baseColor;
        void main() {
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            vec3 norm = normalize(Normal);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * vec3(baseColor);
            vec3 ambient = 0.2 * vec3(baseColor);
            FragColor = vec4(ambient + diffuse, 1.0);
        }
    )";

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
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float oceanVertices[] = {
        -7.5f, -7.5f, 0.0f,
         7.5f, -7.5f, 0.0f,
         7.5f,  7.5f, 0.0f,
         7.5f,  7.5f, 0.0f,
        -7.5f,  7.5f, 0.0f,
        -7.5f, -7.5f, 0.0f
    };
    GLuint oceanVao, oceanVbo;
    glGenVertexArrays(1, &oceanVao);
    glGenBuffers(1, &oceanVbo);
    glBindVertexArray(oceanVao);
    glBindBuffer(GL_ARRAY_BUFFER, oceanVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oceanVertices), oceanVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<float> gridVertices;
    for (float x = -7.5f; x <= 7.5f; x += 1.0f) {
        gridVertices.push_back(x); gridVertices.push_back(-7.5f); gridVertices.push_back(0.01f);
        gridVertices.push_back(x); gridVertices.push_back( 7.5f); gridVertices.push_back(0.01f);
    }
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

    std::vector<Unit3D> units = loadModel("../Models/carrier_low.obj");
    if (units.empty()) {
        std::cerr << "Failed to load carrier model" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    Camera camera(0.0f, -10.0f, 10.0f, 0.0f, -45.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            camera.update(event);

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float x = (2.0f * mouseX) / SCREEN_WIDTH - 1.0f;
                float y = 1.0f - (2.0f * mouseY) / SCREEN_HEIGHT;
                glm::vec4 rayClip = glm::vec4(x, y, -1.0, 1.0);
                glm::vec4 rayEye = glm::inverse(projection) * rayClip;
                rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
                glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.getViewMatrix()) * rayEye);
                rayWorld = glm::normalize(rayWorld);
                glm::mat4 view = camera.getViewMatrix();
                glm::mat4 invView = glm::inverse(view);
                glm::vec3 rayOrigin = glm::vec3(invView[3]);
                float t = -rayOrigin.z / rayWorld.z;
                glm::vec3 intersection = rayOrigin + t * rayWorld;
                int gridX = std::floor(intersection.x + 7.5f);
                int gridY = std::floor(intersection.y + 7.5f);
                if (!units.empty()) {
                    for (auto& unit : units) {
                        unit.gridX = gridX;
                        unit.gridY = gridY;
                    }
                    std::cout << "Moved carrier to gridX=" << gridX << ", gridY=" << gridY << std::endl;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 0.0f, 0.0f, 1.0f, 1.0f);
        glBindVertexArray(oceanVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glLineWidth(1.0f);
        glBindVertexArray(gridVao);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

        for (auto& unit : units) {
            model = glm::mat4(1.0f);
            float worldX = unit.gridX - 7.5f + 0.5f;
            float worldY = unit.gridY - 7.5f + 0.5f;
            unit.position = glm::vec3(worldX, worldY, 0.0f);
            model = glm::translate(model, unit.position);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(2.0f, 0.0f, 0.0f));
            float scaleFactor = 0.5f;
            model = glm::scale(model, glm::vec3(scaleFactor));
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.00f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform4f(glGetUniformLocation(shaderProgram, "baseColor"), 0.5f, 0.5f, 0.5f, 1.0f);
            glBindVertexArray(unit.vao);
            glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_INT, 0);
        }

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &oceanVao);
    glDeleteBuffers(1, &oceanVbo);
    glDeleteVertexArrays(1, &gridVao);
    glDeleteBuffers(1, &gridVbo);
    for (auto& unit : units) {
        glDeleteVertexArrays(1, &unit.vao);
        glDeleteBuffers(1, &unit.vbo);
        glDeleteBuffers(1, &unit.ebo);
    }
    glDeleteProgram(shaderProgram);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Cleaning up..." << std::endl;
    return 0;
}