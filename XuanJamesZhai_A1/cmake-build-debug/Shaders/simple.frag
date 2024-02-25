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

void main() {
    vec3 light = mix(vec3(0,0,0), vec3(1,1,1), dot(normalize(fragNormal), vec3(0,0,1)) * 0.5 + 0.5);
    outColor = vec4(light * fragColor.xyz, fragColor.w);
}