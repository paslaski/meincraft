#pragma once

#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader() = default; // default constructor
    Shader(std::string vertexPath, std::string fragmentPath);
    ~Shader();

//    // delete copy constructor and assignment operators
//    Shader(const Shader& shader) = delete; // copy constructor
//    Shader& operator=( const Shader & ) = delete; // copy assignment operator
//    // enable move constructor and assignment operators
//    Shader(Shader&& shader) = default;
//    Shader& operator=(const Shader&& ) = default;

    void Bind();
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