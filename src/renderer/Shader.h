#pragma once

#include <string>

class Shader
{
public:
    Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();

    void bind() const;

    void setMat4(const char* name, const float* value) const;

private:
    unsigned int m_program;
};