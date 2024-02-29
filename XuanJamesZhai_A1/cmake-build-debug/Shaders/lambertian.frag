#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;


layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform samplerCube cubeSampler;

vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}

void main() {

    vec3 viewDir = normalize(fragPosition-ubo.viewPos);
    vec3 reflectedDir = reflect(normalize(viewDir), normalize(fragNormal));

    vec3 baseColor = texture(albedoSampler, fragTexCoord).xyz;

    vec3 lambert = toneMapReinhard(texture(cubeSampler, reflectedDir).xyz,1);
    //vec3 lambert = texture(cubeSampler, reflectedDir).xyz;

    outColor = vec4(lambert * baseColor,1.0);
}