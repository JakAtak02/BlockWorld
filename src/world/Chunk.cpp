#include "world/Chunk.h"

#include "world/GreedyMeshBuilder.h"

Chunk::Chunk()
{
}

int Chunk::index(
    int x,
    int y,
    int z
) const
{
    return x + SIZE * (y + SIZE * z);
}

uint16_t Chunk::getBlock(
    int x,
    int y,
    int z
) const
{
    if (x < 0 || x >= SIZE ||
        y < 0 || y >= SIZE ||
        z < 0 || z >= SIZE)
    {
        return 0;
    }

    return m_blocks[index(x, y, z)];
}

void Chunk::setBlock(
    int x,
    int y,
    int z,
    uint16_t blockId
)
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

const std::vector<uint16_t>&
Chunk::getBlocks() const
{
    return m_blocks;
}

void Chunk::setBlocks(
    const std::vector<uint16_t>& blocks
)
{
    m_blocks = blocks;

    markMeshDirty();
    clearSaveDirty();
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

std::vector<float> Chunk::buildMesh(
    const std::vector<BlockRenderInfo>& renderInfo,
    const glm::vec3& worldPosition,
    const BlockLookupFunction& blockLookup
) const
{
    std::vector<float> vertices;

    vertices.reserve(
        SIZE * SIZE * 6 * 6
    );

    GreedyMeshBuilder::addGreedyFaces(
        *this,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup
    );

    return vertices;
}