#pragma once

#include "Module.h"
#include "DebugUtils.h"

class ShaderModule : public Module
{
  public:
    ShaderModule();
    ~ShaderModule() override;

    bool Init() override;
    bool ShutDown() override;

    unsigned int CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
    void DeleteProgram(unsigned int programID);

    int GetSpecularGlossinessProgram() const
    {
        return debugShaderOptions[RENDER_LIGTHS] ? specularGlossinessProgram : specularGlossinessProgramUnlit;
    }
    int GetMetallicRoughnessProgram() const
    {
        return debugShaderOptions[RENDER_LIGTHS] ? metallicRoughnessProgram : metallicRoughnessProgramUnlit;
    }

  private:
    char* LoadShaderSource(const char* shaderPath);
    unsigned int CompileShader(unsigned int shaderType, const char* source);
    unsigned int CreateProgram(unsigned int vertexShader, unsigned fragmentShader);

  private:
    int specularGlossinessProgram = -1;
    int specularGlossinessProgramUnlit = -1;

    int metallicRoughnessProgram  = -1;
    int metallicRoughnessProgramUnlit  = -1;
};
