#pragma once

#include <vector>

class Mesh
{
public:
    static constexpr int FLOATS_PER_VERTEX = 9;

    Mesh(const std::vector<float>& vertices);
    ~Mesh();

    void draw() const;
    int getVertexCount() const;
    size_t getEstimatedMemoryUsage() const;

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_vertexCount = 0;
    size_t m_estimatedMemoryUsage = 0;
};