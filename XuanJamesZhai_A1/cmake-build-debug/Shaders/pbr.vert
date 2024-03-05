#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

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
layout(location = 4) out mat3 TBN;

void main() {

    gl_Position = ubo.proj * ubo.view * inModel * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragNormal = (transpose(inverse(inModel)) * vec4(inNormal,1.0)).xyz;
    fragTexCoord = inTexCoord;

    vec3 T = (transpose(inverse(inModel)) * vec4(inTangent.xyz,1.0)).xyz;
    vec3 B = cross(fragNormal,T);
    if(inTangent.a < 0){
        B = -B;
    }
    TBN = mat3(T, B, fragNormal);;

    fragPosition = (inModel * vec4(inPosition,1.0)).xyz;
}