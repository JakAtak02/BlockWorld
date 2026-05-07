#include "world/GreedyMeshBuilder.h"

void GreedyMeshBuilder::addGreedyFaces(
    const Chunk& chunk,
    std::vector<float>& vertices,
    const std::array<BlockRenderInfo, 4>& renderInfo,
    const glm::vec3& worldPosition,
    const Chunk::BlockLookupFunction& blockLookup
)
{
    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Back
    );

    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Front
    );

    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Left
    );

    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Right
    );

    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Bottom
    );

    addGreedyFaceDirection(
        chunk,
        vertices,
        renderInfo,
        worldPosition,
        blockLookup,
        Face::Top
    );
}

void GreedyMeshBuilder::addGreedyFaceDirection(
    const Chunk& chunk,
    std::vector<float>& vertices,
    const std::array<BlockRenderInfo, 4>& renderInfo,
    const glm::vec3& worldPosition,
    const Chunk::BlockLookupFunction& blockLookup,
    Face face
)
{
    for (int slice = 0; slice < Chunk::SIZE; slice++)
    {
        bool used[Chunk::SIZE][Chunk::SIZE] = {};

        for (int b = 0; b < Chunk::SIZE; b++)
        {
            for (int a = 0; a < Chunk::SIZE; a++)
            {
                if (used[a][b])
                {
                    continue;
                }

                Cell cell = getCell(
                    chunk,
                    renderInfo,
                    worldPosition,
                    blockLookup,
                    face,
                    slice,
                    a,
                    b
                );

                if (!cell.visible)
                {
                    continue;
                }

                int width = 1;

                while (a + width < Chunk::SIZE)
                {
                    if (used[a + width][b])
                    {
                        break;
                    }

                    Cell next = getCell(
                        chunk,
                        renderInfo,
                        worldPosition,
                        blockLookup,
                        face,
                        slice,
                        a + width,
                        b
                    );

                    if (!next.visible ||
                        next.blockId != cell.blockId ||
                        next.textureIndex != cell.textureIndex)
                    {
                        break;
                    }

                    width++;
                }

                int height = 1;

                bool canGrowHeight = true;

                while (b + height < Chunk::SIZE && canGrowHeight)
                {
                    for (int da = 0; da < width; da++)
                    {
                        if (used[a + da][b + height])
                        {
                            canGrowHeight = false;
                            break;
                        }

                        Cell next = getCell(
                            chunk,
                            renderInfo,
                            worldPosition,
                            blockLookup,
                            face,
                            slice,
                            a + da,
                            b + height
                        );

                        if (!next.visible ||
                            next.blockId != cell.blockId ||
                            next.textureIndex != cell.textureIndex)
                        {
                            canGrowHeight = false;
                            break;
                        }
                    }

                    if (canGrowHeight)
                    {
                        height++;
                    }
                }

                for (int db = 0; db < height; db++)
                {
                    for (int da = 0; da < width; da++)
                    {
                        used[a + da][b + db] = true;
                    }
                }

                addQuad(
                    vertices,
                    face,
                    slice,
                    a,
                    b,
                    width,
                    height,
                    cell.textureIndex,
                    worldPosition
                );
            }
        }
    }
}

GreedyMeshBuilder::Cell GreedyMeshBuilder::getCell(
    const Chunk& chunk,
    const std::array<BlockRenderInfo, 4>& renderInfo,
    const glm::vec3& worldPosition,
    const Chunk::BlockLookupFunction& blockLookup,
    Face face,
    int slice,
    int a,
    int b
)
{
    int x = 0;
    int y = 0;
    int z = 0;

    int neighborX = 0;
    int neighborY = 0;
    int neighborZ = 0;

    switch (face)
    {
    case Face::Back:
        x = a;
        y = b;
        z = slice;

        neighborX = x;
        neighborY = y;
        neighborZ = z - 1;
        break;

    case Face::Front:
        x = a;
        y = b;
        z = slice;

        neighborX = x;
        neighborY = y;
        neighborZ = z + 1;
        break;

    case Face::Left:
        x = slice;
        y = b;
        z = a;

        neighborX = x - 1;
        neighborY = y;
        neighborZ = z;
        break;

    case Face::Right:
        x = slice;
        y = b;
        z = a;

        neighborX = x + 1;
        neighborY = y;
        neighborZ = z;
        break;

    case Face::Bottom:
        x = a;
        y = slice;
        z = b;

        neighborX = x;
        neighborY = y - 1;
        neighborZ = z;
        break;

    case Face::Top:
        x = a;
        y = slice;
        z = b;

        neighborX = x;
        neighborY = y + 1;
        neighborZ = z;
        break;
    }

    uint16_t blockId = chunk.getBlock(x, y, z);

    if (blockId == 0)
    {
        return {};
    }

    int worldNeighborX =
        static_cast<int>(worldPosition.x) + neighborX;

    int worldNeighborY =
        static_cast<int>(worldPosition.y) + neighborY;

    int worldNeighborZ =
        static_cast<int>(worldPosition.z) + neighborZ;

    if (blockLookup(
        worldNeighborX,
        worldNeighborY,
        worldNeighborZ
    ) != 0)
    {
        return {};
    }

    Cell cell;
    cell.visible = true;
    cell.blockId = blockId;

    cell.textureIndex =
        getTextureIndexForFace(
            blockId,
            face,
            renderInfo
        );

    return cell;
}

