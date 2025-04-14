#version 460

#extension GL_ARB_bindless_texture : require

#define PI 3.14159265359

in vec3 pos;
in vec2 uv0;
in vec3 normal;
in vec4 tangent;
flat in int instance_index;

out vec4 outColor;

uniform vec3 cameraPos;

struct Material
{
    vec4 diffColor;
    vec3 specColor;
    float shininess;
    bool shininessInAlpha;
    float metallicFactor;
    float roughnessFactor;
    uvec2 diffuseTex;
    uvec2 specularTex;
    uvec2 metallicTex;
    uvec2 normalTex;
};

struct DirectionalLight
{
    vec3 direction;
    vec4 color;             // rbg = color & alpha = intensity
};

struct PointLight
{
	vec4 position;		// xyz = position & w = range
	vec4 color;			// rgb = color & alpha = intensity 
};

struct SpotLight
{
	vec4 position;		// xyz = position & w = range
	vec4 color;			// rgb = color & alpha = intensity
	vec3 direction;	
	float innerAngle;
	float outerAngle;
};


// UBOs
layout(std140, binding = 2) uniform Ambient
{
	vec4 ambient_color;		// rbg = color & alpha = intensity
    samplerCube cubemapIrradiance;
    samplerCube cubemapPrefiltered;
    uvec2 environmentBRDF;
    int numLevels;
};

layout(std140, binding = 3) uniform Directional
{
    vec4 directional_dir;
    vec4 directional_color;
};

// SSBOs
readonly layout(std430, binding = 4) buffer PointLights
{
	int pointLightsCount;
	PointLight pointLights[];
};

readonly layout(std430, binding = 5) buffer SpotLights
{
	int spotLightsCount;
	SpotLight spotLights[];
};

readonly layout(std430, binding = 11) buffer Materials {
    Material materials[];
};


float PointLightAttenuation(const int index) 
{
	float distance = length(pos - pointLights[index].position.xyz);
	return pow(max(1 - pow((distance / pointLights[index].position.w), 4.0), 0.0), 2.0) / (pow(distance, 2.0) + 1.0);
}

float SpotLightAttenuation(const int index)
{
	vec3 dirLight = normalize(spotLights[index].direction);
	float distance = dot(pos - spotLights[index].position.xyz, dirLight);
	float Fatt = pow(max(1 - pow((distance / spotLights[index].position.w), 4.0), 0.0), 2.0) / (pow(distance, 2.0) + 1.0);

	vec3 D = normalize(pos - spotLights[index].position.xyz);
	float C = dot(D, dirLight);

	float cosInner = cos(radians(spotLights[index].innerAngle));
	float cosOuter = cos(radians(spotLights[index].outerAngle));
	float Catt = 0;
	if (C > cosInner) Catt = 1;
	else if (C < cosInner && C > cosOuter) Catt = (C - cosOuter) / (cosInner - cosOuter);

	return Fatt * Catt;
}

vec3 GetAmbientLight(in vec3 normal, in vec3 R, float NdotV, float roughness, in vec3 diffuseColor, in vec3 specularColor)
{
    vec3 irradiance = texture(cubemapIrradiance, normal).rgb;

    //vec3 radiance = texture(cubemapPrefiltered, R).rgb;
    vec3 radiance = textureLod(cubemapPrefiltered, R, roughness * (numLevels - 1)).rgb;
    vec2 fab = texture(sampler2D(environmentBRDF), vec2(NdotV, roughness)).rg;

    vec3 diffuse = (diffuseColor * (1 - specularColor));

    return diffuse * irradiance + radiance * (specularColor * fab.x + fab.y);
}

float VisibilityFunction(float NdotL, float NdotV, float roughness){
    float G1 = NdotL * (NdotV * (1 - roughness) + roughness);
    float G2 = NdotV * (NdotL * (1 - roughness) + roughness);
    return 0.5/(G1 + G2);
}

float GGXNormalDistribution(float NdotH, float roughness){
    float roughness2 = roughness * roughness;
    float NdotH2 = NdotH * NdotH;
    float denomTerm = NdotH2 * (roughness2 - 1) + 1;
    float denominator  = PI * denomTerm * denomTerm;
    return roughness2/denominator;
}

