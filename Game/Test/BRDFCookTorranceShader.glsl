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

#define PI 3.14159265359


// Lights data structures
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


// Lights SSBOs
layout(std140, binding = 2) uniform Ambient
{
	vec4 ambient_color;		// rbg = color & alpha = intensity
};

layout(std140, binding = 3) uniform Directional
{
    vec4 directional_dir;
    vec4 directional_color;
};

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


// Material UBO
layout(std140, binding = 1) uniform Material
{
    vec4 diffColor;
    vec3 specColor;
    float shininess;   
    bool shininessInAlpha;  
    bool hasNormal;
};


float PointLightAttenuation(const int index) 
{
	float distance = length(pos - pointLights[index].position.xyz);
	return pow(max(1 - pow((distance / pointLights[index].position.w), 4), 0), 2) / (pow(distance, 2) + 1);
}

float SpotLightAttenuation(const int index)
{
	vec3 dirLight = normalize(spotLights[index].direction);
	float distance = dot(pos - spotLights[index].position.xyz, dirLight);
	float Fatt = pow(max(1 - pow((distance / spotLights[index].position.w), 4), 0), 2) / (pow(distance, 2) + 1);

	vec3 D = normalize(pos - spotLights[index].position.xyz);
	float C = dot(D, dirLight);

	float cosInner = cos(radians(spotLights[index].innerAngle));
	float cosOuter = cos(radians(spotLights[index].outerAngle));
	float Catt = 0;
	if (C > cosInner) Catt = 1;
	else if (C < cosInner && C > cosOuter) Catt = (C - cosOuter) / (cosInner - cosOuter);

	return Fatt * Catt;
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

vec3 RenderLight(vec3 L, vec3 N, vec4 specTexColor, vec3 texColor, vec3 Li, float NdotL, float alpha, float roughness)
 {
    float shininessValue;
	if(shininessInAlpha) shininessValue = exp2(alpha * 7 + 1);
	else shininessValue = shininess;

    float normalization = (shininessValue + 2.0) / (2.0 * 3.1415926535);
    vec3 V = normalize(cameraPos - pos);
    vec3 R = reflect(L, N);
    float VR = pow(max(dot(V, R), 0.0f), shininessValue);

    vec3 H = normalize(V + L);
    float NdotV = dot(N, V);
    float NdotH = dot(N, H);

    vec3 BaseColor = diffColor.rgb * texColor;
    //Cd = BaseColor(1 - metalness);
    //RF0 = mix(0.04, BaseColor, metalness);
    
    vec3 RF0 = specTexColor.rgb;
    float cosTheta = max(dot(L, H), 0.0);
    vec3 fresnel = RF0 + (1 - RF0) * pow(1 - cosTheta, 5);

    float visibility = VisibilityFunction(NdotL, NdotV, roughness);
    float GGX = GGXNormalDistribution(NdotH, roughness);


    vec3 diffspec = (texColor*(1-RF0) + (1/4)*fresnel*visibility*GGX) * Li * (NdotL);

    vec3 diffuse = (1.0 - RF0) / 3.1415926535 * BaseColor * Li * NdotL;
    vec3 specular = normalization * specColor.rgb * specTexColor.rgb * VR * Li * fresnel;
    return diffuse + specular;
}

vec3 RenderPointLight(const int index, const vec3 N, vec4 specTexColor, const vec3 texColor, const float alpha, float roughness)
{
	float attenuation = PointLightAttenuation(index);
	vec3 L = normalize(pos - pointLights[index].position.xyz);
	vec3 Li = pointLights[index].color.rgb * pointLights[index].color.a * attenuation;
	float NdotL = dot(N, -L);

	if (NdotL > 0 && attenuation > 0) return RenderLight(L, N, specTexColor, texColor, Li, NdotL, alpha, roughness);
	else return vec3(0);	
}

vec3 RenderSpotLight(const int index, const vec3 N, vec4 specTexColor, const vec3 texColor, const float alpha, float roughness)
{
	float attenuation = SpotLightAttenuation(index);
	vec3 L = normalize(pos - spotLights[index].position.xyz);
	vec3 Li = spotLights[index].color.rgb * spotLights[index].color.a * attenuation;
	float NdotL = dot(N, -L);

	if (NdotL > 0 && attenuation > 0) return RenderLight(L, N, specTexColor, texColor, Li, NdotL, alpha, roughness);
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
    vec3 texColor = pow(texture(diffuseTexture, uv0).rgb, vec3(2.2f));
    vec4 specTexColor = texture(specularTexture, uv0);
    float alpha = specTexColor.a;

    alpha = specTexColor.a;

    // Ambient light
    vec3 ambient = ambient_color.rgb * ambient_color.a;
    vec3 hdr = ambient * texColor;

    vec3 N = normalize(normal);
    // Retrive normal for normal map
    if (hasNormal) {
        mat3 space = CreateTBN();
        vec3 texNormal = (texture(normal_map, uv0).xyz*2.0-1.0);
        vec3 final_normal = space * texNormal;
        N = normalize(final_normal);
    }

    // Point Lights
    for (int i = 0; i < pointLightsCount; ++i)
	{
		hdr += RenderPointLight(i, N, specTexColor, texColor, alpha, roughness);
	}

    //Spot Lights
    for (int i = 0; i < spotLightsCount; ++i)
	{
		hdr += RenderSpotLight(i, N, specTexColor, texColor, alpha, roughness);
	}

    // Directional light
    vec3 lightColor = directional_color.rgb * directional_color.a;
    vec3 L = normalize(directional_dir.xyz);
    float NdotL = dot(N, -L);
    if (NdotL > 0)
    {
		hdr += RenderLight(L, N, specTexColor, texColor, lightColor, NdotL, alpha, roughness);
    }

    vec3 ldr = hdr.rgb / (hdr.rgb + vec3(1.0));
    ldr = pow(hdr, vec3(1/2.2));
    outColor = vec4(ldr, alpha);
}