#include "renderer/Shader.h"

#include <glad/glad.h>
#include <iostream>

static unsigned int compile(unsigned int type, const char* src)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char info[512];
        glGetShaderInfoLog(id, 512, nullptr, info);
        std::cout << "Shader compile error:\n" << info << std::endl;
    }

    return id;
}

Shader::Shader(const char* vertexSrc, const char* fragmentSrc)
{
    unsigned int vs = compile(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    int success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char info[512];
        glGetProgramInfoLog(m_program, 512, nullptr, info);
        std::cout << "Shader link error:\n" << info << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader()
{
    glDeleteProgram(m_program);
}

void Shader::bind() const
{
    glUseProgram(m_program);
}

void Shader::setMat4(const char* name, const float* value) const
{
    int loc = glGetUniformLocation(m_program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, value);
}