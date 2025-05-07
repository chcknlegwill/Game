#pragma once
#include <string>

std::string getResourcePath(const std::string& relativePath) {
    std::string basePath = "../Resources/"; // Configurable via env or config file
    return basePath + relativePath;
}