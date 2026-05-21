#include "world/save/WorldSaveManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include <json.hpp>

WorldSaveManager::WorldSaveManager(
    const std::string& worldName,
    uint32_t worldVersion,
    uint32_t worldSeed
)
    :
    m_worldName(worldName),
    m_worldVersion(worldVersion),
    m_worldSeed(worldSeed)
{
}

void WorldSaveManager::initialize()
{
    namespace fs = std::filesystem;

    fs::create_directories(
        getWorldSavePath() + "/regions"
    );

    if (!worldMetadataExists())
    {
        createWorldMetadataFile();
    }
}

bool WorldSaveManager::loadChunk(
    Chunk& chunk,
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    return ChunkSerializer::loadChunk(
        chunk,
        getChunkSavePath(
            chunkX,
            chunkY,
            chunkZ
        )
    );
}

bool WorldSaveManager::saveChunk(
    const Chunk& chunk,
    int chunkX,
    int chunkY,
    int chunkZ
)
{

    if (chunk.isEmpty() &&
        !chunk.shouldSaveIfEmpty())
    {
        return true;
    }

    namespace fs = std::filesystem;

    int regionX =
        chunkToRegionCoord(chunkX);

    int regionZ =
        chunkToRegionCoord(chunkZ);

    fs::create_directories(
        getRegionDirectoryPath(
            regionX,
            regionZ
        )
    );

    bool success =
        ChunkSerializer::saveChunk(
            chunk,
            getChunkSavePath(
                chunkX,
                chunkY,
                chunkZ
            ),
            chunkX,
            chunkY,
            chunkZ
        );

    if (!success)
    {
        return false;
    }

    ChunkColumnKey key
    {
        chunkX,
        chunkZ
    };

    std::vector<int>& cachedYs =
        m_savedChunkColumnCache[key];

    if (std::find(
        cachedYs.begin(),
        cachedYs.end(),
        chunkY
    ) ==
        cachedYs.end())
    {
        cachedYs.push_back(chunkY);
    }

    return true;
}

std::vector<int> WorldSaveManager::getSavedChunkYs(
    int chunkX,
    int chunkZ
) const
{
    ChunkColumnKey key
    {
        chunkX,
        chunkZ
    };

    auto cachedIt =
        m_savedChunkColumnCache.find(key);

    if (cachedIt !=
        m_savedChunkColumnCache.end())
    {
        return cachedIt->second;
    }

    namespace fs = std::filesystem;

    std::vector<int> chunkYs;

    int regionX =
        chunkToRegionCoord(chunkX);

    int regionZ =
        chunkToRegionCoord(chunkZ);

    int localChunkX =
        chunkX -
        (regionX * REGION_SIZE);

    int localChunkZ =
        chunkZ -
        (regionZ * REGION_SIZE);

    fs::path regionPath =
        getRegionDirectoryPath(
            regionX,
            regionZ
        );

    if (!fs::exists(regionPath))
    {
        return chunkYs;
    }

    std::string filenamePrefix =
        "chunk_" +
        std::to_string(localChunkX) +
        "_";

    std::string filenameSuffix =
        "_" +
        std::to_string(localChunkZ) +
        ".json";

    for (const auto& entry :
        fs::directory_iterator(regionPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::string filename =
            entry.path().filename().string();

        if (!filename.starts_with(
            filenamePrefix))
        {
            continue;
        }

        if (!filename.ends_with(
            filenameSuffix))
        {
            continue;
        }

        size_t start =
            filenamePrefix.length();

        size_t end =
            filename.length() -
            filenameSuffix.length();

        std::string yString =
            filename.substr(
                start,
                end - start
            );

        try
        {
            int chunkY =
                std::stoi(yString);

            chunkYs.push_back(chunkY);
        }
        catch (...)
        {
        }
    }

    m_savedChunkColumnCache[key] =
        chunkYs;

    return chunkYs;
}

int WorldSaveManager::chunkToRegionCoord(
    int chunkCoordinate
) const
{
    if (chunkCoordinate >= 0)
    {
        return chunkCoordinate / REGION_SIZE;
    }

    return ((chunkCoordinate + 1) / REGION_SIZE) - 1;
}

std::string WorldSaveManager::getRegionDirectoryPath(
    int regionX,
    int regionZ
) const
{
    std::stringstream stream;

    stream
        << getWorldSavePath()
        << "/regions/region_"
        << regionX
        << "_"
        << regionZ;

    return stream.str();
}

std::string WorldSaveManager::getWorldSavePath() const
{
    return "saves/" + m_worldName;
}

std::string WorldSaveManager::getChunkSavePath(
    int chunkX,
    int chunkY,
    int chunkZ
) const
{
    int regionX =
        chunkToRegionCoord(chunkX);

    int regionZ =
        chunkToRegionCoord(chunkZ);

    int localChunkX =
        chunkX - regionX * REGION_SIZE;

    int localChunkZ =
        chunkZ - regionZ * REGION_SIZE;

    std::stringstream stream;

    stream
        << getRegionDirectoryPath(
            regionX,
            regionZ
        )
        << "/chunk_"
        << localChunkX
        << "_"
        << chunkY
        << "_"
        << localChunkZ
        << ".json";

    return stream.str();
}

bool WorldSaveManager::worldMetadataExists() const
{
    namespace fs = std::filesystem;

    return fs::exists(
        getWorldSavePath() + "/world.json"
    );
}

void WorldSaveManager::createWorldMetadataFile() const
{
    nlohmann::json data;

    data["name"] = m_worldName;
    data["version"] = m_worldVersion;
    data["seed"] = m_worldSeed;
    data["chunk_size"] = Chunk::SIZE;
    data["world_min_y"] = 0;
    data["vertical_chunk_count"] = 64;
    data["world_height"] = Chunk::SIZE * 64;

    std::ofstream file(
        getWorldSavePath() + "/world.json"
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to create world metadata file."
            << std::endl;

        return;
    }

    file << data.dump(1);

    std::cout
        << "Created world metadata file."
        << std::endl;
}