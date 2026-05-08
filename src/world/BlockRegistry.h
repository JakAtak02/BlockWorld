#pragma once

#include "world/BlockType.h"
#include "world/Chunk.h"

#include <string>
#include <unordered_map>
#include <vector>

class BlockRegistry
{
public:
    bool loadBlockFromJson(const std::string& path);

    const BlockType* getBlock(const std::string& id) const;

    const std::vector<std::string>& getTexturePaths() const;

    std::vector<BlockRenderInfo> buildRenderInfo() const;

private:
    std::unordered_map<std::string, BlockType> m_blocks;

    std::vector<std::string> m_texturePaths;

    uint16_t m_nextBlockId = 1;
};