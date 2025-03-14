#include "Interpolation.h"

float Interpolation::Lerp(const float a, const float b, const float t) {
    return a + t * (b - a);
}

float2 Interpolation::Lerp(const float2& a, const float2& b, const float t) {
    return a + t * (b - a);
}

float3 Interpolation::Lerp(const float3& a, const float3& b, const float t) {
    return a + t * (b - a);
}

float4 Interpolation::Lerp(const float4& a, const float4& b,const  float t) {
    return a + t * (b - a);
}

Quat Interpolation::Lerp(const Quat& a, const Quat& b, const float t) {
    return a.Slerp(b, t);
}