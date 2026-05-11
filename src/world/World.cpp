#include "world/World.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

World::World(
    const std::vector<BlockRenderInfo>& renderInfo
)
    :
    m_renderInfo(renderInfo)
{
    m_saveManager.initialize();

    m_chunkManager.loadInitialChunks(
        m_terrainGenerator,
        m_saveManager
    );

    for (auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        rebuildChunkMesh(data);

        data.chunk.clearMeshDirty();

        m_chunkManager.setChunkState(
            coord.x,
            coord.z,
            ChunkStreamingState::ReadyToRender
        );
    }
}

void World::update(
    float deltaTime
)
{
    int meshRebuildBudget =
        calculateMeshRebuildBudget(deltaTime);

    for (auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        if (!data.chunk.isMeshDirty())
        {
            continue;
        }

        enqueueMeshRebuild(
            coord.x,
            coord.z
        );
    }

    processMeshRebuildQueue(
        meshRebuildBudget
    );
}

void World::updateAroundPlayer(
    const glm::vec3& playerPosition,
    const glm::vec3& playerForward,
    float deltaTime
)
{
    constexpr int LOAD_RADIUS_CHUNKS = 4;
    constexpr int UNLOAD_RADIUS_CHUNKS = 5;

    int chunkLoadBudget =
        calculateChunkLoadBudget(deltaTime);

    m_lastPlayerPosition =
        playerPosition;

    m_chunkManager.ensureChunksAroundPosition(
        playerPosition,
        playerForward,
        LOAD_RADIUS_CHUNKS,
        m_terrainGenerator,
        m_saveManager
    );

    m_chunkManager.unloadChunksFarFromPosition(
        playerPosition,
        UNLOAD_RADIUS_CHUNKS,
        m_saveManager
    );

    m_chunkManager.processChunkLoadQueue(
        chunkLoadBudget,
        m_terrainGenerator,
        m_saveManager
    );
}

uint16_t World::getBlock(
    int worldX,
    int worldY,
    int worldZ
) const
{
    if (worldY < 0 ||
        worldY >= Chunk::SIZE)
    {
        return 0;
    }

    int chunkX =
        floorDiv(worldX, Chunk::SIZE);

    int chunkZ =
        floorDiv(worldZ, Chunk::SIZE);

    int localX =
        positiveMod(worldX, Chunk::SIZE);

    int localZ =
        positiveMod(worldZ, Chunk::SIZE);

    const ChunkRenderData* data =
        m_chunkManager.findChunk(
            chunkX,
            chunkZ
        );

    if (!data)
    {
        return 0;
    }

    return data->chunk.getBlock(
        localX,
        worldY,
        localZ
    );
}

void World::setBlock(
    int worldX,
    int worldY,
    int worldZ,
    uint16_t blockId
)
{
    if (worldY < 0 ||
        worldY >= Chunk::SIZE)
    {
        return;
    }

    int chunkX =
        floorDiv(worldX, Chunk::SIZE);

    int chunkZ =
        floorDiv(worldZ, Chunk::SIZE);

    int localX =
        positiveMod(worldX, Chunk::SIZE);

    int localZ =
        positiveMod(worldZ, Chunk::SIZE);

    ChunkRenderData* data =
        m_chunkManager.getOrCreateChunk(
            chunkX,
            chunkZ,
            m_terrainGenerator,
            m_saveManager
        );

    if (!data)
    {
        return;
    }

    data->chunk.setBlock(
        localX,
        worldY,
        localZ,
        blockId
    );

    m_chunkManager.setChunkState(
        chunkX,
        chunkZ,
        ChunkStreamingState::MeshQueued
    );

    if (localX == 0)
    {
        if (ChunkRenderData* neighbor =
            m_chunkManager.findChunk(
                chunkX - 1,
                chunkZ
            ))
        {
            neighbor->chunk.markMeshDirty();

            m_chunkManager.setChunkState(
                chunkX - 1,
                chunkZ,
                ChunkStreamingState::MeshQueued
            );
        }
    }

    if (localX == Chunk::SIZE - 1)
    {
        if (ChunkRenderData* neighbor =
            m_chunkManager.findChunk(
                chunkX + 1,
                chunkZ
            ))
        {
            neighbor->chunk.markMeshDirty();

            m_chunkManager.setChunkState(
                chunkX + 1,
                chunkZ,
                ChunkStreamingState::MeshQueued
            );
        }
    }

    if (localZ == 0)
    {
        if (ChunkRenderData* neighbor =
            m_chunkManager.findChunk(
                chunkX,
                chunkZ - 1
            ))
        {
            neighbor->chunk.markMeshDirty();

            m_chunkManager.setChunkState(
                chunkX,
                chunkZ - 1,
                ChunkStreamingState::MeshQueued
            );
        }
    }

    if (localZ == Chunk::SIZE - 1)
    {
        if (ChunkRenderData* neighbor =
            m_chunkManager.findChunk(
                chunkX,
                chunkZ + 1
            ))
        {
            neighbor->chunk.markMeshDirty();

            m_chunkManager.setChunkState(
                chunkX,
                chunkZ + 1,
                ChunkStreamingState::MeshQueued
            );
        }
    }
}

