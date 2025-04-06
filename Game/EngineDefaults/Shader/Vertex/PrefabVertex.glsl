#version 460

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform CameraMatrices {
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

uniform mat4 modelMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
}
