#version 460

#extension GL_ARB_bindless_texture : require

in vec3 texCoords;

layout(binding = 1) uniform samplerCube skybox;

out vec4 fragColor;


void main()
{    
    fragColor = texture(skybox, texCoords);
}