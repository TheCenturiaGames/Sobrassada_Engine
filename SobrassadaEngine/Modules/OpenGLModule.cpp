#include "OpenGLModule.h"

#include "Application.h"
#include "Framebuffer.h"
#include "GBuffer.h"
#include "WindowModule.h"

#include "glew.h"
#include <Windows.h>

OpenGLModule::OpenGLModule()
{
}

OpenGLModule::~OpenGLModule()
{
}

bool OpenGLModule::Init()
{
    GLOG("Creating Renderer context");

    context    = SDL_GL_CreateContext(App->GetWindowModule()->window);
    GLenum err = glewInit();

    GLOG("Using Glew %s", glewGetString(GLEW_VERSION));

    GLOG("Vendor: %s", glGetString(GL_VENDOR));
    GLOG("Renderer: %s", glGetString(GL_RENDERER));
    GLOG("OpenGL version supported %s", glGetString(GL_VERSION));
    GLOG("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_DEPTH_TEST); // Enable depth test
    glEnable(GL_CULL_FACE);  // Enable cull backward faces
    glFrontFace(GL_CCW);     // Enable conter clock wise backward faces
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Enable seamless cubemap 
    
    // stencil op
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    framebuffer = new Framebuffer(App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight(), true);
    gBuffer     = new GBuffer(App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight());

    WindowModule* windowModule = App->GetWindowModule();
    windowModule->SetVsync(windowModule->GetVsync());

    int x;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &x);

    return true;
}

update_status OpenGLModule::PreUpdate(float deltaTime)
{
    int CurrentWidth  = 0;
    int CurrentHeight = 0;
    SDL_GetWindowSize(App->GetWindowModule()->window, &CurrentWidth, &CurrentHeight);

#ifndef GAME
    framebuffer->Bind();
#endif

    if (CurrentWidth && CurrentHeight)
    {
#ifndef GAME
        glViewport(0, 0, framebuffer->GetTextureWidth(), framebuffer->GetTextureHeight());
#else
        glViewport(0, 0, CurrentWidth, CurrentHeight);
#endif
        glClearColor(clearColorRed, clearColorGreen, clearColorBlue, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    drawCallsCount     = 0;
    verticesCount      = 0;
    trianglesPerSecond = 0;

    return UPDATE_CONTINUE;
}

update_status OpenGLModule::Update(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status OpenGLModule::PostUpdate(float deltaTime)
{
    framebuffer->CheckResize();
    gBuffer->CheckResize();

    SDL_GL_SwapWindow(App->GetWindowModule()->window);

    return UPDATE_CONTINUE;
}

bool OpenGLModule::ShutDown()
{
    GLOG("Destroying renderer");

    SDL_GL_DeleteContext(App->GetWindowModule()->window);
    delete framebuffer;
    return true;
}

void OpenGLModule::DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    glDrawElements(mode, count, type, indices);
    drawCallsCount++;
}

void OpenGLModule::DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    glDrawArrays(mode, first, count);
    drawCallsCount++;
}

void OpenGLModule::SetRenderWireframe(bool renderWireframe)
{
    if (renderWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLModule::SetDepthTest(bool enable)
{
    if (enable) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}

void OpenGLModule::SetFaceCull(bool enable)
{
    if (enable) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
}

void OpenGLModule::SetDepthFunc(bool enable)
{
    if (enable) glDepthFunc(GL_LESS);
    else glDepthFunc(GL_ALWAYS);
}

void OpenGLModule::SetFrontFaceMode(int mode)
{
    glFrontFace(mode);
}
