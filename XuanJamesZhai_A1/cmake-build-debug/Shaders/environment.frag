#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in mat3 TBN;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D heightSampler;
layout (binding = 3) uniform samplerCube cubeMapTexture;


vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}


/* Reference: https://learnopengl.com/Advanced-Lighting/Parallax-Mapping */
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir){
    // number of depth layers
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * 0.1;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(heightSampler, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightSampler, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightSampler, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return clamp(finalTexCoords,0,1);
}


void main() {

    vec3 viewDir = normalize(fragPosition-ubo.viewPos);

    vec2 texCoord = ParallaxOcclusionMapping(fragTexCoord,viewDir);

    vec3 normal = texture(normalSampler, texCoord).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    vec3 color = toneMapReinhard(texture(cubeMapTexture,normal).xyz,1);

    outColor = vec4(color,1.0);
}