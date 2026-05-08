#pragma once

#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"
#include "renderer/Mesh.h"
#include "world/Chunk.h"

#include <glm/glm.hpp>

#include <memory>
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
    struct ChunkCoord
    {
        int x = 0;
        int z = 0;

        bool operator==(const ChunkCoord& other) const
        {
            return x == other.x && z == other.z;
        }
    };

    struct ChunkCoordHash
    {
        std::size_t operator()(const ChunkCoord& coord) const
        {
            std::size_t h1 = std::hash<int>{}(coord.x);
            std::size_t h2 = std::hash<int>{}(coord.z);
            return h1 ^ (h2 << 1);
        }
    };

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
    World(const std::vector<BlockRenderInfo>& renderInfo);

    void update();

    uint16_t getBlock(int worldX, int worldY, int worldZ) const;
    void setBlock(int worldX, int worldY, int worldZ, uint16_t blockId);

    bool raycastBlock(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        BlockRaycastHit& hit
    ) const;

    void draw(const Frustum& frustum) const;

    void drawChunkBorders(
        DebugRenderer& debugRenderer,
        const glm::mat4& projection,
        const glm::mat4& view
    ) const;

private:
    static int floorDiv(int value, int divisor);
    static int positiveMod(int value, int divisor);

    ChunkRenderData* findChunk(int chunkX, int chunkZ);
    const ChunkRenderData* findChunk(int chunkX, int chunkZ) const;

    void rebuildChunkMesh(ChunkRenderData& chunkData);

private:
    std::unordered_map<ChunkCoord, ChunkRenderData, ChunkCoordHash> m_chunks;

    std::vector<BlockRenderInfo> m_renderInfo;
};