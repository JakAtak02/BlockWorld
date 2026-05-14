#pragma once

#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"

#include "world/Chunk.h"
#include "world/ChunkManager.h"
#include "world/streaming/ChunkStreamingTypes.h"
#include "world/generation/TerrainGenerator.h"
#include "world/save/WorldSaveManager.h"

#include "core/JobSystem.h"
#include "core/ThreadSafeQueue.h"

#include <glm/glm.hpp>

#include <deque>
#include <cstdint>
#include <memory>
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
    static constexpr int VERTICAL_CHUNK_COUNT = 64;
    static constexpr int WORLD_MIN_Y = 0;
    static constexpr int WORLD_HEIGHT =
        Chunk::SIZE * VERTICAL_CHUNK_COUNT;
    static constexpr int WORLD_MAX_Y =
        WORLD_MIN_Y + WORLD_HEIGHT - 1;

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

    uint64_t submitAsyncChunkLoad(
        int chunkX,
        int chunkY,
        int chunkZ
    );

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
        int chunkY,
        int chunkZ
    );

    void markChunkAndNeighborsDirty(
        int chunkX,
        int chunkY,
        int chunkZ
    );

    void processMeshRebuildQueue(
        int maxMeshRebuilds
    );

    void processMeshUploadQueue(
        int maxMeshUploads
    );

    bool isMeshRebuildQueued(
        int chunkX,
        int chunkY,
        int chunkZ
    ) const;

    void processCompletedChunkLoads(
        int maxResultsToProcess
    );

    uint64_t submitAsyncMeshBuild(
        const ChunkMeshBuildSnapshot& snapshot
    );

    void processCompletedMeshBuilds(
        int maxResultsToProcess
    );

    ChunkMeshBuildSnapshot
        createMeshBuildSnapshot(
            const ChunkRenderData& data
        ) const;

private:
    ChunkManager m_chunkManager;

    JobSystem m_jobSystem;

    ThreadSafeQueue<
        AsyncChunkLoadResult
    > m_completedChunkLoads;

    ThreadSafeQueue<
        AsyncMeshBuildResult
    > m_completedMeshBuilds;

    uint64_t m_nextChunkRequestId = 1;

    std::deque<ChunkCoord>
        m_pendingMeshRebuilds;

    std::deque<ChunkCoord>
        m_pendingMeshUploads;

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