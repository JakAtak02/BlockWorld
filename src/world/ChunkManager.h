#pragma once

#include "renderer/Mesh.h"

#include "world/Chunk.h"
#include "world/ChunkCoord.h"
#include "world/generation/TerrainGenerator.h"
#include "world/save/WorldSaveManager.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

class World;

enum class ChunkStreamingState
{
    Loading,
    Loaded,
    MeshQueued,
    Meshing,
    MeshReadyForUpload,
    ReadyToRender,
    UnloadPending
};

struct ChunkRenderData
{
    Chunk chunk;

    std::unique_ptr<Mesh> mesh;

    std::vector<float> pendingMeshVertices;
    bool meshUploadQueued = false;

    ChunkStreamingState streamingState =
        ChunkStreamingState::Loading;

    uint64_t activeRequestId = 0;
    uint64_t activeMeshRequestId = 0;

    bool meshBuildInProgress = false;

    int chunkX = 0;
    int chunkZ = 0;

    glm::vec3 minBounds;
    glm::vec3 maxBounds;
};

class ChunkManager
{
public:
    void loadInitialChunks(
        TerrainGenerator& terrainGenerator,
        WorldSaveManager& saveManager
    );

    ChunkRenderData* findChunk(
        int chunkX,
        int chunkZ
    );

    const ChunkRenderData* findChunk(
        int chunkX,
        int chunkZ
    ) const;

    ChunkRenderData* getOrCreateChunk(
        int chunkX,
        int chunkZ,
        TerrainGenerator& terrainGenerator,
        WorldSaveManager& saveManager
    );

    void ensureChunksAroundPosition(
        const glm::vec3& worldPosition,
        const glm::vec3& forwardDirection,
        int loadRadiusChunks,
        TerrainGenerator& terrainGenerator,
        WorldSaveManager& saveManager
    );

    void processChunkLoadQueue(
        int maxChunksToLoad,
        World& world
    );

    void unloadChunksFarFromPosition(
        const glm::vec3& worldPosition,
        int unloadRadiusChunks,
        WorldSaveManager& saveManager
    );

    void setChunkState(
        int chunkX,
        int chunkZ,
        ChunkStreamingState state
    );

    size_t getPendingChunkLoadCount() const;

    std::unordered_map<
        ChunkCoord,
        ChunkRenderData,
        ChunkCoordHash
    >& getChunks();

    const std::unordered_map<
        ChunkCoord,
        ChunkRenderData,
        ChunkCoordHash
    >& getChunks() const;

private:
    void createOrLoadChunk(
        int chunkX,
        int chunkZ,
        TerrainGenerator& terrainGenerator,
        WorldSaveManager& saveManager
    );

    void enqueueChunkLoad(
        int chunkX,
        int chunkZ
    );

    bool isChunkLoadQueued(
        int chunkX,
        int chunkZ
    ) const;

    static int worldToChunkCoord(
        float worldCoordinate
    );

private:
    std::unordered_map<
        ChunkCoord,
        ChunkRenderData,
        ChunkCoordHash
    > m_chunks;

    std::deque<ChunkCoord>
        m_pendingChunkLoads;
};