#pragma once

#include "world/Chunk.h"

class TerrainGenerator
{
public:
    void generateChunk(
        Chunk& chunk,
        int chunkX,
        int chunkZ
    ) const;
};