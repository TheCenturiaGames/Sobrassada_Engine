#version 460
in vec2 uv0;
out vec4 color;

uniform sampler2D fontTexture;
layout(location=3) uniform vec3 fontColor;

void main()
{
    vec4 sampled = texture(fontTexture, uv0);
    color = vec4(fontColor.r, fontColor.g, fontColor.b, sampled.r);
}
