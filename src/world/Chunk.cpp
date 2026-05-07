#include "world/Chunk.h"
#include "world/GreedyMeshBuilder.h"

Chunk::Chunk()
{
    m_blocks.fill(0);
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

    markDirty();
}

bool Chunk::isDirty() const
{
    return m_dirty;
}

void Chunk::markDirty()
{
    m_dirty = true;
}

void Chunk::clearDirty()
{
    m_dirty = false;
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

    markDirty();
}

std::vector<float> Chunk::buildMesh(
    const std::array<BlockRenderInfo, 4>& renderInfo,
    const glm::vec3& worldPosition
) const
{
    std::vector<float> vertices;
    vertices.reserve(SIZE * SIZE * 6 * 6);

    GreedyMeshBuilder::addGreedyFaces(
        *this,
        vertices,
        renderInfo,
        worldPosition
    );

    return vertices;
}

float Chunk::getTextureIndexForFace(
    uint16_t blockId,
    int face,
    const std::array<BlockRenderInfo, 4>& renderInfo
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