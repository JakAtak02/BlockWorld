#pragma once

#include "world/Chunk.h"
#include "world/save/ChunkSerializer.h"

#include <string>

class WorldSaveManager
{
public:
    WorldSaveManager(
        const std::string& worldName,
        uint32_t worldVersion,
        uint32_t worldSeed
    );

    void initialize();

    bool loadChunk(
        Chunk& chunk,
        int chunkX,
        int chunkZ
    );

    bool saveChunk(
        const Chunk& chunk,
        int chunkX,
        int chunkZ
    );

private:
    std::string getWorldSavePath() const;

    std::string getChunkSavePath(
        int chunkX,
        int chunkZ
    ) const;

    bool worldMetadataExists() const;

    void createWorldMetadataFile() const;

private:
    std::string m_worldName;

    uint32_t m_worldVersion = 1;

    uint32_t m_worldSeed = 0;
};