bool World::raycastBlock(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float maxDistance,
    BlockRaycastHit& hit
) const
{
    glm::vec3 rayDirection =
        glm::normalize(direction);

    glm::ivec3 blockPosition(
        static_cast<int>(
            std::floor(origin.x)
            ),

        static_cast<int>(
            std::floor(origin.y)
            ),

        static_cast<int>(
            std::floor(origin.z)
            )
    );

    glm::ivec3 step(
        rayDirection.x > 0.0f ? 1 : -1,
        rayDirection.y > 0.0f ? 1 : -1,
        rayDirection.z > 0.0f ? 1 : -1
    );

    glm::vec3 nextBoundary(
        step.x > 0 ?
        static_cast<float>(
            blockPosition.x + 1
            ) :
        static_cast<float>(
            blockPosition.x
            ),

        step.y > 0 ?
        static_cast<float>(
            blockPosition.y + 1
            ) :
        static_cast<float>(
            blockPosition.y
            ),

        step.z > 0 ?
        static_cast<float>(
            blockPosition.z + 1
            ) :
        static_cast<float>(
            blockPosition.z
            )
    );

    glm::vec3 tMax;

    glm::vec3 tDelta;

    constexpr float infinity =
        std::numeric_limits<float>::infinity();

    tMax.x =
        rayDirection.x != 0.0f ?
        (nextBoundary.x - origin.x) /
        rayDirection.x :
        infinity;

    tMax.y =
        rayDirection.y != 0.0f ?
        (nextBoundary.y - origin.y) /
        rayDirection.y :
        infinity;

    tMax.z =
        rayDirection.z != 0.0f ?
        (nextBoundary.z - origin.z) /
        rayDirection.z :
        infinity;

    tDelta.x =
        rayDirection.x != 0.0f ?
        std::abs(1.0f / rayDirection.x) :
        infinity;

    tDelta.y =
        rayDirection.y != 0.0f ?
        std::abs(1.0f / rayDirection.y) :
        infinity;

    tDelta.z =
        rayDirection.z != 0.0f ?
        std::abs(1.0f / rayDirection.z) :
        infinity;

    glm::ivec3 previousBlockPosition =
        blockPosition;

    float distance = 0.0f;

    while (distance <= maxDistance)
    {
        uint16_t blockId =
            getBlock(
                blockPosition.x,
                blockPosition.y,
                blockPosition.z
            );

        if (blockId != 0)
        {
            hit.hit = true;

            hit.blockPosition =
                blockPosition;

            hit.previousBlockPosition =
                previousBlockPosition;

            hit.blockId = blockId;

            hit.distance = distance;

            return true;
        }

        previousBlockPosition =
            blockPosition;

        if (tMax.x < tMax.y)
        {
            if (tMax.x < tMax.z)
            {
                blockPosition.x += step.x;

                distance = tMax.x;

                tMax.x += tDelta.x;
            }
            else
            {
                blockPosition.z += step.z;

                distance = tMax.z;

                tMax.z += tDelta.z;
            }
        }
        else
        {
            if (tMax.y < tMax.z)
            {
                blockPosition.y += step.y;

                distance = tMax.y;

                tMax.y += tDelta.y;
            }
            else
            {
                blockPosition.z += step.z;

                distance = tMax.z;

                tMax.z += tDelta.z;
            }
        }
    }

    hit = {};

    return false;
}

void World::draw(
    const Frustum& frustum
) const
{
    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        if (data.streamingState !=
            ChunkStreamingState::ReadyToRender)
        {
            continue;
        }

        if (!frustum.isBoxVisible(
            data.minBounds,
            data.maxBounds
        ))
        {
            continue;
        }

        if (!data.mesh)
        {
            continue;
        }

        data.mesh->draw();
    }
}

void World::drawChunkBorders(
    DebugRenderer& debugRenderer,
    const glm::mat4& projection,
    const glm::mat4& view
) const
{
    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        debugRenderer.drawChunkBorder(
            data.minBounds,
            data.maxBounds,
            projection,
            view
        );
    }
}

int World::floorDiv(
    int value,
    int divisor
)
{
    int result = value / divisor;

    int remainder = value % divisor;

    if (remainder != 0 &&
        ((remainder < 0) !=
            (divisor < 0)))
    {
        result--;
    }

    return result;
}

int World::positiveMod(
    int value,
    int divisor
)
{
    int result = value % divisor;

    if (result < 0)
    {
        result += divisor;
    }

    return result;
}

int World::calculateChunkLoadBudget(
    float deltaTime
)
{
    if (deltaTime <= 0.012f)
    {
        return 4;
    }

    if (deltaTime <= 0.018f)
    {
        return 2;
    }

    return 1;
}

int World::calculateMeshRebuildBudget(
    float deltaTime
)
{
    if (deltaTime <= 0.012f)
    {
        return 4;
    }

    if (deltaTime <= 0.018f)
    {
        return 2;
    }

    return 1;
}

