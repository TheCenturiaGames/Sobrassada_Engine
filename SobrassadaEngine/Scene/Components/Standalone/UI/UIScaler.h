#pragma once

enum class UIScaleMode
{
    MatchWidth,
    MatchHeight,
    Expand,
    Shrink,
    MatchWidthOrHeight
};

class UIScaler
{
  public:
    UIScaler(float refWidth, float refHeight, UIScaleMode mode, float matchFactor = 0.5f);

    float GetScale(float currentWidth, float currentHeight) const;

  private:
    float referenceWidth;
    float referenceHeight;
    UIScaleMode scaleMode;
    float matchFactor;
};
