#include "world/World.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <json.hpp>

World::World(const std::vector<BlockRenderInfo>& renderInfo)
    : m_renderInfo(renderInfo)
{
    namespace fs = std::filesystem;

    fs::create_directories(
        getWorldSavePath() + "/chunks"
    );

    if (!worldMetadataExists())
    {
        createWorldMetadataFile();
    }

    for (int chunkZ = -1; chunkZ <= 1; chunkZ++)
    {
        for (int chunkX = -1; chunkX <= 1; chunkX++)
        {
            ChunkRenderData data;

            data.chunkX = chunkX;
            data.chunkZ = chunkZ;

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

            std::string chunkPath =
                getChunkSavePath(chunkX, chunkZ);

            if (!data.chunk.loadFromFile(chunkPath))
            {
                data.chunk.generateTerrain(chunkX, chunkZ);

                data.chunk.saveToFile(
                    chunkPath,
                    chunkX,
                    chunkZ
                );

                data.chunk.clearSaveDirty();
            }

            ChunkCoord coord{ chunkX, chunkZ };

            m_chunks.emplace(coord, std::move(data));
        }
    }

    for (auto& [coord, data] : m_chunks)
    {
        rebuildChunkMesh(data);

        data.chunk.clearMeshDirty();
    }
}

void World::update()
{
    for (auto& [coord, data] : m_chunks)
    {
        if (!data.chunk.isMeshDirty())
        {
            continue;
        }

        rebuildChunkMesh(data);

        data.chunk.clearMeshDirty();
    }
}

