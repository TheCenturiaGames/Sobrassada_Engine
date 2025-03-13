#pragma once

#include "Module.h"

#include "SDL.h"

class WindowModule : public Module
{
  public:
    WindowModule();
    ~WindowModule() override;

    bool Init() override;
    bool ShutDown() override;

    void WindowResized(const unsigned int width, const unsigned int height);

    int GetWidth() const { return windowWidth; }
    int GetHeight() const { return windowHeight; }
    SDL_DisplayMode& GetDesktopDisplayMode();
    float GetBrightness() const { return SDL_GetWindowBrightness(window); }

    bool GetFullscreen() const { return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN; }
    bool GetFullDesktop() const { return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP; }
    bool GetBorderless() const { return SDL_GetWindowFlags(window) & SDL_WINDOW_BORDERLESS; }
    bool GetResizable() const { return SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE; }
    bool GetVsync() const { return vsync; }

    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    void SetBrightness(const float brightness) const { SDL_SetWindowBrightness(window, brightness); }

    bool SetFullscreen(bool fullscreen) const { return SDL_SetWindowFullscreen(window, fullscreen); }
    void SetResizable(bool resizable) const { SDL_SetWindowResizable(window, resizable ? SDL_TRUE : SDL_FALSE); }
    void SetBorderless(bool borderless) const { SDL_SetWindowBordered(window, borderless ? SDL_FALSE : SDL_TRUE); }
    bool SetFullDesktop(bool fullDesktop) const
    {
        return SDL_SetWindowFullscreen(window, fullDesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }

    void SetVsync(bool newVSYNC)
    {
        vsync = newVSYNC;
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);
    }

    void UpdateProjectNameInWindowTitle(const std::string& newProjectName) const;

  public:
    SDL_Window* window         = nullptr;
    SDL_Surface* screenSurface = nullptr;

  private:
    int windowWidth  = SCREEN_WIDTH;
    int windowHeight = SCREEN_HEIGHT;
    bool vsync       = false;
    SDL_DisplayMode displayMode;
};
