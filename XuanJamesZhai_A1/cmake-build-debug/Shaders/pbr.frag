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
layout(binding = 3) uniform sampler2D albedoSampler;
layout(binding = 4) uniform sampler2D roughnessSampler;
layout(binding = 5) uniform sampler2D metallicSampler;
layout(binding = 6) uniform sampler2D brdfSampler;
layout(binding = 7) uniform samplerCube cubeSampler[10];


vec3 toneMapReinhard(vec3 color, float exposure) {
    return color / (color + vec3(1.0)) * exposure;
}

// Function to calculate the Fresnel term
float SchlickFresnel(float cosTheta, vec3 F0) {
    return max(dot(F0, vec3(1.0 - cosTheta)), 0.0);
}


vec3 GetEnv(float roughness, vec3 R){
    int lod = int(roughness*10);
    if(lod > 9) lod = 9;
    if(lod < 0) lod = 0;

    return texture(cubeSampler[lod], R).rgb;
}

vec2 GetBRDF(float roughness, float NoV){

    float roughnessLod = roughness*10;
    float novLod = NoV*10;

    if(roughnessLod > 9) roughnessLod = 9;
    if(roughnessLod < 0) roughnessLod = 0;

    if(novLod > 9) novLod = 9;
    if(novLod < 0) novLod = 0;

    vec2 tex = vec2(roughnessLod, novLod);
    return texture(brdfSampler, tex).rg;
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
    // Fresnel term
    vec3 F0 = vec3(0.04); // Base reflectance for non-metals
    F0 = mix(F0, vec3(1.0), metallic);
    float F = SchlickFresnel(cosTheta, F0);

    float NoV = max(0.0, min(1.0, dot(N,V)));
    vec3 EnvColor = GetEnv( roughness, R );
    vec2 EnvBRDF = GetBRDF( roughness, NoV );

    vec3 diffuse = texture(albedoSampler, fragTexCoord).xyz;
    vec3 specular = EnvColor * ( F * EnvBRDF.x + EnvBRDF.y );
    vec3 ambient = vec3(0.05); // Ambient color

    vec3 color = mix(diffuse, specular, metallic);
    color += ambient;

    outColor = vec4(color,1.0);
}