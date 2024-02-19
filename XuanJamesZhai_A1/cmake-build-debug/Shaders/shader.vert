#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 inColor;

layout(location = 3) in mat4 model;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {

    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragNormal = (transpose(inverse(model)) * vec4(normal,1.0)).xyz;
    fragTexCoord = vec2(0,0);
}