#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in mat3 TBN;
layout(location = 7) in vec4 fragPositionLightSpace[10];

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
layout (set = 2, binding = 0) uniform samplerCube cubeMapTexture;

vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}


vec3 toneMapACES(vec3 color, float exposure){
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;

    color *= exposure;
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}


/* Reference: https://learnopengl.com/Advanced-Lighting/Parallax-Mapping */
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir){

    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
/* the amount to shift the texture coordinates per layer. */
    vec2 P = viewDir.xy * 0.1;
    vec2 deltaTexCoords = P / numLayers;
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(heightSampler, currentTexCoords).r;

/* Keep iterating the layers. */
    while(currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(heightSampler, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

/* Interpolate with the previous layer. */
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightSampler, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return clamp(finalTexCoords,0,1);
}


float ShadowCalculation(uint lightIndex) {

    // perform perspective divide
    vec3 projCoords = fragPositionLightSpace[lightIndex].xyz / fragPositionLightSpace[lightIndex].w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap[lightIndex], projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}


void main() {

    vec3 viewDir = normalize(fragPosition-ubo.viewPos);
    vec2 texCoord = fragTexCoord;
    texCoord.y = 1-texCoord.y;
    texCoord = ParallaxOcclusionMapping(texCoord,normalize(transpose(TBN) * viewDir));
    //vec2 texCoord = fragTexCoord;

    vec3 normal = texture(normalSampler, texCoord).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN*normal);

    vec3 reflectedDir = reflect(normalize(viewDir), normalize(normal));

    vec3 color = toneMapACES(texture(cubeMapTexture, reflectedDir).xyz,1);

    outColor = vec4(color,1.0);
}