#pragma once

#include <glm/glm.hpp>

class BlockSelectionRenderer
{
public:
    BlockSelectionRenderer();
    ~BlockSelectionRenderer();

    void drawBlockOutline(
        const glm::ivec3& blockPosition,
        const glm::mat4& projection,
        const glm::mat4& view
    );

private:
    unsigned int compileShader(unsigned int type, const char* source);

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_shaderProgram = 0;
};