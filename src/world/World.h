#pragma once

#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"

#include "world/Chunk.h"
#include "world/ChunkManager.h"
#include "world/generation/TerrainGenerator.h"
#include "world/save/WorldSaveManager.h"

#include <glm/glm.hpp>

#include <deque>
#include <string>
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
public:
    World(
        const std::vector<BlockRenderInfo>& renderInfo
    );

    void updateAroundPlayer(
        const glm::vec3& playerPosition,
        const glm::vec3& playerForward,
        float deltaTime
    );

    void update(
        float deltaTime
    );

    void saveWorld();

    void printStreamingDebugStats() const;

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

    static int calculateChunkLoadBudget(
        float deltaTime
    );

    static int calculateMeshRebuildBudget(
        float deltaTime
    );

    void rebuildChunkMesh(
        ChunkRenderData& chunkData
    );

    void enqueueMeshRebuild(
        int chunkX,
        int chunkZ
    );

    void processMeshRebuildQueue(
        int maxMeshRebuilds
    );

    bool isMeshRebuildQueued(
        int chunkX,
        int chunkZ
    ) const;

private:
    ChunkManager m_chunkManager;

    std::deque<ChunkCoord>
        m_pendingMeshRebuilds;

    glm::vec3
        m_lastPlayerPosition{ 0.0f };

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