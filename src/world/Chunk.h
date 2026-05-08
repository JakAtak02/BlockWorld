#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <glm/glm.hpp>

struct BlockRenderInfo
{
    float sideTextureIndex = 0.0f;
    float topTextureIndex = 0.0f;
    float bottomTextureIndex = 0.0f;

    glm::vec3 guiRotation =
    {
        -28.0f,
        -38.0f,
        0.0f
    };

    float guiScale = 34.0f;
};

class Chunk
{
public:
    static constexpr int SIZE = 32;
    static constexpr int VOLUME = SIZE * SIZE * SIZE;

    using BlockLookupFunction =
        std::function<uint16_t(int, int, int)>;

    Chunk();

    void generateTerrain(int chunkX, int chunkZ);

    uint16_t getBlock(int x, int y, int z) const;

    void setBlock(int x, int y, int z, uint16_t blockId);

    bool isDirty() const;
    void markDirty();
    void clearDirty();

    std::vector<float> buildMesh(
        const std::vector<BlockRenderInfo>& renderInfo,
        const glm::vec3& worldPosition,
        const BlockLookupFunction& blockLookup
    ) const;

private:
    int index(int x, int y, int z) const;

    bool isAir(int x, int y, int z) const;

    float getTextureIndexForFace(
        uint16_t blockId,
        int face,
        const std::vector<BlockRenderInfo>& renderInfo
    ) const;

    void addFace(
        std::vector<float>& vertices,
        int x,
        int y,
        int z,
        int face,
        float textureIndex
    ) const;

private:
    std::vector<uint16_t> m_blocks =
        std::vector<uint16_t>(VOLUME, 0);

    bool m_dirty = true;
};