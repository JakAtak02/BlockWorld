#include "world/generation/TerrainGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
    constexpr int BEDROCK_Y = 0;

    constexpr int BASE_SURFACE_HEIGHT = 384;

    constexpr int MIN_SURFACE_HEIGHT = 220;
    constexpr int MAX_SURFACE_HEIGHT = 700;

    constexpr int DIRT_DEPTH = 5;

    constexpr uint16_t BLOCK_AIR = 0;
    constexpr uint16_t BLOCK_STONE = 1;
    constexpr uint16_t BLOCK_DIRT = 2;
    constexpr uint16_t BLOCK_GRASS = 4;
    constexpr uint16_t BLOCK_BEDROCK = 9;

    float waveNoise(
        int worldX,
        int worldZ,
        float frequency,
        float seedOffset
    )
    {
        return
            std::sin((worldX + seedOffset) * frequency) *
            std::cos((worldZ - seedOffset) * frequency);
    }

    float layeredNoise(
        int worldX,
        int worldZ
    )
    {
        float large =
            waveNoise(worldX, worldZ, 0.0018f, 17.0f) * 260.0f;

        float medium =
            waveNoise(worldX, worldZ, 0.0065f, 91.0f) * 95.0f;

        float small =
            waveNoise(worldX, worldZ, 0.024f, 241.0f) * 24.0f;

        return large + medium + small;
    }

    float mountainMask(
        int worldX,
        int worldZ
    )
    {
        float mask =
            waveNoise(worldX, worldZ, 0.0012f, 431.0f);

        mask =
            (mask + 1.0f) * 0.5f;

        mask =
            std::clamp(mask, 0.0f, 1.0f);

        return mask * mask * mask;
    }

    float valleyMask(
        int worldX,
        int worldZ
    )
    {
        float mask =
            waveNoise(worldX, worldZ, 0.0025f, 812.0f);

        mask =
            (mask + 1.0f) * 0.5f;

        mask =
            std::clamp(mask, 0.0f, 1.0f);

        return mask * mask;
    }

    int internalCalculateTerrainHeight(
        int worldX,
        int worldZ
    )
    {
        float height =
            static_cast<float>(BASE_SURFACE_HEIGHT);

        height += layeredNoise(
            worldX,
            worldZ
        );

        float mountains =
            mountainMask(
                worldX,
                worldZ
            );

        height += mountains * 180.0f;

        float valleys =
            valleyMask(
                worldX,
                worldZ
            );

        height -= valleys * 80.0f;

        return std::clamp(
            static_cast<int>(height),
            MIN_SURFACE_HEIGHT,
            MAX_SURFACE_HEIGHT
        );
    }
}

void TerrainGenerator::generateChunk(
    Chunk& chunk,
    int chunkX,
    int chunkY,
    int chunkZ
) const
{
    int chunkBaseY =
        chunkY * Chunk::SIZE;

    for (int z = 0; z < Chunk::SIZE; z++)
    {
        for (int x = 0; x < Chunk::SIZE; x++)
        {
            int worldX =
                chunkX * Chunk::SIZE + x;

            int worldZ =
                chunkZ * Chunk::SIZE + z;

            int terrainHeight =
                internalCalculateTerrainHeight(
                    worldX,
                    worldZ
                );

            for (int localY = 0;
                localY < Chunk::SIZE;
                localY++)
            {
                int worldY =
                    chunkBaseY + localY;

                uint16_t block =
                    BLOCK_AIR;

                if (worldY == BEDROCK_Y)
                {
                    block = BLOCK_BEDROCK;
                }
                else if (worldY < terrainHeight - DIRT_DEPTH)
                {
                    block = BLOCK_STONE;
                }
                else if (worldY < terrainHeight - 1)
                {
                    block = BLOCK_DIRT;
                }
                else if (worldY == terrainHeight - 1)
                {
                    block = BLOCK_GRASS;
                }

                if (block != BLOCK_AIR)
                {
                    chunk.setGeneratedBlock(
                        x,
                        localY,
                        z,
                        block
                    );
                }
            }
        }
    }
}

int TerrainGenerator::getTerrainHeight(
    int worldX,
    int worldZ
) const
{
    return internalCalculateTerrainHeight(
        worldX,
        worldZ
    );
}

int TerrainGenerator::getHighestTerrainChunkY(
    int chunkX,
    int chunkZ
) const
{
    int maxHeight = 0;

    int baseWorldX =
        chunkX * Chunk::SIZE;

    int baseWorldZ =
        chunkZ * Chunk::SIZE;

    for (int z = 0; z < Chunk::SIZE; z++)
    {
        for (int x = 0; x < Chunk::SIZE; x++)
        {
            int terrainHeight =
                internalCalculateTerrainHeight(
                    baseWorldX + x,
                    baseWorldZ + z
                );

            if (terrainHeight > maxHeight)
            {
                maxHeight = terrainHeight;
            }
        }
    }

    return maxHeight / Chunk::SIZE;
}