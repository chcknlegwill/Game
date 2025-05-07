#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
uniform bool useTexture;
uniform bool isSkybox;
uniform vec4 baseColor;
void main() {
    if (isSkybox) {
        vec3 norm = normalize(FragPos);
        float t = (norm.z + 1.0) * 0.5;
        vec3 color = mix(vec3(0.7, 0.8, 1.0), vec3(0.3, 0.5, 0.8), t);
        FragColor = vec4(color, 1.0);
    } else if (useTexture) {
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec4 texColor = texture(texture1, TexCoord);
        vec3 diffuse = diff * texColor.rgb;
        vec3 ambient = 0.2 * texColor.rgb;
        FragColor = vec4(ambient + diffuse, texColor.a);
    } else {
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * baseColor.rgb;
        vec3 ambient = 0.2 * baseColor.rgb;
        FragColor = vec4(ambient + diffuse, baseColor.a);
    }
}