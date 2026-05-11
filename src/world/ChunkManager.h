#pragma once

#include "renderer/Mesh.h"

#include "world/Chunk.h"
#include "world/ChunkCoord.h"
#include "world/generation/TerrainGenerator.h"
#include "world/save/WorldSaveManager.h"

#include <glm/glm.hpp>

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

enum class ChunkStreamingState
{
    Unloaded,
    Queued,
    Loading,
    Loaded,
    MeshQueued,
    Meshing,
    ReadyToRender,
    UnloadPending
};

struct ChunkRenderData
{
    Chunk chunk;

    std::unique_ptr<Mesh> mesh;

    int chunkX = 0;
    int chunkZ = 0;

    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    ChunkStreamingState streamingState =
        ChunkStreamingState::Unloaded;
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
        TerrainGenerator& terrainGenerator,
        WorldSaveManager& saveManager
    );

    void unloadChunksFarFromPosition(
        const glm::vec3& worldPosition,
        int unloadRadiusChunks,
        WorldSaveManager& saveManager
    );

    int getPendingChunkLoadCount() const;

    void setChunkState(
        int chunkX,
        int chunkZ,
        ChunkStreamingState state
    );

    ChunkStreamingState getChunkState(
        int chunkX,
        int chunkZ
    ) const;

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

    std::deque<ChunkCoord> m_pendingChunkLoads;
};