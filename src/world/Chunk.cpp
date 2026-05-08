#include "world/Chunk.h"
#include "world/GreedyMeshBuilder.h"

#include <fstream>
#include <json.hpp>

Chunk::Chunk()
{

}

int Chunk::index(int x, int y, int z) const
{
    return x + SIZE * (y + SIZE * z);
}

uint16_t Chunk::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= SIZE ||
        y < 0 || y >= SIZE ||
        z < 0 || z >= SIZE)
    {
        return 0;
    }

    return m_blocks[index(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, uint16_t blockId)
{
    if (x < 0 || x >= SIZE ||
        y < 0 || y >= SIZE ||
        z < 0 || z >= SIZE)
    {
        return;
    }

    m_blocks[index(x, y, z)] = blockId;

    markMeshDirty();
    markSaveDirty();
}

bool Chunk::isMeshDirty() const
{
    return m_meshDirty;
}

void Chunk::markMeshDirty()
{
    m_meshDirty = true;
}

void Chunk::clearMeshDirty()
{
    m_meshDirty = false;
}

bool Chunk::isSaveDirty() const
{
    return m_saveDirty;
}

void Chunk::markSaveDirty()
{
    m_saveDirty = true;
}

void Chunk::clearSaveDirty()
{
    m_saveDirty = false;
}

bool Chunk::isAir(int x, int y, int z) const
{
    return getBlock(x, y, z) == 0;
}

void Chunk::generateTerrain(int chunkX, int chunkZ)
{
    for (int z = 0; z < SIZE; z++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            int worldX = chunkX * SIZE + x;
            int worldZ = chunkZ * SIZE + z;

            float heightNoise =
                sin(worldX * 0.05f) * 4.0f +
                cos(worldZ * 0.05f) * 4.0f;

            int terrainHeight = static_cast<int>(heightNoise + 12.0f);

            for (int y = 0; y < SIZE; y++)
            {
                uint16_t block = 0;

                if (y < terrainHeight - 4)
                {
                    block = 1;
                }
                else if (y < terrainHeight - 1)
                {
                    block = 2;
                }
                else if (y == terrainHeight - 1)
                {
                    block = 3;
                }

                m_blocks[index(x, y, z)] = block;
            }
        }
    }

    markMeshDirty();
    markSaveDirty();
}

std::vector<float> Chunk::buildMesh(
    const std::vector<BlockRenderInfo>& renderInfo,
    const glm::vec3& worldPosition,
    const BlockLookupFunction& blockLookup
) const
{
    std::vector<float> vertices;
    vertices.reserve(SIZE * SIZE * 6 * 6);

    GreedyMeshBuilder::addGreedyFaces(
        *this,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup
    );

    return vertices;
}

float Chunk::getTextureIndexForFace(
    uint16_t blockId,
    int face,
    const std::vector<BlockRenderInfo>& renderInfo
) const
{
    if (blockId == 0 || blockId > renderInfo.size())
        return 0.0f;

    const BlockRenderInfo& info = renderInfo[blockId - 1];

    if (face == 4)
        return info.bottomTextureIndex;

    if (face == 5)
        return info.topTextureIndex;

    return info.sideTextureIndex;
}

void Chunk::addFace(
    std::vector<float>& vertices,
    int x,
    int y,
    int z,
    int face,
    float textureIndex
) const
{
}

bool Chunk::saveToFile(
    const std::string& path,
    int chunkX,
    int chunkZ
) const
{
    nlohmann::json data;

    data["version"] = 1;
    data["chunk_x"] = chunkX;
    data["chunk_z"] = chunkZ;
    data["size"] = SIZE;

    data["blocks"] = m_blocks;

    std::ofstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    file << data.dump(1);

    return true;
}

bool Chunk::loadFromFile(
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

    int version = data.value("version", 0);

    if (version != 1)
    {
        return false;
    }

    if (!data.contains("blocks"))
    {
        return false;
    }

    m_blocks =
        data["blocks"].get<std::vector<uint16_t>>();

    if (m_blocks.size() != VOLUME)
    {
        return false;
    }

    markMeshDirty();
    clearSaveDirty();

    return true;
}