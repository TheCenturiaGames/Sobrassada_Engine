#version 460
in vec2 uv0;
out vec4 color;

layout(binding = 0) uniform sampler2D fontTexture;
layout(binding = 1) uniform vec3 fontColor;

void main()
{
    vec4 sampled = texture(fontTexture, uv0);
    color = vec4(sampled.r, 0, 0, sampled.r);
}
