#include "renderer/Frustum.h"

#include <glm/glm.hpp>

static glm::vec4 normalizePlane(const glm::vec4& plane)
{
    float length = glm::length(glm::vec3(plane));

    if (length == 0.0f)
    {
        return plane;
    }

    return plane / length;
}

void Frustum::update(const glm::mat4& projectionView)
{
    glm::mat4 m = glm::transpose(projectionView);

    // Left
    m_planes[0] = normalizePlane(m[3] + m[0]);

    // Right
    m_planes[1] = normalizePlane(m[3] - m[0]);

    // Bottom
    m_planes[2] = normalizePlane(m[3] + m[1]);

    // Top
    m_planes[3] = normalizePlane(m[3] - m[1]);

    // Near
    m_planes[4] = normalizePlane(m[3] + m[2]);

    // Far
    m_planes[5] = normalizePlane(m[3] - m[2]);
}

bool Frustum::isBoxVisible(
    const glm::vec3& min,
    const glm::vec3& max
) const
{
    for (const glm::vec4& plane : m_planes)
    {
        glm::vec3 positiveVertex = min;

        if (plane.x >= 0.0f) positiveVertex.x = max.x;
        if (plane.y >= 0.0f) positiveVertex.y = max.y;
        if (plane.z >= 0.0f) positiveVertex.z = max.z;

        float distance =
            plane.x * positiveVertex.x +
            plane.y * positiveVertex.y +
            plane.z * positiveVertex.z +
            plane.w;

        if (distance < 0.0f)
        {
            return false;
        }
    }

    return true;
}