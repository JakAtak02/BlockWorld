#include "world/World.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <thread>
#include <utility>


World::World(
    const std::vector<BlockRenderInfo>& renderInfo
)
    :
    m_renderInfo(renderInfo)
{
    m_saveManager.initialize();

    size_t workerThreadCount =
        std::max(
            1u,
            std::thread::hardware_concurrency() - 1
        );

    m_jobSystem.initialize(
        workerThreadCount
    );

    m_chunkManager.loadInitialChunks(
        m_terrainGenerator,
        m_saveManager
    );

    for (auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        rebuildChunkMesh(data);

        if (!data.pendingMeshVertices.empty())
        {
            data.mesh =
                std::make_unique<Mesh>(
                    data.pendingMeshVertices
                );

            data.pendingMeshVertices.clear();
        }

        data.chunk.clearMeshDirty();

        m_chunkManager.setChunkState(
            coord.x,
            coord.y,
            coord.z,
            ChunkStreamingState::MeshQueued
        );

        m_chunkManager.setChunkState(
            coord.x,
            coord.y,
            coord.z,
            ChunkStreamingState::MeshReadyForUpload
        );

        m_chunkManager.setChunkState(
            coord.x,
            coord.y,
            coord.z,
            ChunkStreamingState::ReadyToRender
        );
    }
}

void World::update(
    float deltaTime
)
{
    constexpr int MAX_ASYNC_RESULTS_PER_FRAME = 4;

    processCompletedChunkLoads(
        MAX_ASYNC_RESULTS_PER_FRAME
    );

    processCompletedMeshBuilds(
        MAX_ASYNC_RESULTS_PER_FRAME
    );

    int meshRebuildBudget =
        calculateMeshRebuildBudget(deltaTime);

    processMeshRebuildQueue(
        meshRebuildBudget
    );

    constexpr int MAX_MESH_UPLOADS_PER_FRAME = 2;

    processMeshUploadQueue(
        MAX_MESH_UPLOADS_PER_FRAME
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
        *this
    );
}

