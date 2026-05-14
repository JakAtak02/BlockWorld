#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

struct BlockRenderInfo
{
    float sideTextureIndex = 0.0f;
    float topTextureIndex = 0.0f;
    float bottomTextureIndex = 0.0f;

    bool tinted = false;

    glm::vec3 tintColor =
    {
        1.0f,
        1.0f,
        1.0f
    };

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

    uint16_t getBlock(
        int x,
        int y,
        int z
    ) const;

    void setBlock(
        int x,
        int y,
        int z,
        uint16_t blockId
    );

    void setGeneratedBlock(
        int x,
        int y,
        int z,
        uint16_t blockId
    );

    bool isEmpty() const;

    bool shouldSaveIfEmpty() const;

    const std::vector<uint16_t>&
        getBlocks() const;

    void setBlocks(
        const std::vector<uint16_t>& blocks
    );

    bool isMeshDirty() const;

    void markMeshDirty();

    void clearMeshDirty();

    bool isSaveDirty() const;

    void markSaveDirty();

    void clearSaveDirty();

    std::vector<float> buildMesh(
        const std::vector<BlockRenderInfo>& renderInfo,
        const glm::vec3& worldPosition,
        const BlockLookupFunction& blockLookup
    ) const;

private:
    int index(
        int x,
        int y,
        int z
    ) const;

private:
    std::vector<uint16_t> m_blocks =
        std::vector<uint16_t>(VOLUME, 0);

    bool m_meshDirty = true;

    bool m_saveDirty = false;

    bool m_shouldSaveIfEmpty = false;
};