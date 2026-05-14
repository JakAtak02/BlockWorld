#pragma once

#include <cstddef>
#include <functional>

struct ChunkCoord
{
    int x = 0;
    int y = 0;
    int z = 0;

    bool operator==(const ChunkCoord& other) const
    {
        return
            x == other.x &&
            y == other.y &&
            z == other.z;
    }
};

struct ChunkCoordHash
{
    std::size_t operator()(const ChunkCoord& coord) const
    {
        std::size_t h1 = std::hash<int>{}(coord.x);
        std::size_t h2 = std::hash<int>{}(coord.y);
        std::size_t h3 = std::hash<int>{}(coord.z);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};