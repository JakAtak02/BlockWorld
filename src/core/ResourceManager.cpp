#include "core/ResourceManager.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <json.hpp>

bool ResourceManager::initialize()
{
    if (!loadBlocks())
    {
        return false;
    }

    if (!loadBlockTextures())
    {
        return false;
    }

    m_blockRenderInfo =
        m_blockRegistry.buildRenderInfo();

    return true;
}

TextureArray& ResourceManager::getBlockTextureArray()
{
    return m_blockTextureArray;
}

const std::vector<BlockRenderInfo>&
ResourceManager::getBlockRenderInfo() const
{
    return m_blockRenderInfo;
}

bool ResourceManager::loadBlocks()
{
    return loadBlocksFromManifest(
        "resources/data/blocks.json"
    );
}

bool ResourceManager::loadBlocksFromManifest(
    const std::string& manifestPath
)
{
    std::ifstream file(manifestPath);

    if (!file.is_open())
    {
        std::cout
            << "Failed to open block manifest: "
            << manifestPath
            << std::endl;

        return false;
    }

    nlohmann::json data;
    file >> data;

    if (!data.contains("blocks"))
    {
        std::cout
            << "Block manifest missing 'blocks' array."
            << std::endl;

        return false;
    }

    for (const auto& blockPath : data["blocks"])
    {
        if (!blockPath.is_string())
        {
            std::cout
                << "Invalid block path in manifest."
                << std::endl;

            return false;
        }

        std::string path =
            blockPath.get<std::string>();

        if (!m_blockRegistry.loadBlockFromJson(path))
        {
            std::cout
                << "Failed to load block from manifest: "
                << path
                << std::endl;

            return false;
        }
    }

    return true;
}

bool ResourceManager::loadBlockTextures()
{
    std::vector<std::string> texturePaths;

    for (const std::string& path :
        m_blockRegistry.getTexturePaths())
    {
        texturePaths.push_back(
            "resources/" + path
        );
    }

    return m_blockTextureArray
        .loadArrayFromFiles(texturePaths);
}