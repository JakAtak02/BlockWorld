#include "world/save/ChunkSerializer.h"

#include <fstream>

#include <json.hpp>

bool ChunkSerializer::saveChunk(
    const Chunk& chunk,
    const std::string& path,
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    nlohmann::json data;

    data["version"] = 1;
    data["chunk_x"] = chunkX;
    data["chunk_y"] = chunkY;
    data["chunk_z"] = chunkZ;
    data["size"] = Chunk::SIZE;

    data["blocks"] = chunk.getBlocks();

    std::ofstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    file << data.dump(1);

    return true;
}

bool ChunkSerializer::loadChunk(
    Chunk& chunk,
    const std::string& path
)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    nlohmann::json data;
    file >> data;

    int version =
        data.value("version", 0);

    if (version != 1)
    {
        return false;
    }

    if (!data.contains("blocks"))
    {
        return false;
    }

    std::vector<uint16_t> blocks =
        data["blocks"]
        .get<std::vector<uint16_t>>();

    if (blocks.size() != Chunk::VOLUME)
    {
        return false;
    }

    chunk.setBlocks(blocks);

    chunk.markMeshDirty();
    chunk.clearSaveDirty();

    return true;
}