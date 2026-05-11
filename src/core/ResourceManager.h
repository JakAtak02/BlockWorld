#pragma once

#include "renderer/TextureArray.h"
#include "world/BlockRegistry.h"
#include "world/Chunk.h"

#include <string>
#include <vector>

class ResourceManager
{
public:
    bool initialize();

    TextureArray& getBlockTextureArray();

    const std::vector<BlockRenderInfo>&
        getBlockRenderInfo() const;

private:
    bool loadBlocks();

    bool loadBlocksFromManifest(
        const std::string& manifestPath
    );

    bool loadBlockTextures();

private:
    BlockRegistry m_blockRegistry;

    TextureArray m_blockTextureArray;

    std::vector<BlockRenderInfo>
        m_blockRenderInfo;
};