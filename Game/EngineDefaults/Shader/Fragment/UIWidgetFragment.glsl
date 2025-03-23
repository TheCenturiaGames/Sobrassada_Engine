#version 460
in vec2 TexCoords;
out vec4 color;

uniform sampler2D fontTexture;

void main()
{
    vec4 sampled = texture(fontTexture, TexCoords);
    color = vec4(1.0, 1.0, 1.0, sampled.r);
}
