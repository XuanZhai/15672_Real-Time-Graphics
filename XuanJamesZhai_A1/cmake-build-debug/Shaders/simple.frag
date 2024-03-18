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
};

layout(std140, set = 0, binding = 1) uniform UniformLightsObject {
    uint lightSize;
    UniformLightObject lights[10];
} lightObjects;

layout(set = 1, binding = 0) uniform sampler2D normalSampler;
layout(set = 1, binding = 1) uniform sampler2D heightSampler;

vec3 LightCalculation(UniformLightObject light){

    vec3 viewDir = normalize(fragPosition-ubo.viewPos);
    vec3 normal = normalize(fragNormal);

    vec3 r = reflect(viewDir, normal);
    vec3 L = light.pos - fragPosition;
    vec3 centerToRay = (dot(L, r) * r) - L;
    vec3 closestPoint = L + centerToRay * clamp(light.radius / length(centerToRay), 0.0,1.0);
    L = normalize(closestPoint);
    float d = length(closestPoint);

    float falloff = 1;
    if(light.limit != 0){
        falloff = falloff * max(0, 1 - pow(d/light.limit,4.0));
    }
    else{
        falloff = pow(1 - pow(d/light.radius,4),2);
        falloff = clamp(falloff,0.0,1.0);
        falloff = falloff / (d*d+1);
    }

    float diff = max(dot(normal, L), 0.0);
    vec3 Li = light.power*light.tint / (4*3.14159);
    return falloff*Li*diff;
}

void main() {
    //vec3 light = mix(vec3(0,0,0), vec3(1,1,1), dot(normalize(fragNormal), lightObjects.lights[0].dir) * 0.5 + 0.5);
    //outColor = vec4(light * fragColor.xyz, fragColor.w);

    vec3 color = fragColor.xyz;

    for(int i = 0; i < lightObjects.lightSize; i++){
        color = color * LightCalculation(lightObjects.lights[i]);
    }

    outColor = vec4(color, fragColor.w);
}