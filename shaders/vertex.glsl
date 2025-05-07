#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int isSkybox;

void main() {
    // Transform vertex position to world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal to world space (for lighting)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates
    TexCoord = aTexCoord;

    // Skybox: remove translation from view matrix if isSkybox is 1
    mat4 viewNoTranslation = isSkybox == 1 ? mat4(mat3(view)) : view;
    
    // Transform vertex position to clip space
    gl_Position = projection * viewNoTranslation * model * vec4(aPos, 1.0);
}
