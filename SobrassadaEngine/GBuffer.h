#pragma once
class GBuffer
{
  public:
    GBuffer(int width, int height);
    ~GBuffer();

    
    void Bind();
    void Unbind();
    void Resize(int width, int height);
    void CheckResize();

  private:
    bool shouldResize = false;
    int screenHeight  = 0;
    int screenWidth  = 0;

    int gBufferObject = 0;
};
