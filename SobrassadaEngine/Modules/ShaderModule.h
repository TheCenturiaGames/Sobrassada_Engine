#pragma once

#include "Module.h"

class ShaderModule : public Module
{
  public:
    ShaderModule();
    ~ShaderModule() override;

    bool Init() override;
    bool ShutDown() override;

    unsigned int CreateShaderProgram(const char* vertexPath, const char* fragmentPath);
    void DeleteProgram(unsigned int programID);

    int GetProgramID() const { return program; }

  private:
    char* LoadShaderSource(const char* shaderPath);
    unsigned int CompileShader(unsigned int shaderType, const char* source);
    unsigned int CreateProgram(unsigned int vertexShader, unsigned fragmentShader);

  private:
    int program = -1;
};
