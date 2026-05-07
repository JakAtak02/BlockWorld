#pragma once

#include <string>

struct BlockType
{
    std::string id;
    std::string displayName;

    // Used for normal one-texture blocks like stone/dirt/sand.
    std::string texturePath;

    // Used for blocks like grass.
    std::string topTexturePath;
    std::string sideTexturePath;
    std::string bottomTexturePath;

    bool solid = true;
    bool opaque = true;
};