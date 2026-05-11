#pragma once

#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"
#include "renderer/Mesh.h"

#include "world/Chunk.h"
#include "world/ChunkCoord.h"

#include "world/generation/TerrainGenerator.h"

#include "world/save/WorldSaveManager.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct BlockRaycastHit
{
    bool hit = false;

    glm::ivec3 blockPosition{ 0 };

    glm::ivec3 previousBlockPosition{ 0 };

    uint16_t blockId = 0;

    float distance = 0.0f;
};

class World
{
private:
    struct ChunkRenderData
    {
        Chunk chunk;

        std::unique_ptr<Mesh> mesh;

        int chunkX = 0;
        int chunkZ = 0;

        glm::vec3 minBounds;

        glm::vec3 maxBounds;
    };

public:
    World(
        const std::vector<BlockRenderInfo>& renderInfo
    );

    void update();

    void saveWorld();

    uint16_t getBlock(
        int worldX,
        int worldY,
        int worldZ
    ) const;

    void setBlock(
        int worldX,
        int worldY,
        int worldZ,
        uint16_t blockId
    );

    bool raycastBlock(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        BlockRaycastHit& hit
    ) const;

    void draw(
        const Frustum& frustum
    ) const;

    void drawChunkBorders(
        DebugRenderer& debugRenderer,
        const glm::mat4& projection,
        const glm::mat4& view
    ) const;

private:
    static int floorDiv(
        int value,
        int divisor
    );

    static int positiveMod(
        int value,
        int divisor
    );

    ChunkRenderData* findChunk(
        int chunkX,
        int chunkZ
    );

    const ChunkRenderData* findChunk(
        int chunkX,
        int chunkZ
    ) const;

    void rebuildChunkMesh(
        ChunkRenderData& chunkData
    );

private:
    std::unordered_map<
        ChunkCoord,
        ChunkRenderData,
        ChunkCoordHash
    > m_chunks;

    std::vector<BlockRenderInfo>
        m_renderInfo;

    TerrainGenerator
        m_terrainGenerator;

    std::string m_worldName =
        "World_0";

    uint32_t m_worldVersion = 1;

    uint32_t m_worldSeed = 12345;

    WorldSaveManager m_saveManager
    {
        m_worldName,
        m_worldVersion,
        m_worldSeed
    };
};