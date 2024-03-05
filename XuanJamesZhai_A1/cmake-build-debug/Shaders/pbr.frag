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
layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;
layout(binding = 4) uniform sampler2D metallicSampler;
layout(binding = 5) uniform samplerCube cubeSampler[10];

vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}

// Function to calculate the Fresnel term
float SchlickFresnel(float cosTheta, vec3 F0) {
    return max(dot(F0, vec3(1.0 - cosTheta)), 0.0);
}

// Function to calculate the GGX (Trowbridge-Reitz) distribution function
float DistributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159 * denom * denom;

    return nom / denom;
}


vec3 GetEnvBRDF(float roughness, vec3 R){
    int lod = int(roughness*10);
    if(lod > 9) lod = 9;
    if(lod < 0) lod = 0;

    return texture(cubeSampler[lod], R).rgb;
}


vec3 diffuse_brdf(vec3 color) {
    return (1/3.14159) * color;
}


void main() {

    vec3 N = texture(normalSampler, fragTexCoord).rgb;
    N = N * 2.0 - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(fragPosition-ubo.viewPos);
    // Calculate the reflection vector
    vec3 R = reflect(-V, N);

    float roughness = texture(roughnessSampler, fragTexCoord).r;
    float metallic = texture(metallicSampler, fragTexCoord).r;

    // Sample pre-filtered map to get the roughness-dependent specular intensity
    float cosTheta = max(dot(R, V), 0.0);
    vec3 envBRDF = GetEnvBRDF(roughness,R);

    // Fresnel term
    vec3 F0 = vec3(0.04); // Base reflectance for non-metals
    F0 = mix(F0, vec3(1.0), metallic);

    float F = SchlickFresnel(cosTheta, F0);

    // Distribution function (GGX)
    float D = DistributionGGX(cosTheta, roughness);

    // Final specular intensity
    vec3 specular = F * D * envBRDF;

    // Final color
    vec3 baseColor = texture(albedoSampler, fragTexCoord).xyz;
    vec3 ambient = vec3(1.0); // Ambient color

    vec3 color = (baseColor * (1.0 - F) * ambient + specular);


    outColor = vec4(color,1.0);
}