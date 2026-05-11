#pragma once

#include <string>

class Shader
{
public:
    Shader(
        const char* vertexSrc,
        const char* fragmentSrc
    );

    ~Shader();

    static Shader fromFiles(
        const std::string& vertexPath,
        const std::string& fragmentPath
    );

    void bind() const;

    void setMat4(
        const char* name,
        const float* value
    ) const;

private:
    unsigned int m_program = 0;

private:
    static std::string loadFile(
        const std::string& path
    );
};