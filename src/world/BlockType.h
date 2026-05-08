#pragma once

#include "world/BlockModel.h"

#include <cstdint>
#include <string>

struct BlockType
{
    uint16_t numericId = 0;

    std::string id;
    std::string displayName;

    std::string modelPath;

    // Used for normal one-texture blocks like stone/dirt/sand.
    std::string texturePath;

    // Used for blocks like grass/logs/etc.
    std::string topTexturePath;
    std::string sideTexturePath;
    std::string bottomTexturePath;

    float topTextureIndex = 0.0f;
    float sideTextureIndex = 0.0f;
    float bottomTextureIndex = 0.0f;

    bool solid = true;
    bool opaque = true;

    BlockModel model;
};