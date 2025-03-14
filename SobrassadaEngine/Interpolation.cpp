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

/*
#include <Interpolation.h>
void TestInterpolation() {
    float a = 0.0f, b = 10.0f;
    float f1lerp = Interpolation::Lerp(a, b, 0.5f);
    GLOG("Lerp float(0, 10, 0.5): %f", f1lerp);

    float3 v1(0, 0, 0), v2(10, 10, 10);
    float3 vLerp = Interpolation::Lerp(v1, v2, 0.5f);
    GLOG("Lerp float3(0,0,0 -> 10,10,10, 0.5): (%f, %f, %f)", vLerp.x, vLerp.y, vLerp.z);

    Quat q1 = Quat::identity;
    Quat q2 = Quat::RotateAxisAngle(float3(0, 1, 0), 90.0f * (M_PI / 180.0f));
    Quat qLerp = Interpolation::Lerp(q1, q2, 0.5f);

    float3 euler = qLerp.ToEulerXYZ() * (180.0f / M_PI);
    GLOG("Slerp quaternion(Identity -> 90° Y, 0.5): (%f, %f, %f) degrees", euler.x, euler.y, euler.z);
}*/