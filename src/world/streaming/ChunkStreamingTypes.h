#pragma once

#include "world/Chunk.h"

#include <cstdint>
#include <vector>

enum class AsyncChunkLoadResultType
{
    LoadedFromDisk,
    Generated,
    Failed
};

struct AsyncChunkLoadResult
{
    int chunkX = 0;
    int chunkZ = 0;

    uint64_t requestId = 0;

    AsyncChunkLoadResultType resultType =
        AsyncChunkLoadResultType::Failed;

    Chunk chunk;
};

struct ChunkMeshBuildSnapshot
{
    int chunkX = 0;
    int chunkZ = 0;

    uint64_t requestId = 0;

    Chunk centerChunk;

    bool hasNegativeXNeighbor = false;
    bool hasPositiveXNeighbor = false;
    bool hasNegativeZNeighbor = false;
    bool hasPositiveZNeighbor = false;

    Chunk negativeXNeighbor;
    Chunk positiveXNeighbor;
    Chunk negativeZNeighbor;
    Chunk positiveZNeighbor;
};

struct AsyncMeshBuildResult
{
    int chunkX = 0;
    int chunkZ = 0;

    uint64_t requestId = 0;

    std::vector<float> vertices;
};