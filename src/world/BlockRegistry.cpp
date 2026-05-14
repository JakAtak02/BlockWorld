#include "world/BlockRegistry.h"
#include "world/BlockModel.h"

#include <fstream>
#include <iostream>

#include <json.hpp>

bool BlockRegistry::loadBlockFromJson(
    const std::string& path
)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout
            << "Failed to open block JSON: "
            << path
            << std::endl;

        return false;
    }

    nlohmann::json data;
    file >> data;

    BlockType block;

    block.numericId = m_nextBlockId++;

    block.id = data.value("id", "");
    block.displayName = data.value("display_name", "");

    block.modelPath =
        data.value(
            "model",
            "resources/data/models/block/cube.json"
        );

    block.solid = data.value("solid", true);
    block.opaque = data.value("opaque", true);

    block.tinted = data.value("tinted", false);

    if (data.contains("texture"))
    {
        block.texturePath =
            data.value("texture", "");

        block.topTexturePath =
            block.texturePath;

        block.sideTexturePath =
            block.texturePath;

        block.bottomTexturePath =
            block.texturePath;
    }
    else if (data.contains("textures"))
    {
        auto textures = data["textures"];

        block.topTexturePath =
            textures.value("top", "");

        block.sideTexturePath =
            textures.value("side", "");

        block.bottomTexturePath =
            textures.value("bottom", "");

        block.texturePath =
            block.sideTexturePath;
    }

    if (block.id.empty())
    {
        std::cout
            << "Block JSON missing id: "
            << path
            << std::endl;

        return false;
    }

    if (block.topTexturePath.empty() ||
        block.sideTexturePath.empty() ||
        block.bottomTexturePath.empty())
    {
        std::cout
            << "Block JSON missing texture path: "
            << path
            << std::endl;

        return false;
    }

    auto registerTexture =
        [this](const std::string& texturePath) -> float
        {
            for (size_t i = 0; i < m_texturePaths.size(); i++)
            {
                if (m_texturePaths[i] == texturePath)
                {
                    return static_cast<float>(i);
                }
            }

            m_texturePaths.push_back(texturePath);

            return static_cast<float>(
                m_texturePaths.size() - 1
                );
        };

    block.sideTextureIndex =
        registerTexture(block.sideTexturePath);

    block.topTextureIndex =
        registerTexture(block.topTexturePath);

    block.bottomTextureIndex =
        registerTexture(block.bottomTexturePath);

    std::ifstream modelFile(block.modelPath);

    if (modelFile.is_open())
    {
        nlohmann::json modelData;
        modelFile >> modelData;

        if (modelData.contains("display"))
        {
            auto display = modelData["display"];

            if (display.contains("gui"))
            {
                auto gui = display["gui"];

                if (gui.contains("rotation"))
                {
                    auto rotation = gui["rotation"];

                    block.model.guiRotation =
                    {
                        rotation[0],
                        rotation[1],
                        rotation[2]
                    };
                }

                if (gui.contains("scale"))
                {
                    block.model.guiScale =
                        gui["scale"];
                }
            }
        }
    }

    m_blocks[block.id] = block;

    std::cout
        << "Loaded block type: "
        << block.id
        << " (numeric id "
        << block.numericId
        << ")"
        << std::endl;

    return true;
}

const BlockType* BlockRegistry::getBlock(
    const std::string& id
) const
{
    auto it = m_blocks.find(id);

    if (it == m_blocks.end())
    {
        return nullptr;
    }

    return &it->second;
}

const std::vector<std::string>&
BlockRegistry::getTexturePaths() const
{
    return m_texturePaths;
}

std::vector<BlockRenderInfo>
BlockRegistry::buildRenderInfo() const
{
    std::vector<BlockRenderInfo> renderInfo;

    renderInfo.resize(m_blocks.size());

    for (const auto& [id, block] : m_blocks)
    {
        size_t index =
            static_cast<size_t>(
                block.numericId - 1
                );

        glm::vec3 tintColor =
        {
            1.0f,
            1.0f,
            1.0f
        };

        if (block.tinted)
        {
            if (block.id == "blockworld:grass_block")
            {
                tintColor =
                {
                    0.48f,
                    0.84f,
                    0.36f
                };
            }
            else if (block.id == "blockworld:oak_leaves")
            {
                tintColor =
                {
                    0.42f,
                    0.76f,
                    0.34f
                };
            }
            else if (block.id == "blockworld:flowing_water")
            {
                tintColor =
                {
                    0.52f,
                    0.72f,
                    0.92f
                };
            }
        }

        BlockRenderInfo info;

        info.sideTextureIndex =
            block.sideTextureIndex;

        info.topTextureIndex =
            block.topTextureIndex;

        info.bottomTextureIndex =
            block.bottomTextureIndex;

        info.tinted =
            block.tinted;

        info.tintColor =
            tintColor;

        info.guiRotation =
            block.model.guiRotation;

        info.guiScale =
            block.model.guiScale;

        renderInfo[index] = info;
    }

    return renderInfo;
}