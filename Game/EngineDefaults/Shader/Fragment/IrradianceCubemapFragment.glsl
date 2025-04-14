#version 460

#extension GL_ARB_bindless_texture : require

#define NUM_SAMPLES 2048
#define PI 3.14159265359

out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube skybox;

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
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

vec3 hemisphereSample(const float u1, const float u2)
{
    const float phi = u1 * 2.0 * PI;
    const float r = sqrt(u2);
    return vec3(r * cos(phi), r * sin(phi), sqrt(1 - u2));
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
    vec3 irradiance = vec3(0.0);
    const vec3 normal = normalize(texCoords);
    const mat3 tangentSpace = computeTangetSpace(normal);

    for(int i=0; i< NUM_SAMPLES; ++i)
    {
        const vec2 rand_value = hammersley2D(i, NUM_SAMPLES);
        vec3 L = hemisphereSample(rand_value[0], rand_value[1]);
        const float cosTheta = L.z;
		const float pdf = cosTheta / PI;
		const float lod = computeLod(pdf, NUM_SAMPLES, 512);
        L = tangentSpace * L;
        const vec3 Li = textureLod(skybox, L, lod).rgb;
        irradiance += Li;
    }

    fragColor = vec4(irradiance * (1.0 / float(NUM_SAMPLES)), 1.0);
}