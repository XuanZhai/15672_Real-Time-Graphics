#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in mat3 TBN;

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
};

layout(std140, set = 0, binding = 1) uniform UniformLightsObject {
    uint lightSize;
    UniformLightObject lights[10];
} lightObjects;

layout(set = 1, binding = 0) uniform sampler2D normalSampler;
layout(set = 1, binding = 1) uniform sampler2D heightSampler;
layout(set = 1, binding = 2) uniform sampler2D albedoSampler;
layout(set = 1, binding = 3) uniform sampler2D roughnessSampler;
layout(set = 1, binding = 4) uniform sampler2D metallicSampler;
layout(set = 2, binding = 0) uniform samplerCube LamcubeSampler;
layout(set = 2, binding = 1) uniform sampler2D brdfSampler;
layout(set = 2, binding = 2) uniform samplerCube cubeSampler[10];


vec3 toneMapACES(vec3 color, float exposure){
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;

    color *= exposure;
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}


// Function to calculate the Fresnel term
// Reference: https://learnopengl.com/PBR/IBL/Diffuse-irradiance
// Reference: https://learnopengl.com/PBR/IBL/Specular-IBL

vec3 SchlickFresnel(float WoH, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - WoH, 0.0, 1.0), 5.0);
}


vec3 GetEnv(float roughness, vec3 R){
    int lod = int(roughness*10);
    if(lod > 9) lod = 9;
    if(lod < 0) lod = 0;

    return toneMapACES(texture(cubeSampler[lod], R).rgb,1.0);
}

vec2 GetBRDF(float roughness, float NoV){

    float roughnessLod = roughness*10;
    float novLod = NoV*10;

    if(roughnessLod > 9) roughnessLod = 9;
    if(roughnessLod < 0) roughnessLod = 0;

    if(novLod > 9) novLod = 9;
    if(novLod < 0) novLod = 0;

    //vec2 tex = vec2(roughnessLod, novLod);
    vec2 tex = vec2(1-roughness, NoV);
    return texture(brdfSampler, tex).rg;
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


void main() {

    vec3 viewDir = normalize(fragPosition-ubo.viewPos) ;
    vec2 texCoord = fragTexCoord;
    texCoord.y = 1-texCoord.y;
    texCoord = ParallaxOcclusionMapping(texCoord, normalize(transpose(TBN) * viewDir));

    vec3 N = texture(normalSampler, texCoord).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(fragPosition-ubo.viewPos);
    vec3 R = reflect(V, N);

    float roughness = texture(roughnessSampler, texCoord).r;
    float metallic = texture(metallicSampler, texCoord).r;

    // Sample pre-filtered map to get the roughness-dependent specular intensity
    float cosTheta = max(dot(R, V), 0.0);
    // Fresnel term
    vec3 F0 = vec3(0.04);
   // F0 = mix(F0, vec3(1.0), metallic);
    vec3 F = SchlickFresnel(cosTheta, F0,roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD = kD * (1-metallic);

    float NoV = max(0.0, min(1.0, dot(N,V)));
    vec3 EnvColor = GetEnv( roughness, R );
    vec2 EnvBRDF = GetBRDF( roughness, NoV );

    vec3 irradiance = toneMapACES(texture(LamcubeSampler, N).rgb,1.0);
    vec3 diffuse  = irradiance * texture(albedoSampler, vec2(0,0)).xyz;

    //vec3 diffuse = texture(albedoSampler, texCoord).xyz;
    vec3 specular = EnvColor * ( F * EnvBRDF.x + EnvBRDF.y );

    vec3 color = kD * diffuse + specular;

    outColor = vec4(color,1.0);
}