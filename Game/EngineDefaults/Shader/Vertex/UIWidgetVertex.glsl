#version 460
layout (location = 0) in vec2 vertexPos;
layout (location = 1) in vec2 texCoords;

out vec2 uv0;

layout(location=0) uniform mat4 model;
layout(location=1) uniform mat4 view;
layout(location=2) uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(vertexPos, 0.0, 1.0);
    uv0 = texCoords;
}
