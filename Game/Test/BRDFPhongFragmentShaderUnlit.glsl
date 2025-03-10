#version 460

in vec3 pos;
in vec2 uv0;
in vec3 normal;
in vec4 tangent;

out vec4 outColor;

layout(binding=0) uniform sampler2D diffuseTexture;
layout(binding=1) uniform sampler2D specularTexture;
layout(binding = 2) uniform sampler2D normal_map;

uniform vec3 cameraPos;

void main()
{
	outColor = texture2D(diffuseTexture, uv0);
}