#version 460

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.3));
uniform vec3 lightColor = vec3(1.0);
uniform vec3 objectColor = vec3(1.0);

uniform sampler2D albedoTex;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);

    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    vec3 result = (albedo * objectColor) * diff * lightColor;

    FragColor = vec4(result, 1.0);
}
