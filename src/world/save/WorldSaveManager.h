#pragma once

#include "world/Chunk.h"
#include "world/save/ChunkSerializer.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class WorldSaveManager
{
public:
    static constexpr int REGION_SIZE = 32;

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
        int chunkY,
        int chunkZ
    );

    bool saveChunk(
        const Chunk& chunk,
        int chunkX,
        int chunkY,
        int chunkZ
    );

    std::vector<int> getSavedChunkYs(
        int chunkX,
        int chunkZ
    ) const;

private:
    struct ChunkColumnKey
    {
        int chunkX = 0;
        int chunkZ = 0;

        bool operator==(
            const ChunkColumnKey& other
            ) const
        {
            return chunkX == other.chunkX &&
                chunkZ == other.chunkZ;
        }
    };

    struct ChunkColumnKeyHash
    {
        size_t operator()(
            const ChunkColumnKey& key
            ) const
        {
            size_t hashX =
                std::hash<int>{}(key.chunkX);

            size_t hashZ =
                std::hash<int>{}(key.chunkZ);

            return hashX ^
                (hashZ << 1);
        }
    };

private:
    std::string getWorldSavePath() const;

    std::string getRegionDirectoryPath(
        int regionX,
        int regionZ
    ) const;

    std::string getChunkSavePath(
        int chunkX,
        int chunkY,
        int chunkZ
    ) const;

    int chunkToRegionCoord(
        int chunkCoordinate
    ) const;

    bool worldMetadataExists() const;

    void createWorldMetadataFile() const;

private:
    std::string m_worldName;

    uint32_t m_worldVersion = 1;

    uint32_t m_worldSeed = 0;

    mutable std::unordered_map<
        ChunkColumnKey,
        std::vector<int>,
        ChunkColumnKeyHash
    > m_savedChunkColumnCache;
};