#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 viewDir;

layout(location = 0) out vec4 outColor;

//layout(binding = 1) uniform sampler2D texSampler;
layout (binding = 1) uniform samplerCube cubeMapTexture;

void main() {
    vec3 light = mix(vec3(0,0,0), vec3(1,1,1), dot(normalize(fragNormal), vec3(0,0,1)) * 0.5 + 0.5);

    vec3 reflectedDir = reflect(normalize(viewDir), normalize(fragNormal));

    outColor = texture(cubeMapTexture, normalize(fragNormal));
}