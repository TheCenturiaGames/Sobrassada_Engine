#version 460

#extension GL_ARB_bindless_texture : require

#define PI 3.14159265359

in vec3 texCoords;
uniform bool isHDR;

uniform uvec2 hdrSkybox;
uniform samplerCube skybox;


out vec4 fragColor;

vec2 CartesianToEquirectangular(in vec3 dir)
{
    float phi;
    phi = atan(dir.z, dir.x);
    phi = phi/(2.0 * PI) + 0.5;
    float theta = asin(dir.y);
    theta = (theta / PI + 0.5);
    return vec2(phi, 1.0 - theta);
}


void main()
{   
    if(isHDR == true){
        vec3 dir = normalize(texCoords);
        vec2 uv = CartesianToEquirectangular(dir);
        fragColor = texture(sampler2D(hdrSkybox), uv);
    }
    else {
        fragColor = texture(skybox, texCoords);
    }
}