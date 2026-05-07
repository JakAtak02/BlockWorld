#pragma once

#include <glm/glm.hpp>

class Frustum
{
public:
    void update(const glm::mat4& projectionView);

    bool isBoxVisible(
        const glm::vec3& min,
        const glm::vec3& max
    ) const;

private:
    glm::vec4 m_planes[6];
};