uint16_t World::getBlock(
    int worldX,
    int worldY,
    int worldZ
) const
{
    if (worldY < WORLD_MIN_Y ||
        worldY > WORLD_MAX_Y)
    {
        return 0;
    }

    int chunkX =
        floorDiv(worldX, Chunk::SIZE);

    int chunkY =
        floorDiv(worldY, Chunk::SIZE);

    int chunkZ =
        floorDiv(worldZ, Chunk::SIZE);

    int localX =
        positiveMod(worldX, Chunk::SIZE);

    int localY =
        positiveMod(worldY, Chunk::SIZE);

    int localZ =
        positiveMod(worldZ, Chunk::SIZE);

    const ChunkRenderData* data =
        m_chunkManager.findChunk(
            chunkX,
            chunkY,
            chunkZ
        );

    if (!data)
    {
        return 0;
    }

    return data->chunk.getBlock(
        localX,
        localY,
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
    if (worldY < WORLD_MIN_Y ||
        worldY > WORLD_MAX_Y)
    {
        return;
    }

    int chunkX =
        floorDiv(worldX, Chunk::SIZE);

    int chunkY =
        floorDiv(worldY, Chunk::SIZE);

    int chunkZ =
        floorDiv(worldZ, Chunk::SIZE);

    int localX =
        positiveMod(worldX, Chunk::SIZE);

    int localY =
        positiveMod(worldY, Chunk::SIZE);

    int localZ =
        positiveMod(worldZ, Chunk::SIZE);

    ChunkRenderData* data =
        m_chunkManager.getOrCreateChunk(
            chunkX,
            chunkY,
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
        localY,
        localZ,
        blockId
    );

    markChunkAndNeighborsDirty(
        chunkX,
        chunkY,
        chunkZ
    );
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
        static_cast<float>(blockPosition.x + 1) :
        static_cast<float>(blockPosition.x),

        step.y > 0 ?
        static_cast<float>(blockPosition.y + 1) :
        static_cast<float>(blockPosition.y),

        step.z > 0 ?
        static_cast<float>(blockPosition.z + 1) :
        static_cast<float>(blockPosition.z)
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
            hit.blockPosition = blockPosition;
            hit.previousBlockPosition = previousBlockPosition;
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

bool World::isChunkOccluded(
    const ChunkRenderData& chunkData,
    const glm::vec3& cameraPosition
) const
{
    glm::vec3 chunkCenter =
        (chunkData.minBounds +
            chunkData.maxBounds) * 0.5f;

    glm::vec3 directionToChunk =
        chunkCenter - cameraPosition;

    float distanceToChunk =
        glm::length(directionToChunk);

    constexpr float MIN_OCCLUSION_DISTANCE =
        Chunk::SIZE * 4.0f;

    if (distanceToChunk <
        MIN_OCCLUSION_DISTANCE)
    {
        return false;
    }

    directionToChunk =
        glm::normalize(directionToChunk);

    constexpr float STEP_SIZE =
        Chunk::SIZE * 0.5f;

    constexpr float MAX_OCCLUSION_DISTANCE =
        Chunk::SIZE * 8.0f;

    int blockingSamples = 0;

    float currentDistance =
        Chunk::SIZE * 1.5f;

    while (currentDistance <
        distanceToChunk &&
        currentDistance <
        MAX_OCCLUSION_DISTANCE)
    {
        glm::vec3 samplePosition =
            cameraPosition +
            directionToChunk *
            currentDistance;

        int sampleChunkX =
            floorDiv(
                static_cast<int>(
                    std::floor(samplePosition.x)
                    ),
                Chunk::SIZE
            );

        int sampleChunkY =
            floorDiv(
                static_cast<int>(
                    std::floor(samplePosition.y)
                    ),
                Chunk::SIZE
            );

        int sampleChunkZ =
            floorDiv(
                static_cast<int>(
                    std::floor(samplePosition.z)
                    ),
                Chunk::SIZE
            );

        if (sampleChunkX == chunkData.chunkX &&
            sampleChunkY == chunkData.chunkY &&
            sampleChunkZ == chunkData.chunkZ)
        {
            currentDistance += STEP_SIZE;
            continue;
        }

        const ChunkRenderData* blockingChunk =
            m_chunkManager.findChunk(
                sampleChunkX,
                sampleChunkY,
                sampleChunkZ
            );

        if (blockingChunk &&
            !blockingChunk->chunk.isEmpty() &&
            blockingChunk->mesh)
        {
            blockingSamples++;

            if (blockingSamples >= 3)
            {
                return true;
            }
        }
        else
        {
            blockingSamples = 0;
        }

        currentDistance += STEP_SIZE;
    }

    return false;
}

bool World::shouldRenderChunkLOD(
    const ChunkRenderData& chunkData,
    const glm::vec3& cameraPosition
) const
{
    glm::vec3 chunkCenter =
        (chunkData.minBounds +
            chunkData.maxBounds) * 0.5f;

    float distance =
        glm::distance(
            chunkCenter,
            cameraPosition
        );

    constexpr float FULL_RENDER_DISTANCE =
        Chunk::SIZE * 6.0f;

    constexpr float FAR_RENDER_DISTANCE =
        Chunk::SIZE * 8.0f;

    if (distance <= FULL_RENDER_DISTANCE)
    {
        return true;
    }

    if (distance >= FAR_RENDER_DISTANCE)
    {
        int checker =
            (chunkData.chunkX +
             chunkData.chunkZ +
             chunkData.chunkY) & 1;

        return checker == 0;
    }

    return true;
}

void World::draw(
    const Frustum& frustum
) const
{
    int visibleChunks = 0;

    int drawnChunks = 0;

    int occludedChunks = 0;

    int lodRejectedChunks = 0;

    int renderedVertices = 0;

    glm::vec3 cameraPosition =
        m_lastPlayerPosition;

    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        if (!frustum.isBoxVisible(
            data.minBounds,
            data.maxBounds
        ))
        {
            continue;
        }

        visibleChunks++;

        if (isChunkOccluded(
            data,
            cameraPosition
        ))
        {
            occludedChunks++;
            continue;
        }

        if (!shouldRenderChunkLOD(
            data,
            cameraPosition
        ))
        {
            lodRejectedChunks++;
            continue;
        }

        if (!data.mesh)
        {
            continue;
        }

        drawnChunks++;

        renderedVertices +=
            data.mesh->getVertexCount();

        data.mesh->draw();
    }

    m_debugMetrics.lastVisibleChunks =
        visibleChunks;

    m_debugMetrics.lastOccludedChunks =
        occludedChunks;

    m_debugMetrics.lastLODRejectedChunks =
        lodRejectedChunks;

    m_debugMetrics.lastDrawnChunks =
        drawnChunks;

    m_debugMetrics.lastRenderedVertices =
        renderedVertices;
}

void World::drawChunkBorders(
    DebugRenderer& debugRenderer,
    const glm::mat4& projection,
    const glm::mat4& view
) const
{
    std::set<std::pair<int, int>> drawnColumns;

    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        std::pair<int, int> columnKey =
        {
            coord.x,
            coord.z
        };

        if (drawnColumns.contains(columnKey))
        {
            continue;
        }

        drawnColumns.insert(columnKey);

        glm::vec3 minBounds(
            static_cast<float>(
                coord.x * Chunk::SIZE
                ),
            static_cast<float>(
                WORLD_MIN_Y
                ),
            static_cast<float>(
                coord.z * Chunk::SIZE
                )
        );

        glm::vec3 maxBounds(
            minBounds.x + static_cast<float>(Chunk::SIZE),
            static_cast<float>(WORLD_MAX_Y + 1),
            minBounds.z + static_cast<float>(Chunk::SIZE)
        );

        debugRenderer.drawChunkBorder(
            minBounds,
            maxBounds,
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
    int chunkY,
    int chunkZ
)
{
    if (isMeshRebuildQueued(
        chunkX,
        chunkY,
        chunkZ
    ))
    {
        return;
    }

    m_pendingMeshRebuilds.push_back(
        ChunkCoord
        {
            chunkX,
            chunkY,
            chunkZ
        }
    );
}

void World::enqueueChunkMeshIfDirty(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    ChunkRenderData* data =
        m_chunkManager.findChunk(
            chunkX,
            chunkY,
            chunkZ
        );

    if (!data)
    {
        return;
    }

    if (!data->chunk.isMeshDirty())
    {
        return;
    }

    enqueueMeshRebuild(
        chunkX,
        chunkY,
        chunkZ
    );
}

void World::markChunkDirtyOnly(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    ChunkRenderData* data =
        m_chunkManager.findChunk(
            chunkX,
            chunkY,
            chunkZ
        );

    if (!data)
    {
        return;
    }

    data->chunk.markMeshDirty();

    m_chunkManager.setChunkState(
        chunkX,
        chunkY,
        chunkZ,
        ChunkStreamingState::MeshQueued
    );

    enqueueChunkMeshIfDirty(
        chunkX,
        chunkY,
        chunkZ
    );
}

void World::markChunkAndNeighborsDirty(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    auto markDirty =
        [this](
            int targetChunkX,
            int targetChunkY,
            int targetChunkZ
            )
        {
            ChunkRenderData* data =
                m_chunkManager.findChunk(
                    targetChunkX,
                    targetChunkY,
                    targetChunkZ
                );

            if (!data)
            {
                return;
            }

            data->chunk.markMeshDirty();

            m_chunkManager.setChunkState(
                targetChunkX,
                targetChunkY,
                targetChunkZ,
                ChunkStreamingState::MeshQueued
            );

            enqueueChunkMeshIfDirty(
                targetChunkX,
                targetChunkY,
                targetChunkZ
            );
        };

    markDirty(chunkX, chunkY, chunkZ);

    markDirty(chunkX - 1, chunkY, chunkZ);
    markDirty(chunkX + 1, chunkY, chunkZ);

    markDirty(chunkX, chunkY - 1, chunkZ);
    markDirty(chunkX, chunkY + 1, chunkZ);

    markDirty(chunkX, chunkY, chunkZ - 1);
    markDirty(chunkX, chunkY, chunkZ + 1);
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

    int playerChunkY =
        floorDiv(
            static_cast<int>(
                std::floor(
                    m_lastPlayerPosition.y
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
        [playerChunkX, playerChunkY, playerChunkZ](
            const ChunkCoord& left,
            const ChunkCoord& right
            )
        {
            int leftDistanceX =
                left.x - playerChunkX;

            int leftDistanceY =
                left.y - playerChunkY;

            int leftDistanceZ =
                left.z - playerChunkZ;

            int rightDistanceX =
                right.x - playerChunkX;

            int rightDistanceY =
                right.y - playerChunkY;

            int rightDistanceZ =
                right.z - playerChunkZ;

            int leftDistanceSquared =
                leftDistanceX * leftDistanceX +
                leftDistanceY * leftDistanceY +
                leftDistanceZ * leftDistanceZ;

            int rightDistanceSquared =
                rightDistanceX * rightDistanceX +
                rightDistanceY * rightDistanceY +
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
                coord.y,
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

        if (data->meshBuildInProgress)
        {
            continue;
        }

        if (data->chunk.isEmpty())
        {
            data->mesh.reset();
            data->pendingMeshVertices.clear();
            data->meshUploadQueued = false;

            data->chunk.clearMeshDirty();

            m_chunkManager.setChunkState(
                coord.x,
                coord.y,
                coord.z,
                ChunkStreamingState::ReadyToRender
            );

            continue;
        }

        m_chunkManager.setChunkState(
            coord.x,
            coord.y,
            coord.z,
            ChunkStreamingState::Meshing
        );

        ChunkMeshBuildSnapshot snapshot =
            createMeshBuildSnapshot(*data);

        data->activeMeshRequestId =
            submitAsyncMeshBuild(
                snapshot
            );

        data->meshBuildInProgress = true;

        data->chunk.clearMeshDirty();

        meshesRebuilt++;
    }
}

void World::processMeshUploadQueue(
    int maxMeshUploads
)
{
    int uploadsProcessed = 0;

    while (!m_pendingMeshUploads.empty() &&
        uploadsProcessed < maxMeshUploads)
    {
        ChunkCoord coord =
            m_pendingMeshUploads.front();

        m_pendingMeshUploads.pop_front();

        ChunkRenderData* data =
            m_chunkManager.findChunk(
                coord.x,
                coord.y,
                coord.z
            );

        if (!data)
        {
            continue;
        }

        if (!data->pendingMeshVertices.empty())
        {
            data->mesh =
                std::make_unique<Mesh>(
                    data->pendingMeshVertices
                );
        }
        else
        {
            data->mesh.reset();
        }

        data->pendingMeshVertices.clear();

        data->meshUploadQueued = false;

        m_chunkManager.setChunkState(
            coord.x,
            coord.y,
            coord.z,
            ChunkStreamingState::ReadyToRender
        );

        uploadsProcessed++;
    }
}

bool World::isMeshRebuildQueued(
    int chunkX,
    int chunkY,
    int chunkZ
) const
{
    for (const ChunkCoord& coord :
        m_pendingMeshRebuilds)
    {
        if (coord.x == chunkX &&
            coord.y == chunkY &&
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
        static_cast<float>(
            data.chunkY * Chunk::SIZE
            ),
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

    data.pendingMeshVertices =
        data.chunk.buildMesh(
            m_renderInfo,
            worldPosition,
            blockLookup
        );
}

void World::printStreamingDebugStats() const
{
    int loadingCount = 0;
    int loadedCount = 0;
    int meshQueuedCount = 0;
    int meshingCount = 0;
    int meshReadyForUploadCount = 0;
    int readyToRenderCount = 0;
    int unloadPendingCount = 0;

    int emptyChunkCount = 0;
    int meshChunkCount = 0;
    int totalVertexCount = 0;
    size_t totalMeshMemory = 0;

    int totalChunks = 0;

    for (const auto& [coord, data] :
        m_chunkManager.getChunks())
    {
        totalChunks++;

        if (data.chunk.isEmpty())
        {
            emptyChunkCount++;
        }

        if (data.mesh)
        {
            meshChunkCount++;

            totalVertexCount +=
                data.mesh->getVertexCount();

            totalMeshMemory +=
                data.mesh->getEstimatedMemoryUsage();
        }

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

        case ChunkStreamingState::MeshReadyForUpload:
            meshReadyForUploadCount++;
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
        << "\n========== STREAMING DEBUG =========="
        << "\nTotal Chunks: "
        << totalChunks

        << "\nEmpty Chunks: "
        << emptyChunkCount

        << "\nChunks With Meshes: "
        << meshChunkCount

        << "\nTotal Mesh Vertices: "
        << totalVertexCount

        << "\nEstimated Mesh Memory: "
        << (totalMeshMemory / 1024)
        << " KB"

        << "\nPending Queue: "
        << m_chunkManager.getPendingChunkLoadCount()

        << "\nPending Mesh Rebuilds: "
        << m_pendingMeshRebuilds.size()

        << "\nPending Mesh Uploads: "
        << m_pendingMeshUploads.size()

        << "\nCompleted Chunk Results: "
        << m_completedChunkLoads.size()

        << "\nCompleted Mesh Results: "
        << m_completedMeshBuilds.size()

        << "\nPending JobSystem Jobs: "
        << m_jobSystem.getPendingJobCount()

        << "\nLoading: "
        << loadingCount

        << "\nLoaded: "
        << loadedCount

        << "\nMeshQueued: "
        << meshQueuedCount

        << "\nMeshing: "
        << meshingCount

        << "\nMeshReadyForUpload: "
        << meshReadyForUploadCount

        << "\nReadyToRender: "
        << readyToRenderCount

        << "\nUnloadPending: "
        << unloadPendingCount

        << "\nLast Visible Chunks: "
        << m_debugMetrics.lastVisibleChunks

        << "\nLast Occluded Chunks: "
        << m_debugMetrics.lastOccludedChunks

        << "\nLast LOD Rejected Chunks: "
        << m_debugMetrics.lastLODRejectedChunks
        
        << "\nLast Drawn Chunks: "
        << m_debugMetrics.lastDrawnChunks

        << "\nLast Rendered Vertices: "
        << m_debugMetrics.lastRenderedVertices

        << "\n===================================="
        << std::endl;
}

uint64_t World::submitAsyncChunkLoad(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    uint64_t requestId =
        m_nextChunkRequestId++;

    m_jobSystem.submit(
        [this, chunkX, chunkY, chunkZ, requestId]()
        {
            AsyncChunkLoadResult result;

            result.chunkX = chunkX;
            result.chunkY = chunkY;
            result.chunkZ = chunkZ;
            result.requestId = requestId;

            if (m_saveManager.loadChunk(
                result.chunk,
                chunkX,
                chunkY,
                chunkZ
            ))
            {
                result.resultType =
                    AsyncChunkLoadResultType
                    ::LoadedFromDisk;
            }
            else
            {
                m_terrainGenerator.generateChunk(
                    result.chunk,
                    chunkX,
                    chunkY,
                    chunkZ
                );

                result.resultType =
                    AsyncChunkLoadResultType
                    ::Generated;
            }

            m_completedChunkLoads.push(
                std::move(result)
            );
        }
    );

    return requestId;
}

uint64_t World::submitAsyncMeshBuild(
    const ChunkMeshBuildSnapshot& snapshot
)
{
    uint64_t requestId =
        m_nextChunkRequestId++;

    ChunkMeshBuildSnapshot snapshotCopy =
        snapshot;

    snapshotCopy.requestId =
        requestId;

    m_jobSystem.submit(
        [this, snapshotCopy]()
        {
            AsyncMeshBuildResult result;

            result.chunkX =
                snapshotCopy.chunkX;

            result.chunkY =
                snapshotCopy.chunkY;

            result.chunkZ =
                snapshotCopy.chunkZ;

            result.requestId =
                snapshotCopy.requestId;

            glm::vec3 worldPosition(
                static_cast<float>(
                    snapshotCopy.chunkX *
                    Chunk::SIZE
                    ),
                static_cast<float>(
                    snapshotCopy.chunkY *
                    Chunk::SIZE
                    ),
                static_cast<float>(
                    snapshotCopy.chunkZ *
                    Chunk::SIZE
                    )
            );

            Chunk::BlockLookupFunction blockLookup =
                [snapshotCopy](
                    int worldX,
                    int worldY,
                    int worldZ
                    ) -> uint16_t
                {
                    int chunkX =
                        floorDiv(worldX, Chunk::SIZE);

                    int chunkY =
                        floorDiv(worldY, Chunk::SIZE);

                    int chunkZ =
                        floorDiv(worldZ, Chunk::SIZE);

                    int localX =
                        positiveMod(worldX, Chunk::SIZE);

                    int localY =
                        positiveMod(worldY, Chunk::SIZE);

                    int localZ =
                        positiveMod(worldZ, Chunk::SIZE);

                    if (chunkX ==
                        snapshotCopy.chunkX &&
                        chunkY ==
                        snapshotCopy.chunkY &&
                        chunkZ ==
                        snapshotCopy.chunkZ)
                    {
                        return snapshotCopy
                            .centerChunk.getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX - 1 &&
                        chunkY ==
                        snapshotCopy.chunkY &&
                        chunkZ ==
                        snapshotCopy.chunkZ &&
                        snapshotCopy
                        .hasNegativeXNeighbor)
                    {
                        return snapshotCopy
                            .negativeXNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX + 1 &&
                        chunkY ==
                        snapshotCopy.chunkY &&
                        chunkZ ==
                        snapshotCopy.chunkZ &&
                        snapshotCopy
                        .hasPositiveXNeighbor)
                    {
                        return snapshotCopy
                            .positiveXNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX &&
                        chunkY ==
                        snapshotCopy.chunkY - 1 &&
                        chunkZ ==
                        snapshotCopy.chunkZ &&
                        snapshotCopy
                        .hasNegativeYNeighbor)
                    {
                        return snapshotCopy
                            .negativeYNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX &&
                        chunkY ==
                        snapshotCopy.chunkY + 1 &&
                        chunkZ ==
                        snapshotCopy.chunkZ &&
                        snapshotCopy
                        .hasPositiveYNeighbor)
                    {
                        return snapshotCopy
                            .positiveYNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX &&
                        chunkY ==
                        snapshotCopy.chunkY &&
                        chunkZ ==
                        snapshotCopy.chunkZ - 1 &&
                        snapshotCopy
                        .hasNegativeZNeighbor)
                    {
                        return snapshotCopy
                            .negativeZNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    if (chunkX ==
                        snapshotCopy.chunkX &&
                        chunkY ==
                        snapshotCopy.chunkY &&
                        chunkZ ==
                        snapshotCopy.chunkZ + 1 &&
                        snapshotCopy
                        .hasPositiveZNeighbor)
                    {
                        return snapshotCopy
                            .positiveZNeighbor
                            .getBlock(
                                localX,
                                localY,
                                localZ
                            );
                    }

                    return 0;
                };

            result.vertices =
                snapshotCopy.centerChunk
                .buildMesh(
                    m_renderInfo,
                    worldPosition,
                    blockLookup
                );

            m_completedMeshBuilds.push(
                std::move(result)
            );
        }
    );

    return requestId;
}

void World::processCompletedChunkLoads(
    int maxResultsToProcess
)
{
    int processedResults = 0;

    while (processedResults <
        maxResultsToProcess)
    {
        auto result =
            m_completedChunkLoads.tryPop();

        if (!result)
        {
            break;
        }

        ChunkRenderData* data =
            m_chunkManager.findChunk(
                result->chunkX,
                result->chunkY,
                result->chunkZ
            );

        if (!data)
        {
            continue;
        }

        if (data->activeRequestId !=
            result->requestId)
        {
            continue;
        }

        data->chunk =
            std::move(result->chunk);

        data->streamingState =
            ChunkStreamingState::Loaded;

        markChunkDirtyOnly(
            result->chunkX,
            result->chunkY,
            result->chunkZ
        );

        processedResults++;
    }
}

void World::processCompletedMeshBuilds(
    int maxResultsToProcess
)
{
    int processedResults = 0;

    while (processedResults <
        maxResultsToProcess)
    {
        auto result =
            m_completedMeshBuilds.tryPop();

        if (!result)
        {
            break;
        }

        ChunkRenderData* data =
            m_chunkManager.findChunk(
                result->chunkX,
                result->chunkY,
                result->chunkZ
            );

        if (!data)
        {
            continue;
        }

        if (data->activeMeshRequestId !=
            result->requestId)
        {
            data->meshBuildInProgress = false;
            continue;
        }

        data->meshBuildInProgress = false;

        data->pendingMeshVertices =
            std::move(result->vertices);

        m_chunkManager.setChunkState(
            result->chunkX,
            result->chunkY,
            result->chunkZ,
            ChunkStreamingState::MeshReadyForUpload
        );

        if (!data->meshUploadQueued)
        {
            m_pendingMeshUploads.push_back(
                ChunkCoord
                {
                    result->chunkX,
                    result->chunkY,
                    result->chunkZ
                }
            );

            data->meshUploadQueued = true;
        }

        processedResults++;
    }
}

ChunkMeshBuildSnapshot World::createMeshBuildSnapshot(
    const ChunkRenderData& data
) const
{
    ChunkMeshBuildSnapshot snapshot;

    snapshot.chunkX =
        data.chunkX;

    snapshot.chunkY =
        data.chunkY;

    snapshot.chunkZ =
        data.chunkZ;

    snapshot.centerChunk =
        data.chunk;

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX - 1,
            data.chunkY,
            data.chunkZ
        ))
    {
        snapshot.hasNegativeXNeighbor = true;
        snapshot.negativeXNeighbor =
            neighbor->chunk;
    }

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX + 1,
            data.chunkY,
            data.chunkZ
        ))
    {
        snapshot.hasPositiveXNeighbor = true;
        snapshot.positiveXNeighbor =
            neighbor->chunk;
    }

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX,
            data.chunkY - 1,
            data.chunkZ
        ))
    {
        snapshot.hasNegativeYNeighbor = true;
        snapshot.negativeYNeighbor =
            neighbor->chunk;
    }

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX,
            data.chunkY + 1,
            data.chunkZ
        ))
    {
        snapshot.hasPositiveYNeighbor = true;
        snapshot.positiveYNeighbor =
            neighbor->chunk;
    }

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX,
            data.chunkY,
            data.chunkZ - 1
        ))
    {
        snapshot.hasNegativeZNeighbor = true;
        snapshot.negativeZNeighbor =
            neighbor->chunk;
    }

    if (const ChunkRenderData* neighbor =
        m_chunkManager.findChunk(
            data.chunkX,
            data.chunkY,
            data.chunkZ + 1
        ))
    {
        snapshot.hasPositiveZNeighbor = true;
        snapshot.positiveZNeighbor =
            neighbor->chunk;
    }

    return snapshot;
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
            data.chunkY,
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
