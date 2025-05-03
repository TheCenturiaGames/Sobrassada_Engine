#include "UIScaler.h"
#include <algorithm>

UIScaler::UIScaler(float refWidth, float refHeight, UIScaleMode mode, float matchFactor)
    : referenceWidth(refWidth), referenceHeight(refHeight), scaleMode(mode), matchFactor(matchFactor)
{
}

float UIScaler::GetScale(float currentWidth, float currentHeight) const
{
    float scaleX = currentWidth / referenceWidth;
    float scaleY = currentHeight / referenceHeight;

    switch (scaleMode)
    {
    case UIScaleMode::MatchWidth:
        return scaleX;
    case UIScaleMode::MatchHeight:
        return scaleY;
    case UIScaleMode::Expand:
        return std::max(scaleX, scaleY);
    case UIScaleMode::Shrink:
        return std::min(scaleX, scaleY);
    case UIScaleMode::MatchWidthOrHeight:
    default:
        return scaleX * (1.0f - matchFactor) + scaleY * matchFactor;
    }
}
