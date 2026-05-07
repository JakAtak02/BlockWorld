#include "world/BlockRegistry.h"

#include <fstream>
#include <iostream>
#include <json.hpp>

bool BlockRegistry::loadBlockFromJson(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "Failed to open block JSON: " << path << std::endl;
        return false;
    }

    nlohmann::json data;
    file >> data;

    BlockType block;
    block.id = data.value("id", "");
    block.displayName = data.value("display_name", "");
    block.solid = data.value("solid", true);
    block.opaque = data.value("opaque", true);

    if (data.contains("texture"))
    {
        block.texturePath = data.value("texture", "");

        block.topTexturePath = block.texturePath;
        block.sideTexturePath = block.texturePath;
        block.bottomTexturePath = block.texturePath;
    }
    else if (data.contains("textures"))
    {
        auto textures = data["textures"];

        block.topTexturePath = textures.value("top", "");
        block.sideTexturePath = textures.value("side", "");
        block.bottomTexturePath = textures.value("bottom", "");

        block.texturePath = block.sideTexturePath;
    }

    if (block.id.empty())
    {
        std::cout << "Block JSON missing id: " << path << std::endl;
        return false;
    }

    if (block.topTexturePath.empty() ||
        block.sideTexturePath.empty() ||
        block.bottomTexturePath.empty())
    {
        std::cout << "Block JSON missing texture path: " << path << std::endl;
        return false;
    }

    m_blocks[block.id] = block;

    std::cout << "Loaded block type: " << block.id << std::endl;

    return true;
}

const BlockType* BlockRegistry::getBlock(const std::string& id) const
{
    auto it = m_blocks.find(id);

    if (it == m_blocks.end())
    {
        return nullptr;
    }

    return &it->second;
}