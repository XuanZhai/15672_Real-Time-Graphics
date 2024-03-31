#version 460
#extension GL_EXT_nonuniform_qualifier : require

const uint MAX_LIGHT_COUNT = 10;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in mat3 TBN;
layout(location = 7) in vec4 fragPositionLightSpace[MAX_LIGHT_COUNT];

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec3 viewPos;
} ubo;

/* Struct of a single light source. */
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

/* The number of lights and a list of lights. */
layout(std140, set = 0, binding = 1) uniform UniformLightsObject {
    uint lightSize;
    UniformLightObject lights[MAX_LIGHT_COUNT];
} lightObjects;
/* A list of shadow maps, one for each light source. */
layout(set = 0, binding = 2) uniform sampler2D depthMap[];

layout(set = 1, binding = 0) uniform sampler2D normalSampler;
layout(set = 1, binding = 1) uniform sampler2D heightSampler;
layout(set = 1, binding = 2) uniform sampler2D albedoSampler;
layout(set = 2, binding = 0) uniform samplerCube cubeSampler;


vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}

/* Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/ */
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


/* Approximate the Fresnel term using Schlick approximation. */
vec3 SchlickFresnel(float WoH, vec3 F0) {
    return F0 + (1 - F0) * pow(clamp(1.0 - WoH, 0.0, 1.0), 5.0);
}


/* Reference: https://learnopengl.com/PBR/IBL/Diffuse-irradiance. */
vec3 GetEnvironmentLight(vec3 viewDir, vec3 normal, vec3 albedo){
    vec3 F0 = vec3(0.04);
    vec3 kS = SchlickFresnel(max(dot(normal, normalize(-viewDir)), 0.0),F0);
    vec3 kD = 1.0 - kS;

    vec3 irradiance = texture(cubeSampler, normal).xyz;

    return toneMapACES(irradiance * albedo * kD,1.0);
}


/* Add light effect for a signle light. */
vec3 DiffuseLightCalculation(UniformLightObject light, vec3 normal, vec3 albedo){

    vec3 Lo = vec3(0);
    vec3 L = light.pos - fragPosition;
    float NoL = max(dot(normal, normalize(L)), 0.0);

    /* If sun light, use the light direction and also no falloff. */
    if(light.type == 0){
        NoL = max(dot(normalize(-light.dir), normal), 0.0);
        Lo = (albedo/3.14159) * light.strength * light.tint * NoL;
        Lo = Lo / (Lo + vec3(1.0));
        Lo = pow(Lo, vec3(1.0/2.2));
        return Lo;
    }

    /* Calculate the falloff */
    float d = length(L);
    float falloff = 1 - pow(d/light.limit,4.0);
    falloff = clamp(falloff,0.0,1.0);
    falloff = falloff / (d*d+1);
    vec3 irradiance = light.power * light.tint / (4*3.14159);

    /* If it is a sphere light. */
    if(light.type == 1){
        Lo = (albedo/3.14159) * falloff * irradiance * NoL;
    }
    /* If it is a spot light. */
    else if(light.type == 2){
        float LoDir = max(dot(normalize(-light.dir), normalize(L)), 0.0);
        float angle = abs(acos(LoDir));
        float minAngle = abs((light.fov * (1 - light.blend)) / 2);
        float maxAngle = abs(light.fov / 2);
        vec3 maxLo = (albedo/3.14159) * falloff * irradiance * NoL;
        /* If out of the max angle, no light effect adds to the fragment. */
        if(angle > maxAngle){}
        /* If in the inner bound, use the full light. */
        else if(angle <= minAngle){
            Lo = maxLo;
        }
        /* Lerp based on the randians. */
        else{
            float alpha = (angle - minAngle) / (maxAngle - minAngle);
            Lo = mix(maxLo,vec3(0),alpha);
        }
    }
    /* Gamma Correction. */
    Lo = Lo / (Lo + vec3(1.0));
    Lo = pow(Lo, vec3(1.0/2.2));
    return Lo;
}


/* Check shadow effect for a given light. */
float ShadowCalculation(uint lightIndex, vec3 normal) {
    /* Convert light to NDC space. */
    vec3 fragPositionLightNDC = fragPositionLightSpace[lightIndex].xyz / fragPositionLightSpace[lightIndex].w;

    /* NDC to [0,1] range. */
    fragPositionLightNDC.x = fragPositionLightNDC.x * 0.5 + 0.5;
    fragPositionLightNDC.y = fragPositionLightNDC.y * 0.5 + 0.5;
    /* Current fragment from light's perspective. */
    float currentDepth = fragPositionLightNDC.z;

    /* check whether current frag pos is in shadow using PCF with bias. */
    /* Inspired by: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping */
    //float bias = max(0.05 * (1.0 - dot(normal, -lightObjects.lights[lightIndex].dir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap[lightIndex], 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(depthMap[lightIndex], fragPositionLightNDC.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth) > pcfDepth ? 0.0 : 1.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}


void main() {

    vec3 viewDir = normalize(fragPosition-ubo.viewPos);
    vec2 texCoord = fragTexCoord;
    texCoord.y = 1-texCoord.y;
    texCoord = ParallaxOcclusionMapping(texCoord,normalize(transpose(TBN) * viewDir));

    vec3 normal = texture(normalSampler, texCoord).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    vec3 albedo = texture(albedoSampler, texCoord).xyz * fragColor.xyz;

    vec3 color = vec3(0);
    color += GetEnvironmentLight(viewDir, normal, albedo);

    for(uint i = 0; i < lightObjects.lightSize; i++){
        color += ShadowCalculation(i,normal) * DiffuseLightCalculation(lightObjects.lights[i], normal,albedo);
    }

    outColor = vec4(color,1.0);
}