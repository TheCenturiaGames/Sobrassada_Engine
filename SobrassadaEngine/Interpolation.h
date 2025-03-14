#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/Quat.h"

class Interpolation {
public:
    static float Lerp(const float a, const float b, const float t);
    static float2 Lerp(const float2& a, const float2& b, const float t);
    static float3 Lerp(const float3& a, const float3& b, const float t);
    static float4 Lerp(const float4& a, const float4& b, const float t);
    static Quat Lerp(const Quat& a, const Quat& b, const float t);
};