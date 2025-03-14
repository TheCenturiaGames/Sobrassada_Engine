#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/Quat.h"

class Interpolation {
public:
    template <typename T>
    static T Lerp(const T& a, const T& b, const float t) {
        return a + t * (b - a);
    }

    //Quat are spherical, so we use a spherical linear interpolation instead of linear one
    static Quat Lerp(const Quat& a, const Quat& b, float t) {
        return a.Slerp(b, t);
    }
};