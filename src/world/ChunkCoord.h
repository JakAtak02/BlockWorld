#pragma once

#include <cstddef>
#include <functional>

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