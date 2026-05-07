#pragma once

#include <glm/glm.hpp>

class DebugRenderer
{
public:
    DebugRenderer();
    ~DebugRenderer();

    void drawChunkBorder(
        const glm::vec3& min,
        const glm::vec3& max,
        const glm::mat4& projection,
        const glm::mat4& view
    );

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_shaderProgram = 0;

private:
    unsigned int compileShader(unsigned int type, const char* source);
};