#include "world/save/WorldSaveManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
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
        getWorldSavePath() + "/chunks"
    );

    if (!worldMetadataExists())
    {
        createWorldMetadataFile();
    }
}

bool WorldSaveManager::loadChunk(
    Chunk& chunk,
    int chunkX,
    int chunkZ
)
{
    return ChunkSerializer::loadChunk(
        chunk,
        getChunkSavePath(chunkX, chunkZ)
    );
}

bool WorldSaveManager::saveChunk(
    const Chunk& chunk,
    int chunkX,
    int chunkZ
)
{
    return ChunkSerializer::saveChunk(
        chunk,
        getChunkSavePath(chunkX, chunkZ),
        chunkX,
        chunkZ
    );
}

std::string WorldSaveManager::getWorldSavePath() const
{
    return "saves/" + m_worldName;
}

std::string WorldSaveManager::getChunkSavePath(
    int chunkX,
    int chunkZ
) const
{
    std::stringstream stream;

    stream
        << getWorldSavePath()
        << "/chunks/chunk_"
        << chunkX
        << "_"
        << chunkZ
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