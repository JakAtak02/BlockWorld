#pragma once

#include "world/BlockType.h"

#include <string>
#include <unordered_map>

class BlockRegistry
{
public:
    bool loadBlockFromJson(const std::string& path);

    const BlockType* getBlock(const std::string& id) const;

private:
    std::unordered_map<std::string, BlockType> m_blocks;
};