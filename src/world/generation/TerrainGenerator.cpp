#include "world/generation/TerrainGenerator.h"

#include <cmath>

void TerrainGenerator::generateChunk(
    Chunk& chunk,
    int chunkX,
    int chunkZ
) const
{
    for (int z = 0; z < Chunk::SIZE; z++)
    {
        for (int x = 0; x < Chunk::SIZE; x++)
        {
            int worldX = chunkX * Chunk::SIZE + x;
            int worldZ = chunkZ * Chunk::SIZE + z;

            float heightNoise =
                std::sin(worldX * 0.05f) * 4.0f +
                std::cos(worldZ * 0.05f) * 4.0f;

            int terrainHeight =
                static_cast<int>(heightNoise + 12.0f);

            for (int y = 0; y < Chunk::SIZE; y++)
            {
                uint16_t block = 0;

                if (y < terrainHeight - 4)
                {
                    block = 1;
                }
                else if (y < terrainHeight - 1)
                {
                    block = 2;
                }
                else if (y == terrainHeight - 1)
                {
                    block = 3;
                }

                if (block != 0)
                {
                    chunk.setBlock(
                        x,
                        y,
                        z,
                        block
                    );
                }
            }
        }
    }
}