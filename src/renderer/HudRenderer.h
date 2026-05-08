#pragma once

#include "renderer/Texture2D.h"
#include "world/Chunk.h"

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

class HudRenderer
{
public:
    HudRenderer();
    ~HudRenderer();

    bool loadHotbarTexture(const char* path);

    bool loadHotbarTextureFromJson(const char* path);

    bool loadHotbarSelectionTextureFromJson(const char* path);

    void draw(
        int screenWidth,
        int screenHeight,
        const Texture2D& blockTextureArray,
        const std::vector<BlockRenderInfo>& renderInfo,
        uint16_t selectedBlockId
    );

private:
    unsigned int compileShader(
        unsigned int type,
        const char* source
    );

    unsigned int createShaderProgram(
        const char* vertexSource,
        const char* fragmentSource
    );

    bool loadTexture2DFromJson(
        const char* jsonPath,
        unsigned int& textureId,
        int& width,
        int& height
    );

    bool loadTexture2D(
        const char* texturePath,
        unsigned int& textureId,
        int& width,
        int& height
    );

    void drawTextureQuad(
        unsigned int textureId,
        float x,
        float y,
        float width,
        float height
    );

    void drawHotbarBackground(
        float x,
        float y,
        float width,
        float height
    );

    void drawSelectedSlotOutline(
        float x,
        float y,
        float width,
        float height
    );

    void drawBlockIcon3D(
        float centerX,
        float centerY,
        uint16_t blockId,
        const Texture2D& blockTextureArray,
        const std::vector<BlockRenderInfo>& renderInfo
    );

    void uploadAndDrawIconFace(
        const float* vertices,
        int vertexCount,
        float brightness
    );

private:
    unsigned int m_hotbarTexture = 0;
    int m_hotbarWidth = 0;
    int m_hotbarHeight = 0;

    unsigned int m_hotbarSelectionTexture = 0;
    int m_hotbarSelectionWidth = 0;
    int m_hotbarSelectionHeight = 0;

    unsigned int m_quadVao = 0;
    unsigned int m_quadVbo = 0;

    unsigned int m_iconVao = 0;
    unsigned int m_iconVbo = 0;

    unsigned int m_lineVao = 0;
    unsigned int m_lineVbo = 0;

    unsigned int m_hotbarShader = 0;
    unsigned int m_iconShader = 0;
    unsigned int m_lineShader = 0;

    glm::mat4 m_currentIconMvp{ 1.0f };

    int m_screenWidth = 1280;
    int m_screenHeight = 720;
};