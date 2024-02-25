#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in mat4 inModel;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragPosition;

void main() {

    gl_Position = ubo.proj * ubo.view * inModel * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragNormal = (transpose(inverse(inModel)) * vec4(normal,1.0)).xyz;
    fragTexCoord = vec2(0,0);

    fragPosition = (inModel * vec4(inPosition,1.0)).xyz;
}