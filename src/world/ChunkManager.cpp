#include "world/ChunkManager.h"

#include "world/World.h"

#include <algorithm>
#include <cmath>
#include <vector>

static constexpr int ACTIVE_VERTICAL_CHUNK_MIN_Y = 0;
static constexpr int ACTIVE_VERTICAL_CHUNK_MAX_Y = 15;
static constexpr int INITIAL_SYNC_LOAD_RADIUS_CHUNKS = 0;
static constexpr int MAX_CHUNKS_QUEUED_PER_UPDATE = 64;

void ChunkManager::loadInitialChunks(
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    for (int chunkZ = -INITIAL_SYNC_LOAD_RADIUS_CHUNKS;
        chunkZ <= INITIAL_SYNC_LOAD_RADIUS_CHUNKS;
        chunkZ++)
    {
        for (int chunkX = -INITIAL_SYNC_LOAD_RADIUS_CHUNKS;
            chunkX <= INITIAL_SYNC_LOAD_RADIUS_CHUNKS;
            chunkX++)
        {
            std::vector<int> chunkYsToLoad =
                getChunkYsToLoadForColumn(
                    chunkX,
                    chunkZ,
                    terrainGenerator,
                    saveManager
                );

            for (int chunkY :
            chunkYsToLoad)
            {
                createOrLoadChunk(
                    chunkX,
                    chunkY,
                    chunkZ,
                    terrainGenerator,
                    saveManager
                );

                setChunkState(
                    chunkX,
                    chunkY,
                    chunkZ,
                    ChunkStreamingState::ReadyToRender
                );
            }
        }
    }
}

void ChunkManager::createOrLoadChunk(
    int chunkX,
    int chunkY,
    int chunkZ,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    if (findChunk(chunkX, chunkY, chunkZ))
    {
        return;
    }

    ChunkRenderData data;

    data.chunkX = chunkX;
    data.chunkY = chunkY;
    data.chunkZ = chunkZ;

    data.streamingState =
        ChunkStreamingState::Loaded;

    data.activeRequestId = 0;

    data.minBounds = glm::vec3(
        static_cast<float>(chunkX * Chunk::SIZE),
        static_cast<float>(chunkY * Chunk::SIZE),
        static_cast<float>(chunkZ * Chunk::SIZE)
    );

    data.maxBounds = glm::vec3(
        data.minBounds.x + static_cast<float>(Chunk::SIZE),
        data.minBounds.y + static_cast<float>(Chunk::SIZE),
        data.minBounds.z + static_cast<float>(Chunk::SIZE)
    );

    if (!saveManager.loadChunk(
        data.chunk,
        chunkX,
        chunkY,
        chunkZ
    ))
    {
        terrainGenerator.generateChunk(
            data.chunk,
            chunkX,
            chunkY,
            chunkZ
        );

        if (!data.chunk.isEmpty())
        {
            saveManager.saveChunk(
                data.chunk,
                chunkX,
                chunkY,
                chunkZ
            );
        }

        data.chunk.clearSaveDirty();
    }

    ChunkCoord coord
    {
        chunkX,
        chunkY,
        chunkZ
    };

    m_chunks.emplace(
        coord,
        std::move(data)
    );
}

ChunkRenderData* ChunkManager::findChunk(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    auto it =
        m_chunks.find(
            ChunkCoord
            {
                chunkX,
                chunkY,
                chunkZ
            }
        );

    if (it == m_chunks.end())
    {
        return nullptr;
    }

    return &it->second;
}

const ChunkRenderData* ChunkManager::findChunk(
    int chunkX,
    int chunkY,
    int chunkZ
) const
{
    auto it =
        m_chunks.find(
            ChunkCoord
            {
                chunkX,
                chunkY,
                chunkZ
            }
        );

    if (it == m_chunks.end())
    {
        return nullptr;
    }

    return &it->second;
}

ChunkRenderData* ChunkManager::getOrCreateChunk(
    int chunkX,
    int chunkY,
    int chunkZ,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    if (ChunkRenderData* existing =
        findChunk(chunkX, chunkY, chunkZ))
    {
        return existing;
    }

    createOrLoadChunk(
        chunkX,
        chunkY,
        chunkZ,
        terrainGenerator,
        saveManager
    );

    return findChunk(
        chunkX,
        chunkY,
        chunkZ
    );
}