uint16_t World::getBlock(
    int worldX,
    int worldY,
    int worldZ
) const
{
    if (worldY < 0 || worldY >= Chunk::SIZE)
    {
        return 0;
    }

    int chunkX = floorDiv(worldX, Chunk::SIZE);
    int chunkZ = floorDiv(worldZ, Chunk::SIZE);

    int localX = positiveMod(worldX, Chunk::SIZE);
    int localZ = positiveMod(worldZ, Chunk::SIZE);

    const ChunkRenderData* data =
        findChunk(chunkX, chunkZ);

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
    if (worldY < 0 || worldY >= Chunk::SIZE)
    {
        return;
    }

    int chunkX = floorDiv(worldX, Chunk::SIZE);
    int chunkZ = floorDiv(worldZ, Chunk::SIZE);

    int localX = positiveMod(worldX, Chunk::SIZE);
    int localZ = positiveMod(worldZ, Chunk::SIZE);

    ChunkRenderData* data =
        findChunk(chunkX, chunkZ);

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

    if (localX == 0)
    {
        if (ChunkRenderData* neighbor =
            findChunk(chunkX - 1, chunkZ))
        {
            neighbor->chunk.markMeshDirty();
        }
    }

    if (localX == Chunk::SIZE - 1)
    {
        if (ChunkRenderData* neighbor =
            findChunk(chunkX + 1, chunkZ))
        {
            neighbor->chunk.markMeshDirty();
        }
    }

    if (localZ == 0)
    {
        if (ChunkRenderData* neighbor =
            findChunk(chunkX, chunkZ - 1))
        {
            neighbor->chunk.markMeshDirty();
        }
    }

    if (localZ == Chunk::SIZE - 1)
    {
        if (ChunkRenderData* neighbor =
            findChunk(chunkX, chunkZ + 1))
        {
            neighbor->chunk.markMeshDirty();
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
        static_cast<int>(std::floor(origin.x)),
        static_cast<int>(std::floor(origin.y)),
        static_cast<int>(std::floor(origin.z))
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
        (nextBoundary.x - origin.x) / rayDirection.x :
        infinity;

    tMax.y =
        rayDirection.y != 0.0f ?
        (nextBoundary.y - origin.y) / rayDirection.y :
        infinity;

    tMax.z =
        rayDirection.z != 0.0f ?
        (nextBoundary.z - origin.z) / rayDirection.z :
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
        uint16_t blockId = getBlock(
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

        previousBlockPosition = blockPosition;

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

void World::draw(const Frustum& frustum) const
{
    for (const auto& [coord, data] : m_chunks)
    {
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
    for (const auto& [coord, data] : m_chunks)
    {
        debugRenderer.drawChunkBorder(
            data.minBounds,
            data.maxBounds,
            projection,
            view
        );
    }
}

int World::floorDiv(int value, int divisor)
{
    int result = value / divisor;

    int remainder = value % divisor;

    if (remainder != 0 &&
        ((remainder < 0) != (divisor < 0)))
    {
        result--;
    }

    return result;
}

int World::positiveMod(int value, int divisor)
{
    int result = value % divisor;

    if (result < 0)
    {
        result += divisor;
    }

    return result;
}

World::ChunkRenderData* World::findChunk(
    int chunkX,
    int chunkZ
)
{
    auto it =
        m_chunks.find(ChunkCoord{ chunkX, chunkZ });

    if (it == m_chunks.end())
    {
        return nullptr;
    }

    return &it->second;
}

const World::ChunkRenderData* World::findChunk(
    int chunkX,
    int chunkZ
) const
{
    auto it =
        m_chunks.find(ChunkCoord{ chunkX, chunkZ });

    if (it == m_chunks.end())
    {
        return nullptr;
    }

    return &it->second;
}

void World::rebuildChunkMesh(ChunkRenderData& data)
{
    glm::vec3 worldPosition(
        static_cast<float>(data.chunkX * Chunk::SIZE),
        0.0f,
        static_cast<float>(data.chunkZ * Chunk::SIZE)
    );

    Chunk::BlockLookupFunction blockLookup =
        [this](int worldX, int worldY, int worldZ) -> uint16_t
        {
            return getBlock(worldX, worldY, worldZ);
        };

    std::vector<float> vertices =
        data.chunk.buildMesh(
            m_renderInfo,
            worldPosition,
            blockLookup
        );

    std::cout
        << "Rebuilt chunk ("
        << data.chunkX
        << ", "
        << data.chunkZ
        << ") vertices: "
        << vertices.size() / Mesh::FLOATS_PER_VERTEX
        << std::endl;

    data.mesh =
        std::make_unique<Mesh>(vertices);
}

std::string World::getWorldSavePath() const
{
    return "saves/" + m_worldName;
}

std::string World::getChunkSavePath(
    int chunkX,
    int chunkZ
) const
{
    std::stringstream stream;

    stream
        << getWorldSavePath()
        << "/chunks/chunk_"
        << chunkX
        << "_"
        << chunkZ
        << ".json";

    return stream.str();
}

void World::saveWorld()
{
    namespace fs = std::filesystem;

    fs::create_directories(
        getWorldSavePath() + "/chunks"
    );

    for (auto& [coord, data] : m_chunks)
    {
        if (!data.chunk.isSaveDirty())
        {
            continue;
        }

        std::string chunkPath =
            getChunkSavePath(
                data.chunkX,
                data.chunkZ
            );

        if (data.chunk.saveToFile(
            chunkPath,
            data.chunkX,
            data.chunkZ))
        {
            data.chunk.clearSaveDirty();
        }
    }

    std::cout
        << "World saved."
        << std::endl;
}

bool World::worldMetadataExists() const
{
    namespace fs = std::filesystem;

    return fs::exists(
        getWorldSavePath() + "/world.json"
    );
}

void World::createWorldMetadataFile() const
{
    nlohmann::json data;

    data["name"] = m_worldName;
    data["version"] = m_worldVersion;
    data["seed"] = m_worldSeed;
    data["chunk_size"] = Chunk::SIZE;

    std::ofstream file(
        getWorldSavePath() + "/world.json"
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to create world metadata file."
            << std::endl;

        return;
    }

    file << data.dump(1);

    std::cout
        << "Created world metadata file."
        << std::endl;
}