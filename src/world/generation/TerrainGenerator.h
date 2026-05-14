#pragma once

#include "world/Chunk.h"

class TerrainGenerator
{
public:
    void generateChunk(
        Chunk& chunk,
        int chunkX,
        int chunkY,
        int chunkZ
    ) const;

    int getTerrainHeight(
        int worldX,
        int worldZ
    ) const;

    int getHighestTerrainChunkY(
        int chunkX,
        int chunkZ
    ) const;
};