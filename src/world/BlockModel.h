#pragma once

#include <string>

#include <glm/glm.hpp>

struct BlockModel
{
    std::string parent = "block/cube";

    glm::vec3 guiRotation =
    {
        -28.0f,
        -38.0f,
        0.0f
    };

    float guiScale = 34.0f;
};