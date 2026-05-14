#pragma once

#include "world/Chunk.h"

#include <string>

class ChunkSerializer
{
public:
    static bool saveChunk(
        const Chunk& chunk,
        const std::string& path,
        int chunkX,
        int chunkY,
        int chunkZ
    );

    static bool loadChunk(
        Chunk& chunk,
        const std::string& path
    );
};