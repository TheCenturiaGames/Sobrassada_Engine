#pragma once

#include "Module.h"

#include "glew.h"

class Framebuffer;

class OpenGLModule : public Module
{
  public:
    OpenGLModule();
    ~OpenGLModule() override;

    bool Init() override;
    update_status PreUpdate(float deltaTime) override;
    update_status Update(float deltaTime) override;
    update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
    {
        glDrawElements(mode, count, type, indices);
        drawCallsCount++;
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count)
    {
        glDrawArrays(mode, first, count);
        drawCallsCount++;
    }

    void AddTrianglesPerSecond(float meshTrianglesPerSecond) { trianglesPerSecond += meshTrianglesPerSecond; }
    void AddVerticesCount(int meshVertices) { verticesCount += meshVertices; }

    void* GetContext() const { return context; }
    float GetClearRed() const { return clearColorRed; }
    float GetClearGreen() const { return clearColorGreen; }
    float GetClearBlue() const { return clearColorBlue; }
    Framebuffer* GetFramebuffer() const { return framebuffer; }
    int GetDrawCallsCount() const { return drawCallsCount; }
    float GetTrianglesPerSecond() const { return trianglesPerSecond; }
    int GetVerticesCount() const { return verticesCount; }

    void SetDepthTest(bool enable)
    {
        if (enable) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
    }

    void SetFaceCull(bool enable)
    {
        if (enable) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);
    }

    void SetDepthFunc(bool enable)
    {
        if (enable) glDepthFunc(GL_LESS);
        else glDepthFunc(GL_ALWAYS);
    }

    void SetFrontFaceMode(int mode) { glFrontFace(mode); }
    void SetClearRed(float newValue) { clearColorRed = newValue; }
    void SetClearGreen(float newValue) { clearColorGreen = newValue; }
    void SetClearBlue(float newValue) { clearColorBlue = newValue; }

    void SetRenderWireframe(bool renderWireframe);

  private:
    void* context            = nullptr;
    Framebuffer* framebuffer = nullptr;
    float clearColorRed      = DEFAULT_GL_CLEAR_COLOR_RED;
    float clearColorGreen    = DEFAULT_GL_CLEAR_COLOR_GREEN;
    float clearColorBlue     = DEFAULT_GL_CLEAR_COLOR_BLUE;
    int drawCallsCount       = 0;
    float trianglesPerSecond = 0;
    int verticesCount        = 0;
};
