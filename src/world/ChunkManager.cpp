#include "world/ChunkManager.h"

#include <algorithm>
#include <cmath>
#include <vector>

void ChunkManager::loadInitialChunks(
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    for (int chunkZ = -1; chunkZ <= 1; chunkZ++)
    {
        for (int chunkX = -1; chunkX <= 1; chunkX++)
        {
            createOrLoadChunk(
                chunkX,
                chunkZ,
                terrainGenerator,
                saveManager
            );
        }
    }
}

void ChunkManager::createOrLoadChunk(
    int chunkX,
    int chunkZ,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    if (findChunk(chunkX, chunkZ))
    {
        return;
    }

    ChunkRenderData data;

    data.chunkX = chunkX;
    data.chunkZ = chunkZ;

    data.streamingState =
        ChunkStreamingState::Loading;

    data.minBounds = glm::vec3(
        static_cast<float>(chunkX * Chunk::SIZE),
        0.0f,
        static_cast<float>(chunkZ * Chunk::SIZE)
    );

    data.maxBounds = glm::vec3(
        data.minBounds.x + static_cast<float>(Chunk::SIZE),
        static_cast<float>(Chunk::SIZE),
        data.minBounds.z + static_cast<float>(Chunk::SIZE)
    );

    if (!saveManager.loadChunk(
        data.chunk,
        chunkX,
        chunkZ
    ))
    {
        terrainGenerator.generateChunk(
            data.chunk,
            chunkX,
            chunkZ
        );

        saveManager.saveChunk(
            data.chunk,
            chunkX,
            chunkZ
        );

        data.chunk.clearSaveDirty();
    }

    data.streamingState =
        ChunkStreamingState::Loaded;

    ChunkCoord coord
    {
        chunkX,
        chunkZ
    };

    m_chunks.emplace(
        coord,
        std::move(data)
    );
}

ChunkRenderData* ChunkManager::findChunk(
    int chunkX,
    int chunkZ
)
{
    auto it =
        m_chunks.find(
            ChunkCoord
            {
                chunkX,
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
    int chunkZ
) const
{
    auto it =
        m_chunks.find(
            ChunkCoord
            {
                chunkX,
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
    int chunkZ,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    if (ChunkRenderData* existing =
        findChunk(chunkX, chunkZ))
    {
        return existing;
    }

    createOrLoadChunk(
        chunkX,
        chunkZ,
        terrainGenerator,
        saveManager
    );

    return findChunk(
        chunkX,
        chunkZ
    );
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
            if (findChunk(chunkX, chunkZ))
            {
                continue;
            }

            if (isChunkLoadQueued(chunkX, chunkZ))
            {
                continue;
            }

            chunksToQueue.push_back(
                ChunkCoord
                {
                    chunkX,
                    chunkZ
                }
            );
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

    for (const ChunkCoord& coord :
        chunksToQueue)
    {
        enqueueChunkLoad(
            coord.x,
            coord.z
        );
    }
}

void ChunkManager::processChunkLoadQueue(
    int maxChunksToLoad,
    TerrainGenerator& terrainGenerator,
    WorldSaveManager& saveManager
)
{
    int chunksLoaded = 0;

    while (!m_pendingChunkLoads.empty() &&
        chunksLoaded < maxChunksToLoad)
    {
        ChunkCoord coord =
            m_pendingChunkLoads.front();

        m_pendingChunkLoads.pop_front();

        if (findChunk(coord.x, coord.z))
        {
            continue;
        }

        createOrLoadChunk(
            coord.x,
            coord.z,
            terrainGenerator,
            saveManager
        );

        chunksLoaded++;
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

int ChunkManager::getPendingChunkLoadCount() const
{
    return static_cast<int>(
        m_pendingChunkLoads.size()
        );
}

void ChunkManager::enqueueChunkLoad(
    int chunkX,
    int chunkZ
)
{
    if (findChunk(chunkX, chunkZ))
    {
        return;
    }

    if (isChunkLoadQueued(
        chunkX,
        chunkZ
    ))
    {
        return;
    }

    m_pendingChunkLoads.push_back(
        ChunkCoord
        {
            chunkX,
            chunkZ
        }
    );
}

bool ChunkManager::isChunkLoadQueued(
    int chunkX,
    int chunkZ
) const
{
    for (const ChunkCoord& coord :
        m_pendingChunkLoads)
    {
        if (coord.x == chunkX &&
            coord.z == chunkZ)
        {
            return true;
        }
    }

    return false;
}

void ChunkManager::setChunkState(
    int chunkX,
    int chunkZ,
    ChunkStreamingState state
)
{
    ChunkRenderData* data =
        findChunk(
            chunkX,
            chunkZ
        );

    if (!data)
    {
        return;
    }

    data->streamingState = state;
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