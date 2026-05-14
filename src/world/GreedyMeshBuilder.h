#pragma once

#include "world/Chunk.h"

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

class GreedyMeshBuilder
{
public:
    static void addGreedyFaces(
        const Chunk& chunk,
        std::vector<float>& vertices,
        const std::vector<BlockRenderInfo>& renderInfo,
        const glm::vec3& worldPosition,
        const Chunk::BlockLookupFunction& blockLookup
    );

private:
    enum class Face
    {
        Back = 0,
        Front = 1,
        Left = 2,
        Right = 3,
        Bottom = 4,
        Top = 5
    };

    struct Cell
    {
        bool visible = false;
        uint16_t blockId = 0;
        float textureIndex = 0.0f;

        glm::vec3 tintColor =
        {
            1.0f,
            1.0f,
            1.0f
        };
    };

private:
    static void addGreedyFaceDirection(
        const Chunk& chunk,
        std::vector<float>& vertices,
        const std::vector<BlockRenderInfo>& renderInfo,
        const glm::vec3& worldPosition,
        const Chunk::BlockLookupFunction& blockLookup,
        Face face
    );

    static Cell getCell(
        const Chunk& chunk,
        const std::vector<BlockRenderInfo>& renderInfo,
        const glm::vec3& worldPosition,
        const Chunk::BlockLookupFunction& blockLookup,
        Face face,
        int slice,
        int a,
        int b
    );

    static float getTextureIndexForFace(
        uint16_t blockId,
        Face face,
        const std::vector<BlockRenderInfo>& renderInfo
    );

    static void addQuad(
        std::vector<float>& vertices,
        Face face,
        int slice,
        int a,
        int b,
        int width,
        int height,
        float textureIndex,
        const glm::vec3& tintColor,
        const glm::vec3& worldPosition
    );
};