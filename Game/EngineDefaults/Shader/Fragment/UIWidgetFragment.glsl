#version 460

#extension GL_ARB_bindless_texture : require

in vec2 uv0;
out vec4 color;

layout(location = 3) uniform vec3 fontColor;
layout(location = 4) uniform uvec2 fontTexture;

void main()
{
    vec4 sampled = texture(sampler2D(fontTexture), uv0);
    color = vec4(fontColor.r, fontColor.g, fontColor.b, sampled.r);
}
