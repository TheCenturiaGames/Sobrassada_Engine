#version 460

#extension GL_ARB_bindless_texture : require

#define PI 3.14159265359

layout(location = 0)out vec4 gDiffuse;
layout(location = 1)out vec4 gSpecular;
layout(location = 2)out vec4 gPosition;
layout(location = 3)out vec4 gNormal;

in vec3 pos;
in vec2 uv0;
in vec3 normal;
in vec4 tangent;
flat in int instance_index;

uniform vec3 cameraPos;

struct Material
{
    vec4 diffColor;
    vec3 specColor;
    float shininess;
    bool shininessInAlpha;
    float metallicFactor;
    float roughnessFactor;
    uvec2 diffuseTex;
    uvec2 specularTex;
    uvec2 metallicTex;
    uvec2 normalTex;
};

readonly layout(std430, binding = 11) buffer Materials {
    Material materials[];
};

void main()
{
    const Material mat = materials[instance_index];

    gDiffuse = vec4(pow(texture(sampler2D(mat.diffuseTex), uv0).rgb, vec3(2.2f)), 1);
    gSpecular = vec4(pow(texture(sampler2D(mat.metallicTex), uv0), vec4(2.2)));
    gPosition = vec4(pos,0);
    gNormal = vec4(normal,0);
}