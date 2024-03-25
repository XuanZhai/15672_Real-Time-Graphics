#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

struct UniformLightObject {
/* 0 = sun, 1 = sphere, 2 = spot */
    uint type;
    float angle;
    float strength;
    float radius;
    float power;
    float limit;
    float fov;
    float blend;
    vec3 pos;
    vec3 dir;
    vec3 tint;
    mat4 view;
    mat4 proj;
};

layout(std140, set = 0, binding = 1) uniform UniformLightsObject {
    uint lightSize;
    UniformLightObject lights[10];
} lightObjects;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;
layout(location = 5) in mat4 inModel;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragPosition;

void main() {

    gl_Position = ubo.proj * ubo.view * inModel * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragNormal = normalize((transpose(inverse(inModel)) * vec4(inNormal,1.0)).xyz);

    fragTexCoord = vec2(0,0);

    fragPosition = (inModel * vec4(inPosition,1.0)).xyz;
}