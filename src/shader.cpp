
#include <glad/glad.h>

#include <fstream>
#include <sstream>

#include "shader.h"
#include "debug.h"

Shader::Shader(std::string vertexPath, std::string fragmentPath) : m_ProgramId(0)
{
    // parse code from shader paths
    std::string vertexCode = ParseShader(vertexPath);
    std::string fragmentCode = ParseShader(fragmentPath);

    // create + compile shaders
    m_ProgramId = CreateShader(vertexCode, fragmentCode);
}

Shader::~Shader()
{
    GLCall( glDeleteProgram(m_ProgramId) );
}

void Shader::Bind() const
{
    GLCall( glUseProgram(m_ProgramId) );
}

void Shader::Unbind() const
{
    GLCall( glUseProgram(0) );
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall( int location = glGetUniformLocation(m_ProgramId, name.c_str()) );
    if (location == -1)
        std::cout << "No active uniform variable with name " << name << " found" << std::endl;

    m_UniformLocationCache[name] = location;

    return location;
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall( glUniform1i(GetUniformLocation(name), value) );
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall( glUniform1f(GetUniformLocation(name), value) );
}

void Shader::SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3)
{
    GLCall( glUniform4f(GetUniformLocation(name), f0, f1, f2, f3) );
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    GLCall( glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]) );
}

std::string Shader::ParseShader(std::string& shaderPath) {
    // load source code from provided path
    std::string shaderCode;
    std::ifstream shaderFile;

    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // open file
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();
        // close file handlers
        shaderFile.close();
        // convert stream into string
        shaderCode   = shaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    return shaderCode;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string &source)
{
    // type is GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
    GLCall( unsigned int id = glCreateShader(type) );
    const char* src = source.c_str();
    GLCall( glShaderSource(id, 1, &src, nullptr) );
    GLCall( glCompileShader(id) );

    // Error handling
    // wrap in ifdef DEBUG? note: should switch to ifndef NDEBUG later bc defined by standard
    int result;
    GLCall( glGetShaderiv(id, GL_COMPILE_STATUS, &result) );
    std::cout << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader compile status: " << result << std::endl;
    if ( result == GL_FALSE )
    {
        int length;
        GLCall( glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length) );
        char* message = (char*) alloca(length * sizeof(char));
        GLCall( glGetShaderInfoLog(id, length, &length, message) );
        std::cout
                << "Failed to compile "
                << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                << "shader"
                << std::endl;
        std::cout << message << std::endl;
        GLCall( glDeleteShader(id) );
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string &vertexShader, const std::string &fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall( glAttachShader(program, vs) );
    GLCall( glAttachShader(program, fs) );

    GLCall( glLinkProgram(program) );

    GLint program_linked;

    GLCall( glGetProgramiv(program, GL_LINK_STATUS, &program_linked) );
    std::cout << "Program link status: " << program_linked << std::endl;
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        GLCall( glGetProgramInfoLog(program, 1024, &log_length, message) );
        std::cout << "Failed to link program" << std::endl;
        std::cout << message << std::endl;
    }

    GLCall( glValidateProgram(program) );

    GLCall( glDeleteShader(vs) );
    GLCall( glDeleteShader(fs) );
}
