#pragma once

#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(std::string vertexPath, std::string fragmentPath);
    ~Shader();
    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
private:
    unsigned int m_ProgramId;
    std::unordered_map<std::string, int> m_UniformLocationCache;

    std::string ParseShader(std::string& shaderPath);
    int GetUniformLocation(const std::string& name);
    unsigned int CompileShader(unsigned int type, const std::string& source);
    unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
};