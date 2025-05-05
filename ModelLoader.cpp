#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <GL/glew.h>

std::vector<Unit3D> loadModels(const std::string& path) {
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