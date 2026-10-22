#include "engine/Renderer/Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "engine/Core/GLCall.h"
#include "engine/Core/Log.h"

namespace
{
struct ShaderStageGuard
{
    GLuint id = 0;

    ~ShaderStageGuard()
    {
        if (id != 0)
        {
            GLCall(glDeleteShader(id));
        }
    }
};
} // namespace

Shader::Shader(const std::string& vertPath, const std::string& fragPath)
{
    const std::string vertSource = ReadFile(vertPath);
    const std::string fragSource = ReadFile(fragPath);
    Init(vertSource.c_str(), fragSource.c_str());
}

Shader::Shader(const char* vertSrc, const char* fragSrc)
{
    Init(vertSrc, fragSrc);
}

void Shader::Init(const char* vertSrc, const char* fragSrc)
{
    ShaderStageGuard vertexStage;
    vertexStage.id = CompileStage(GL_VERTEX_SHADER, vertSrc, "VERTEX");

    ShaderStageGuard fragmentStage;
    fragmentStage.id = CompileStage(GL_FRAGMENT_SHADER, fragSrc, "FRAGMENT");

    GLuint program = 0;
    GLCall(program = glCreateProgram());
    GLCall(glAttachShader(program, vertexStage.id));
    GLCall(glAttachShader(program, fragmentStage.id));
    GLCall(glLinkProgram(program));

    GLint success = 0;
    GLCall(glGetProgramiv(program, GL_LINK_STATUS, &success));

    if (success != GL_TRUE)
    {
        GLint logLength = 0;
        GLCall(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength));

        std::vector<char> log(static_cast<size_t>(logLength > 1 ? logLength : 1), '\0');
        GLCall(glGetProgramInfoLog(program, logLength, nullptr, log.data()));

        GLCall(glDeleteProgram(program));

        throw std::runtime_error(std::string("Shader link failed (LINK):\n") + log.data());
    }

    GLCall(glDetachShader(program, vertexStage.id));
    GLCall(glDetachShader(program, fragmentStage.id));

    m_ID = program;
}

Shader::~Shader()
{
    if (m_ID != 0)
    {
        GLCall(glDeleteProgram(m_ID));
        m_ID = 0;
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_ID(other.m_ID), m_UniformCache(std::move(other.m_UniformCache))
{
    other.m_ID = 0;
    other.m_UniformCache.clear();
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        if (m_ID != 0)
        {
            GLCall(glDeleteProgram(m_ID));
        }

        m_ID = other.m_ID;
        m_UniformCache = std::move(other.m_UniformCache);

        other.m_ID = 0;
        other.m_UniformCache.clear();
    }
    return *this;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_ID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform(const std::string& name, int value)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniform1i(location, value));
    }
}

void Shader::SetUniform(const std::string& name, float value)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniform1f(location, value));
    }
}

void Shader::SetUniform(const std::string& name, const glm::vec2& value)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniform2fv(location, 1, glm::value_ptr(value)));
    }
}

void Shader::SetUniform(const std::string& name, const glm::vec3& value)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniform3fv(location, 1, glm::value_ptr(value)));
    }
}

void Shader::SetUniform(const std::string& name, const glm::vec4& value)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniform4fv(location, 1, glm::value_ptr(value)));
    }
}

void Shader::SetUniform(const std::string& name, const glm::mat3& value, bool transpose)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniformMatrix3fv(location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(value)));
    }
}

void Shader::SetUniform(const std::string& name, const glm::mat4& value, bool transpose)
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0)
    {
        GLCall(glUniformMatrix4fv(location, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(value)));
    }
}

std::string Shader::ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

GLuint Shader::CompileStage(GLenum type, const char* source, const char* stageName)
{
    GLuint shader = 0;
    GLCall(shader = glCreateShader(type));
    GLCall(glShaderSource(shader, 1, &source, nullptr));
    GLCall(glCompileShader(shader));

    GLint success = 0;
    GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

    if (success != GL_TRUE)
    {
        GLint logLength = 0;
        GLCall(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

        std::vector<char> log(static_cast<size_t>(logLength > 1 ? logLength : 1), '\0');
        GLCall(glGetShaderInfoLog(shader, logLength, nullptr, log.data()));

        GLCall(glDeleteShader(shader));

        throw std::runtime_error(std::string("Shader compilation failed (") + stageName + "):\n" + log.data());
    }

    return shader;
}

GLint Shader::GetUniformLocation(const std::string& name)
{
    const auto it = m_UniformCache.find(name);
    if (it != m_UniformCache.end())
    {
        return it->second;
    }

    GLint location = -1;
    GLCall(location = glGetUniformLocation(m_ID, name.c_str()));
    m_UniformCache.emplace(name, location);

    if (location < 0)
    {
        LOG_WARN("Uniform not found (or inactive): " << name);
    }

    return location;
}
