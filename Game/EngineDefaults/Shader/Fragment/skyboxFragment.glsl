#version 460

#extension GL_ARB_bindless_texture : require

in vec3 texCoords;

uniform samplerCube skybox;

out vec4 fragColor;


void main()
{    
    fragColor = texture(skybox, texCoords);
}