void World::enqueueMeshRebuild(
    int chunkX,
    int chunkZ
)
{
    if (isMeshRebuildQueued(chunkX, chunkZ))
    {
        return;
    }

    m_pendingMeshRebuilds.push_back(
        ChunkCoord
        {
            chunkX,
            chunkZ
        }
    );
}

void World::processMeshRebuildQueue(
    int maxMeshRebuilds
)
{
    int playerChunkX =
        floorDiv(
            static_cast<int>(
                std::floor(
                    m_lastPlayerPosition.x
                )
                ),
            Chunk::SIZE
        );

    int playerChunkZ =
        floorDiv(
            static_cast<int>(
                std::floor(
                    m_lastPlayerPosition.z
                )
                ),
            Chunk::SIZE
        );

    std::sort(
        m_pendingMeshRebuilds.begin(),
        m_pendingMeshRebuilds.end(),
        [playerChunkX, playerChunkZ](
            const ChunkCoord& left,
            const ChunkCoord& right
            )
        {
            int leftDistanceX =
                left.x - playerChunkX;

            int leftDistanceZ =
                left.z - playerChunkZ;

            int rightDistanceX =
                right.x - playerChunkX;

            int rightDistanceZ =
                right.z - playerChunkZ;

            int leftDistanceSquared =
                leftDistanceX * leftDistanceX +
                leftDistanceZ * leftDistanceZ;

            int rightDistanceSquared =
                rightDistanceX * rightDistanceX +
                rightDistanceZ * rightDistanceZ;

            return leftDistanceSquared <
                rightDistanceSquared;
        }
    );

    int meshesRebuilt = 0;

    while (!m_pendingMeshRebuilds.empty() &&
        meshesRebuilt < maxMeshRebuilds)
    {
        ChunkCoord coord =
            m_pendingMeshRebuilds.front();

        m_pendingMeshRebuilds.pop_front();

        ChunkRenderData* data =
            m_chunkManager.findChunk(
                coord.x,
                coord.z
            );

        if (!data)
        {
            continue;
        }

        if (!data->chunk.isMeshDirty())
        {
            continue;
        }

        m_chunkManager.setChunkState(
            coord.x,
            coord.z,
            ChunkStreamingState::Meshing
        );

        rebuildChunkMesh(*data);

        data->chunk.clearMeshDirty();

        m_chunkManager.setChunkState(
            coord.x,
            coord.z,
            ChunkStreamingState::ReadyToRender
        );

        meshesRebuilt++;
    }
}

bool World::isMeshRebuildQueued(
    int chunkX,
    int chunkZ
) const
{
    for (const ChunkCoord& coord :
        m_pendingMeshRebuilds)
    {
        if (coord.x == chunkX &&
            coord.z == chunkZ)
        {
            return true;
        }
    }

    return false;
}

void World::rebuildChunkMesh(
    ChunkRenderData& data
)
{
    glm::vec3 worldPosition(
        static_cast<float>(
            data.chunkX * Chunk::SIZE
            ),

        0.0f,

        static_cast<float>(
            data.chunkZ * Chunk::SIZE
            )
    );

    Chunk::BlockLookupFunction blockLookup =
        [this](
            int worldX,
            int worldY,
            int worldZ
            ) -> uint16_t
        {
            return getBlock(
                worldX,
                worldY,
                worldZ
            );
        };

    std::vector<float> vertices =
        data.chunk.buildMesh(
            m_renderInfo,
            worldPosition,
            blockLookup
        );

    data.mesh =
        std::make_unique<Mesh>(
            vertices
        );
}

void World::printStreamingDebugStats() const
{
    int loadingCount = 0;
    int loadedCount = 0;
    int meshQueuedCount = 0;
    int meshingCount = 0;
    int readyToRenderCount = 0;
    int unloadPendingCount = 0;

    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        switch (data.streamingState)
        {
        case ChunkStreamingState::Loading:
            loadingCount++;
            break;

        case ChunkStreamingState::Loaded:
            loadedCount++;
            break;

        case ChunkStreamingState::MeshQueued:
            meshQueuedCount++;
            break;

        case ChunkStreamingState::Meshing:
            meshingCount++;
            break;

        case ChunkStreamingState::ReadyToRender:
            readyToRenderCount++;
            break;

        case ChunkStreamingState::UnloadPending:
            unloadPendingCount++;
            break;

        default:
            break;
        }
    }

    std::cout
        << "[Streaming] "
        << "PendingLoads: "
        << m_chunkManager.getPendingChunkLoadCount()
        << " | Loading: "
        << loadingCount
        << " | Loaded: "
        << loadedCount
        << " | MeshQueued: "
        << meshQueuedCount
        << " | Meshing: "
        << meshingCount
        << " | Ready: "
        << readyToRenderCount
        << " | UnloadPending: "
        << unloadPendingCount
        << std::endl;
}

void World::saveWorld()
{
    for (auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        if (!data.chunk.isSaveDirty())
        {
            continue;
        }

        if (m_saveManager.saveChunk(
            data.chunk,
            data.chunkX,
            data.chunkZ
        ))
        {
            data.chunk.clearSaveDirty();
        }
    }

    std::cout
        << "World saved."
        << std::endl;
}