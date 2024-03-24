#version 450

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;
layout(location = 5) in mat4 inModel;

void main() {
    gl_Position = pushConstants.proj * pushConstants.view * inModel * vec4(inPosition, 1.0);
    //gl_Position = vec4(0,0,fract(sin(dot(inPosition.xy ,vec2(12.9898,78.233))) * 43758.5453), 1.0);
}