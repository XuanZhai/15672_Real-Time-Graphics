#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;


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
layout(set = 0, binding = 2) uniform sampler2D depthMap[];

layout(set = 1, binding = 0) uniform sampler2D normalSampler;
layout(set = 1, binding = 1) uniform sampler2D heightSampler;


void main() {
    //outColor = fragColor;
    float depthValue = texture(depthMap[0], fragTexCoord).r;
    if(depthValue < 1.0f){
        outColor = vec4(1.0,1.0,1.0, 1.0);
    }
    else{
        outColor = vec4(0.0,0.0,0.0, 1.0);
    }
    //outColor = vec4(depthValue,1.0,1.0, 1.0);
}