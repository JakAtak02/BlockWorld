#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

struct BlockRenderInfo
{
    float sideTextureIndex = 0.0f;
    float topTextureIndex = 0.0f;
    float bottomTextureIndex = 0.0f;
};

class Chunk
{
public:
    static constexpr int SIZE = 32;
    static constexpr int VOLUME = SIZE * SIZE * SIZE;

    Chunk();

    void generateTerrain(int chunkX, int chunkZ);

    uint16_t getBlock(int x, int y, int z) const;

    void setBlock(int x, int y, int z, uint16_t blockId);

    bool isDirty() const;
    void markDirty();
    void clearDirty();

    std::vector<float> buildMesh(
        const std::array<BlockRenderInfo, 4>& renderInfo,
        const glm::vec3& worldPosition
    ) const;

private:
    int index(int x, int y, int z) const;

    bool isAir(int x, int y, int z) const;

    float getTextureIndexForFace(
        uint16_t blockId,
        int face,
        const std::array<BlockRenderInfo, 4>& renderInfo
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
    std::array<uint16_t, VOLUME> m_blocks;

    bool m_dirty = true;
};