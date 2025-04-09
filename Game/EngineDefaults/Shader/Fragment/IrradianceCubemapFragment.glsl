#version 460

#extension GL_ARB_bindless_texture : require

out vec4 fragColor;

in vec3 texCoords;
uniform samplerCube environment;

uniform samplerCube skybox;

void main()
{    
    vec3 normal = normalize(texCoords);

    vec3 irradiance = texture(skybox, normal).rgb;

    fragColor = vec4(irradiance, 1.0);
}