std::vector<int> ChunkManager::getChunkYsToLoadForColumn(
    int chunkX,
    int chunkZ,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
) const
{
    std::vector<int> chunkYsToLoad;

    int highestTerrainChunkY =
        terrainGenerator.getHighestTerrainChunkY(
            chunkX,
            chunkZ
        );

    int maxTerrainChunkY =
        std::min(
            highestTerrainChunkY + 1,
            ACTIVE_VERTICAL_CHUNK_MAX_Y
        );

    for (int chunkY = ACTIVE_VERTICAL_CHUNK_MIN_Y;
        chunkY <= maxTerrainChunkY;
        chunkY++)
    {
        chunkYsToLoad.push_back(
            chunkY
        );
    }

    std::vector<int> savedChunkYs =
        saveManager.getSavedChunkYs(
            chunkX,
            chunkZ
        );

    for (int savedChunkY :
    savedChunkYs)
    {
        if (std::find(
            chunkYsToLoad.begin(),
            chunkYsToLoad.end(),
            savedChunkY
        ) ==
            chunkYsToLoad.end())
        {
            chunkYsToLoad.push_back(
                savedChunkY
            );
        }
    }

    std::sort(
        chunkYsToLoad.begin(),
        chunkYsToLoad.end()
    );

    return chunkYsToLoad;
}

void ChunkManager::ensureChunksAroundPosition(
    const glm::vec3& worldPosition,
    const glm::vec3& forwardDirection,
    int loadRadiusChunks,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    int playerChunkX =
        worldToChunkCoord(worldPosition.x);

    int playerChunkZ =
        worldToChunkCoord(worldPosition.z);

    glm::vec2 forward2D(
        forwardDirection.x,
        forwardDirection.z
    );

    if (glm::length(forward2D) > 0.001f)
    {
        forward2D =
            glm::normalize(forward2D);
    }
    else
    {
        forward2D =
            glm::vec2(0.0f, -1.0f);
    }

    std::vector<ChunkCoord> chunksToQueue;

    for (int chunkZ = playerChunkZ - loadRadiusChunks;
        chunkZ <= playerChunkZ + loadRadiusChunks;
        chunkZ++)
    {
        for (int chunkX = playerChunkX - loadRadiusChunks;
            chunkX <= playerChunkX + loadRadiusChunks;
            chunkX++)
        {
            std::vector<int> chunkYsToLoad =
                getChunkYsToLoadForColumn(
                    chunkX,
                    chunkZ,
                    terrainGenerator,
                    saveManager
                );

            for (int chunkY :
            chunkYsToLoad)
            {
                if (findChunk(
                    chunkX,
                    chunkY,
                    chunkZ
                ))
                {
                    continue;
                }

                if (isChunkLoadQueued(
                    chunkX,
                    chunkY,
                    chunkZ
                ))
                {
                    continue;
                }

                chunksToQueue.push_back(
                    ChunkCoord
                    {
                        chunkX,
                        chunkY,
                        chunkZ
                    }
                );
            }
        }
    }

    std::sort(
        chunksToQueue.begin(),
        chunksToQueue.end(),
        [playerChunkX, playerChunkZ, forward2D](
            const ChunkCoord& left,
            const ChunkCoord& right
            )
        {
            glm::vec2 leftOffset(
                static_cast<float>(
                    left.x - playerChunkX
                    ),
                static_cast<float>(
                    left.z - playerChunkZ
                    )
            );

            glm::vec2 rightOffset(
                static_cast<float>(
                    right.x - playerChunkX
                    ),
                static_cast<float>(
                    right.z - playerChunkZ
                    )
            );

            float leftDistanceSquared =
                glm::dot(
                    leftOffset,
                    leftOffset
                );

            float rightDistanceSquared =
                glm::dot(
                    rightOffset,
                    rightOffset
                );

            float leftForwardAmount = 0.0f;

            if (leftDistanceSquared > 0.001f)
            {
                leftForwardAmount =
                    glm::dot(
                        glm::normalize(leftOffset),
                        forward2D
                    );
            }

            float rightForwardAmount = 0.0f;

            if (rightDistanceSquared > 0.001f)
            {
                rightForwardAmount =
                    glm::dot(
                        glm::normalize(rightOffset),
                        forward2D
                    );
            }

            constexpr float
                FORWARD_PRIORITY_WEIGHT =
                1.5f;

            float leftScore =
                leftDistanceSquared -
                leftForwardAmount *
                FORWARD_PRIORITY_WEIGHT;

            float rightScore =
                rightDistanceSquared -
                rightForwardAmount *
                FORWARD_PRIORITY_WEIGHT;

            return leftScore <
                rightScore;
        }
    );

    int queuedThisUpdate = 0;

    for (const ChunkCoord& coord :
        chunksToQueue)
    {
        if (queuedThisUpdate >=
            MAX_CHUNKS_QUEUED_PER_UPDATE)
        {
            break;
        }

        enqueueChunkLoad(
            coord.x,
            coord.y,
            coord.z
        );

        queuedThisUpdate++;
    }
}