float GreedyMeshBuilder::getTextureIndexForFace(
    uint16_t blockId,
    Face face,
    const std::array<BlockRenderInfo, 4>& renderInfo
)
{
    if (blockId == 0 || blockId > renderInfo.size())
    {
        return 0.0f;
    }

    const BlockRenderInfo& info =
        renderInfo[blockId - 1];

    if (face == Face::Bottom)
    {
        return info.bottomTextureIndex;
    }

    if (face == Face::Top)
    {
        return info.topTextureIndex;
    }

    return info.sideTextureIndex;
}

void GreedyMeshBuilder::addQuad(
    std::vector<float>& vertices,
    Face face,
    int slice,
    int a,
    int b,
    int width,
    int height,
    float textureIndex,
    const glm::vec3& worldPosition
)
{
    auto push =
        [&vertices, textureIndex](
            float px,
            float py,
            float pz,
            float u,
            float v
            )
        {
            vertices.push_back(px);
            vertices.push_back(py);
            vertices.push_back(pz);

            vertices.push_back(u);
            vertices.push_back(v);

            vertices.push_back(textureIndex);
        };

    float w = static_cast<float>(width);
    float h = static_cast<float>(height);

    switch (face)
    {
    case Face::Back:
    {
        float x0 = worldPosition.x + static_cast<float>(a);
        float x1 = x0 + w;

        float y0 = worldPosition.y + static_cast<float>(b);
        float y1 = y0 + h;

        float z0 = worldPosition.z + static_cast<float>(slice);

        push(x0, y0, z0, 0.0f, 0.0f);
        push(x1, y0, z0, w, 0.0f);
        push(x1, y1, z0, w, h);

        push(x1, y1, z0, w, h);
        push(x0, y1, z0, 0.0f, h);
        push(x0, y0, z0, 0.0f, 0.0f);

        break;
    }

    case Face::Front:
    {
        float x0 = worldPosition.x + static_cast<float>(a);
        float x1 = x0 + w;

        float y0 = worldPosition.y + static_cast<float>(b);
        float y1 = y0 + h;

        float z1 =
            worldPosition.z +
            static_cast<float>(slice) +
            1.0f;

        push(x0, y0, z1, 0.0f, 0.0f);
        push(x1, y0, z1, w, 0.0f);
        push(x1, y1, z1, w, h);

        push(x1, y1, z1, w, h);
        push(x0, y1, z1, 0.0f, h);
        push(x0, y0, z1, 0.0f, 0.0f);

        break;
    }

    case Face::Left:
    {
        float x0 =
            worldPosition.x +
            static_cast<float>(slice);

        float y0 =
            worldPosition.y +
            static_cast<float>(b);

        float y1 = y0 + h;

        float z0 =
            worldPosition.z +
            static_cast<float>(a);

        float z1 = z0 + w;

        push(x0, y1, z1, w, h);
        push(x0, y1, z0, 0.0f, h);
        push(x0, y0, z0, 0.0f, 0.0f);

        push(x0, y0, z0, 0.0f, 0.0f);
        push(x0, y0, z1, w, 0.0f);
        push(x0, y1, z1, w, h);

        break;
    }

    case Face::Right:
    {
        float x1 =
            worldPosition.x +
            static_cast<float>(slice) +
            1.0f;

        float y0 =
            worldPosition.y +
            static_cast<float>(b);

        float y1 = y0 + h;

        float z0 =
            worldPosition.z +
            static_cast<float>(a);

        float z1 = z0 + w;

        push(x1, y1, z1, 0.0f, h);
        push(x1, y1, z0, w, h);
        push(x1, y0, z0, w, 0.0f);

        push(x1, y0, z0, w, 0.0f);
        push(x1, y0, z1, 0.0f, 0.0f);
        push(x1, y1, z1, 0.0f, h);

        break;
    }

    case Face::Bottom:
    {
        float x0 =
            worldPosition.x +
            static_cast<float>(a);

        float x1 = x0 + w;

        float y0 =
            worldPosition.y +
            static_cast<float>(slice);

        float z0 =
            worldPosition.z +
            static_cast<float>(b);

        float z1 = z0 + h;

        push(x0, y0, z0, 0.0f, h);
        push(x1, y0, z0, w, h);
        push(x1, y0, z1, w, 0.0f);

        push(x1, y0, z1, w, 0.0f);
        push(x0, y0, z1, 0.0f, 0.0f);
        push(x0, y0, z0, 0.0f, h);

        break;
    }

    case Face::Top:
    {
        float x0 =
            worldPosition.x +
            static_cast<float>(a);

        float x1 = x0 + w;

        float y1 =
            worldPosition.y +
            static_cast<float>(slice) +
            1.0f;

        float z0 =
            worldPosition.z +
            static_cast<float>(b);

        float z1 = z0 + h;

        push(x0, y1, z0, 0.0f, h);
        push(x1, y1, z0, w, h);
        push(x1, y1, z1, w, 0.0f);

        push(x1, y1, z1, w, 0.0f);
        push(x0, y1, z1, 0.0f, 0.0f);
        push(x0, y1, z0, 0.0f, h);

        break;
    }
    }
}