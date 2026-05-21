#include "renderer/Mesh.h"

#include <glad/glad.h>

Mesh::Mesh(const std::vector<float>& vertices)
{
    m_vertexCount = static_cast<int>(vertices.size() / FLOATS_PER_VERTEX);

    m_estimatedMemoryUsage =
        vertices.size() * sizeof(float);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // layout location 0: vec3 position
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        FLOATS_PER_VERTEX * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // layout location 1: vec2 texture coordinates
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        FLOATS_PER_VERTEX * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // layout location 2: float texture array layer index
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        FLOATS_PER_VERTEX * sizeof(float),
        (void*)(5 * sizeof(float))
    );
    glEnableVertexAttribArray(2);

    // layout location 3: vec3 tint color
    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        FLOATS_PER_VERTEX * sizeof(float),
        (void*)(6 * sizeof(float))
    );
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    if (m_vbo != 0)
        glDeleteBuffers(1, &m_vbo);

    if (m_vao != 0)
        glDeleteVertexArrays(1, &m_vao);
}

void Mesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
}

int Mesh::getVertexCount() const
{
    return m_vertexCount;
}

size_t Mesh::getEstimatedMemoryUsage() const
{
    return m_estimatedMemoryUsage;
}