void ChunkManager::processChunkLoadQueue(
    int maxChunksToLoad,
    World& world
)
{
    int chunksSubmitted = 0;

    while (!m_pendingChunkLoads.empty() &&
        chunksSubmitted < maxChunksToLoad)
    {
        ChunkCoord coord =
            m_pendingChunkLoads.front();

        m_pendingChunkLoads.pop_front();

        if (findChunk(coord.x, coord.y, coord.z))
        {
            continue;
        }

        ChunkRenderData placeholder;

        placeholder.chunkX =
            coord.x;

        placeholder.chunkY =
            coord.y;

        placeholder.chunkZ =
            coord.z;

        placeholder.streamingState =
            ChunkStreamingState::Loading;

        placeholder.minBounds =
            glm::vec3(
                static_cast<float>(
                    coord.x * Chunk::SIZE
                    ),
                static_cast<float>(
                    coord.y * Chunk::SIZE
                    ),
                static_cast<float>(
                    coord.z * Chunk::SIZE
                    )
            );

        placeholder.maxBounds =
            glm::vec3(
                placeholder.minBounds.x +
                static_cast<float>(
                    Chunk::SIZE
                    ),

                placeholder.minBounds.y +
                static_cast<float>(
                    Chunk::SIZE
                    ),

                placeholder.minBounds.z +
                static_cast<float>(
                    Chunk::SIZE
                    )
            );

        placeholder.activeRequestId =
            world.submitAsyncChunkLoad(
                coord.x,
                coord.y,
                coord.z
            );

        m_chunks.emplace(
            coord,
            std::move(placeholder)
        );

        chunksSubmitted++;
    }
}

void ChunkManager::unloadChunksFarFromPosition(
    const glm::vec3& worldPosition,
    int unloadRadiusChunks,
    WorldSaveManager& saveManager
)
{
    int playerChunkX =
        worldToChunkCoord(worldPosition.x);

    int playerChunkZ =
        worldToChunkCoord(worldPosition.z);

    std::vector<ChunkCoord> chunksToUnload;

    for (const auto& [coord, data] :
        m_chunks)
    {
        int distanceX =
            std::abs(
                coord.x -
                playerChunkX
            );

        int distanceZ =
            std::abs(
                coord.z -
                playerChunkZ
            );

        if (distanceX > unloadRadiusChunks ||
            distanceZ > unloadRadiusChunks)
        {
            chunksToUnload.push_back(coord);
        }
    }

    for (const ChunkCoord& coord :
        chunksToUnload)
    {
        auto it =
            m_chunks.find(coord);

        if (it == m_chunks.end())
        {
            continue;
        }

        ChunkRenderData& data =
            it->second;

        data.streamingState =
            ChunkStreamingState::UnloadPending;

        if (data.chunk.isSaveDirty())
        {
            if (saveManager.saveChunk(
                data.chunk,
                data.chunkX,
                data.chunkY,
                data.chunkZ
            ))
            {
                data.chunk.clearSaveDirty();
            }
        }

        m_chunks.erase(it);
    }

    for (auto it =
        m_pendingChunkLoads.begin();
        it != m_pendingChunkLoads.end();)
    {
        int distanceX =
            std::abs(
                it->x -
                playerChunkX
            );

        int distanceZ =
            std::abs(
                it->z -
                playerChunkZ
            );

        if (distanceX > unloadRadiusChunks ||
            distanceZ > unloadRadiusChunks)
        {
            it =
                m_pendingChunkLoads.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ChunkManager::enqueueChunkLoad(
    int chunkX,
    int chunkY,
    int chunkZ
)
{
    if (findChunk(chunkX, chunkY, chunkZ))
    {
        return;
    }

    if (isChunkLoadQueued(
        chunkX,
        chunkY,
        chunkZ
    ))
    {
        return;
    }

    m_pendingChunkLoads.push_back(
        ChunkCoord
        {
            chunkX,
            chunkY,
            chunkZ
        }
    );
}

bool ChunkManager::isChunkLoadQueued(
    int chunkX,
    int chunkY,
    int chunkZ
) const
{
    for (const ChunkCoord& coord :
        m_pendingChunkLoads)
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

void ChunkManager::setChunkState(
    int chunkX,
    int chunkY,
    int chunkZ,
    ChunkStreamingState state
)
{
    ChunkRenderData* data =
        findChunk(chunkX, chunkY, chunkZ);

    if (!data)
    {
        return;
    }

    data->streamingState = state;
}

size_t ChunkManager::getPendingChunkLoadCount() const
{
    return m_pendingChunkLoads.size();
}

int ChunkManager::worldToChunkCoord(
    float worldCoordinate
)
{
    return static_cast<int>(
        std::floor(
            worldCoordinate /
            static_cast<float>(
                Chunk::SIZE
                )
        )
        );
}

std::unordered_map<
    ChunkCoord,
    ChunkRenderData,
    ChunkCoordHash
>& ChunkManager::getChunks()
{
    return m_chunks;
}

const std::unordered_map<
    ChunkCoord,
    ChunkRenderData,
    ChunkCoordHash
>& ChunkManager::getChunks() const
{
    return m_chunks;
}