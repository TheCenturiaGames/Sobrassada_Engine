#version 460 core

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
    vec3 albedo = texture(albedoTex, TexCoords).rgb;
    vec3 norm = normalize(Normal);

    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * albedo;

    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * albedo;

    vec3 fillDir = normalize(vec3(0.5, 1.0, 0.3));
    float fillDiff = max(dot(norm, -fillDir), 0.0);
    vec3 fill = 0.3 * fillDiff * albedo;

    vec3 finalColor = ambient + diffuse + fill;
    finalColor = pow(finalColor, vec3(1.0 / 2.2)); // gamma correct

    FragColor = vec4(finalColor, 1.0);

}