vec3 RenderLight(vec3 L, vec3 N, vec3 Cd, vec3 Li, float NdotL, float alpha, float roughness, vec3 RF0)
 {
    vec3 V = normalize(cameraPos - pos);
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0001);
    float NdotH = max(dot(N, H), 0.0001);
    
    float cosTheta = max(dot(L, H), 0.0001);
    vec3 fresnel = RF0 + (1 - RF0) * pow(1 - cosTheta, 5);

    float visibility = VisibilityFunction(NdotL, NdotV, roughness);
    float GGX = GGXNormalDistribution(NdotH, roughness);

    vec3 diffspec = (Cd * (1-RF0) + 0.25 * fresnel * visibility * GGX) * Li * NdotL;

    return diffspec;
}

vec3 RenderPointLight(const int index, const vec3 N, const vec3 Cd, const float alpha, float roughness, vec3 RF0)
{
	float attenuation = PointLightAttenuation(index);
	vec3 L = -normalize(pos - pointLights[index].position.xyz);
	vec3 Li = pointLights[index].color.rgb * pointLights[index].color.a * attenuation;
	float NdotL = dot(N, L);

	if (NdotL > 0 && attenuation > 0) return RenderLight(L, N, Cd, Li, NdotL, alpha, roughness, RF0);
	else return vec3(0);	
}

vec3 RenderSpotLight(const int index, const vec3 N, const vec3 Cd, const float alpha, float roughness, vec3 RF0)
{
	float attenuation = SpotLightAttenuation(index);
	vec3 L = -normalize(pos - spotLights[index].position.xyz);
	vec3 Li = spotLights[index].color.rgb * spotLights[index].color.a * attenuation;
	float NdotL = dot(N, L);

	if (NdotL > 0 && attenuation > 0) return RenderLight(L, N, Cd, Li, NdotL, alpha, roughness, RF0);
	else return vec3(0);
}

 mat3 CreateTBN()
 {
    vec3 T = normalize(vec3(tangent));
    vec3 N = normalize(normal);
    vec3 B = tangent.w * cross(N, T);
    return mat3(T, B, N);
 }

void main()
{
    Material mat = materials[instance_index];
    vec3 texColor = pow(texture(sampler2D(mat.diffuseTex), uv0).rgb, vec3(2.2f));
    vec4 metallicRoughnessTexColor = pow(texture(sampler2D(mat.metallicTex), uv0), vec4(2.2));
    float alpha = metallicRoughnessTexColor.a;

    vec3 N = normalize(normal);
    // Retrive normal for normal map
    if (mat.normalTex.r != 0 || mat.normalTex.g != 0) {
        mat3 space = CreateTBN();
        vec3 texNormal = (texture(sampler2D(mat.normalTex), uv0).xyz*2.0-1.0);
        vec3 final_normal = space * texNormal;
        N = normalize(final_normal);
    }

    float roughness = mat.roughnessFactor * metallicRoughnessTexColor.y;
    //roughness = roughness * roughness;
    float metallic = mat.metallicFactor * metallicRoughnessTexColor.z;

    vec3 V = normalize(cameraPos - pos);
    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0001);

    // Ambient light
    vec3 BaseColor = materials[instance_index].diffColor.rgb * texColor;
    vec3 Cd = BaseColor * (1 - metallic);
    vec3 RF0 = mix(vec3(0.04), BaseColor, metallic);

    //vec3 ambient = ambient_color.rgb * ambient_color.a;
    vec3 ambient = GetAmbientLight(N, R, NdotV, roughness, Cd, RF0);
    vec3 hdr = ambient;

    // Point Lights
    for (int i = 0; i < pointLightsCount; ++i)
	{
		hdr += RenderPointLight(i, N, Cd, alpha, roughness, RF0);
	}

    //Spot Lights
    for (int i = 0; i < spotLightsCount; ++i)
	{
		hdr += RenderSpotLight(i, N, Cd, alpha, roughness, RF0);
	}

    // Directional light
    vec3 lightColor = directional_color.rgb * directional_color.a;
    vec3 L = -normalize(directional_dir.xyz);
    float NdotL = max(dot(N, L), 0.001f);
    if (NdotL > 0)
    {
		hdr += RenderLight(L, N, Cd, lightColor, NdotL, alpha, roughness, RF0);
    }

    vec3 ldr = hdr.rgb / (hdr.rgb + vec3(1.0));
    ldr = pow(hdr, vec3(1.0/2.2));
    outColor = vec4(ldr, alpha);
}