#version 460

#extension GL_ARB_bindless_texture : require

#define NUM_SAMPLES 2048
#define PI 3.14159265359

out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform float roughness;

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2D(const uint i, const uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

vec3 hemisphereSampleGGX(const float u1, const float u2, const float rough)
{
    const float a = rough * rough;
    const float phi = 2.0 * PI * u1;
    const float cos_theta = sqrt((1.0 - u2)/(u2 * (a * a - 1) + 1));
    const float sin_theta = sqrt(1 - cos_theta * cos_theta);
    
    // spherical to cartesian conversion
    vec3 dir;
    dir.x = cos(phi) * sin_theta;
    dir.y = sin(phi) * sin_theta;
    dir.z = cos_theta;
    return dir;
}

mat3 computeTangetSpace(const in vec3 normal)
{
    vec3 up = abs(normal.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    const vec3 right = normalize(cross(up, normal));
    up = cross(normal, right);
    return mat3(right, up, normal);
}

float computeLod(const float pdf, const int numSamples, const uint width)
{
	return max(0.5 * log2(6.0 * float(width) * float(width) / (float(numSamples) * pdf)), 0.0);
}

void main()
{
    const vec3 R = normalize(texCoords);
    const vec3 N = R, V = R;
    vec3 color = vec3(0.0);
    float weight = 0.0f;
    const mat3 tangentSpace = computeTangetSpace(N);

    for( int i = 0; i < NUM_SAMPLES; ++i)
    {
        const vec2 rand_value = hammersley2D(i, NUM_SAMPLES);
        vec3 H = hemisphereSampleGGX(rand_value[0], rand_value[1], roughness);
        const float cosTheta = H.z;
		const float ggxDenominator = cosTheta * cosTheta * (roughness * roughness - 1) + 1;
		const float ggx = (roughness * roughness) / (PI * ggxDenominator * ggxDenominator);
		const float pdf = ggx / 4.0;
		const float lod = computeLod(pdf, NUM_SAMPLES, 512);
        H = tangentSpace * H;
        const vec3 L = reflect(-V, H);
        const float NdotL = max(dot(N, L), 0.0);
        if( NdotL > 0 )
        {
            color += textureLod(skybox , L, lod).rgb * NdotL;
            weight += NdotL;
        }
    }

    fragColor = vec4(color / weight, 1.0);
}