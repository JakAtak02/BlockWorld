#include "renderer/HudRenderer.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>

unsigned int HudRenderer::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);

        std::cout
            << "HUD shader compile error:\n"
            << infoLog
            << std::endl;
    }

    return shader;
}

unsigned int HudRenderer::createShaderProgram(
    const char* vertexSource,
    const char* fragmentSource
)
{
    unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexSource);

    unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);

        std::cout
            << "HUD shader link error:\n"
            << infoLog
            << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

HudRenderer::HudRenderer()
{
    const char* hotbarVertexSource = R"(
        #version 460 core

        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;

        uniform vec2 uScreenSize;

        out vec2 vTexCoord;

        void main()
        {
            vec2 clip;

            clip.x = (aPos.x / uScreenSize.x) * 2.0 - 1.0;
            clip.y = 1.0 - (aPos.y / uScreenSize.y) * 2.0;

            vTexCoord = aTexCoord;

            gl_Position = vec4(clip, 0.0, 1.0);
        }
    )";

    const char* hotbarFragmentSource = R"(
        #version 460 core

        in vec2 vTexCoord;

        uniform sampler2D uTexture;

        out vec4 FragColor;

        void main()
        {
            FragColor = texture(uTexture, vTexCoord);
        }
    )";

    const char* iconVertexSource = R"(
        #version 460 core

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in float aTextureLayer;

        uniform mat4 uMvp;

        out vec2 vTexCoord;
        flat out float vTextureLayer;

        void main()
        {
            vTexCoord = aTexCoord;
            vTextureLayer = aTextureLayer;

            gl_Position = uMvp * vec4(aPos, 1.0);
        }
    )";

    const char* iconFragmentSource = R"(
        #version 460 core

        in vec2 vTexCoord;
        flat in float vTextureLayer;

        uniform sampler2DArray uTextureArray;
        uniform float uBrightness;

        out vec4 FragColor;

        void main()
        {
            vec4 color = texture(
                uTextureArray,
                vec3(vTexCoord, vTextureLayer)
            );

            if (color.a < 0.1)
            {
                discard;
            }

            FragColor = vec4(color.rgb * uBrightness, color.a);
        }
    )";

    const char* lineVertexSource = R"(
        #version 460 core

        layout (location = 0) in vec2 aPos;

        uniform vec2 uScreenSize;

        void main()
        {
            vec2 clip;

            clip.x = (aPos.x / uScreenSize.x) * 2.0 - 1.0;
            clip.y = 1.0 - (aPos.y / uScreenSize.y) * 2.0;

            gl_Position = vec4(clip, 0.0, 1.0);
        }
    )";

    const char* lineFragmentSource = R"(
        #version 460 core

        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )";

    m_hotbarShader =
        createShaderProgram(hotbarVertexSource, hotbarFragmentSource);

    m_iconShader =
        createShaderProgram(iconVertexSource, iconFragmentSource);

    m_lineShader =
        createShaderProgram(lineVertexSource, lineFragmentSource);

    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);

    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 6 * 4,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &m_iconVao);
    glGenBuffers(1, &m_iconVbo);

    glBindVertexArray(m_iconVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_iconVbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 6 * 6,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(5 * sizeof(float))
    );

    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &m_lineVao);
    glGenBuffers(1, &m_lineVbo);

    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 2 * 8,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

HudRenderer::~HudRenderer()
{
    if (m_hotbarTexture != 0)
        glDeleteTextures(1, &m_hotbarTexture);

    if (m_hotbarSelectionTexture != 0)
        glDeleteTextures(1, &m_hotbarSelectionTexture);

    if (m_quadVbo != 0)
        glDeleteBuffers(1, &m_quadVbo);

    if (m_quadVao != 0)
        glDeleteVertexArrays(1, &m_quadVao);

    if (m_iconVbo != 0)
        glDeleteBuffers(1, &m_iconVbo);

    if (m_iconVao != 0)
        glDeleteVertexArrays(1, &m_iconVao);

    if (m_lineVbo != 0)
        glDeleteBuffers(1, &m_lineVbo);

    if (m_lineVao != 0)
        glDeleteVertexArrays(1, &m_lineVao);

    if (m_hotbarShader != 0)
        glDeleteProgram(m_hotbarShader);

    if (m_iconShader != 0)
        glDeleteProgram(m_iconShader);

    if (m_lineShader != 0)
        glDeleteProgram(m_lineShader);
}

bool HudRenderer::loadTexture2DFromJson(
    const char* jsonPath,
    unsigned int& textureId,
    int& width,
    int& height
)
{
    std::ifstream file(jsonPath);

    if (!file.is_open())
    {
        std::cout
            << "Failed to open HUD texture JSON: "
            << jsonPath
            << std::endl;

        return false;
    }

    nlohmann::json data;
    file >> data;

    std::string texturePath =
        data.value("texture", "");

    if (texturePath.empty())
    {
        std::cout
            << "HUD texture JSON missing texture path: "
            << jsonPath
            << std::endl;

        return false;
    }

    std::string fullPath =
        "resources/" + texturePath;

    return loadTexture2D(
        fullPath.c_str(),
        textureId,
        width,
        height
    );
}

bool HudRenderer::loadTexture2D(
    const char* texturePath,
    unsigned int& textureId,
    int& width,
    int& height
)
{
    int channels = 0;

    stbi_set_flip_vertically_on_load(false);

    unsigned char* data =
        stbi_load(texturePath, &width, &height, &channels, 4);

    if (!data)
    {
        std::cout
            << "Failed to load HUD texture: "
            << texturePath
            << std::endl;

        return false;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::cout
        << "Loaded HUD texture: "
        << texturePath
        << std::endl;

    return true;
}

bool HudRenderer::loadHotbarTextureFromJson(const char* path)
{
    return loadTexture2DFromJson(
        path,
        m_hotbarTexture,
        m_hotbarWidth,
        m_hotbarHeight
    );
}

bool HudRenderer::loadHotbarSelectionTextureFromJson(const char* path)
{
    return loadTexture2DFromJson(
        path,
        m_hotbarSelectionTexture,
        m_hotbarSelectionWidth,
        m_hotbarSelectionHeight
    );
}

bool HudRenderer::loadHotbarTexture(const char* path)
{
    return loadTexture2D(
        path,
        m_hotbarTexture,
        m_hotbarWidth,
        m_hotbarHeight
    );
}

void HudRenderer::draw(
    int screenWidth,
    int screenHeight,
    const Texture2D& blockTextureArray,
    const std::vector<BlockRenderInfo>& renderInfo,
    uint16_t selectedBlockId
)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float hotbarWidth =
        static_cast<float>(m_hotbarWidth);

    float hotbarHeight =
        static_cast<float>(m_hotbarHeight);

    float hotbarX =
        (static_cast<float>(screenWidth) - hotbarWidth) * 0.5f;

    float hotbarY =
        static_cast<float>(screenHeight) - hotbarHeight - 20.0f;

    glDisable(GL_DEPTH_TEST);

    drawHotbarBackground(
        hotbarX,
        hotbarY,
        hotbarWidth,
        hotbarHeight
    );

    float slotWidth =
        hotbarWidth / 9.0f;

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    for (uint16_t blockId = 1; blockId <= 9; blockId++)
    {
        float centerX =
            hotbarX +
            slotWidth * (static_cast<float>(blockId) - 0.5f) -
            4.0f;

        float centerY =
            hotbarY + hotbarHeight * 0.50f;

        drawBlockIcon3D(
            centerX,
            centerY,
            blockId,
            blockTextureArray,
            renderInfo
        );
    }

    glDisable(GL_DEPTH_TEST);

    if (selectedBlockId >= 1 && selectedBlockId <= 9)
    {
        float slotX =
            hotbarX +
            slotWidth * static_cast<float>(selectedBlockId - 1);

        drawTextureQuad(
            m_hotbarSelectionTexture,
            slotX,
            hotbarY,
            static_cast<float>(m_hotbarSelectionWidth),
            static_cast<float>(m_hotbarSelectionHeight)
        );
    }

    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HudRenderer::drawTextureQuad(
    unsigned int textureId,
    float x,
    float y,
    float width,
    float height
)
{
    if (textureId == 0)
    {
        return;
    }

    float vertices[] =
    {
        x,         y,          0.0f, 0.0f,
        x + width, y,          1.0f, 0.0f,
        x + width, y + height, 1.0f, 1.0f,

        x + width, y + height, 1.0f, 1.0f,
        x,         y + height, 0.0f, 1.0f,
        x,         y,          0.0f, 0.0f
    };

    glUseProgram(m_hotbarShader);

    int screenLoc =
        glGetUniformLocation(m_hotbarShader, "uScreenSize");

    int textureLoc =
        glGetUniformLocation(m_hotbarShader, "uTexture");

    glUniform2f(
        screenLoc,
        static_cast<float>(m_screenWidth),
        static_cast<float>(m_screenHeight)
    );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(textureLoc, 1);

    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HudRenderer::drawHotbarBackground(
    float x,
    float y,
    float width,
    float height
)
{
    drawTextureQuad(
        m_hotbarTexture,
        x,
        y,
        width,
        height
    );
}

void HudRenderer::drawBlockIcon3D(
    float centerX,
    float centerY,
    uint16_t blockId,
    const Texture2D& blockTextureArray,
    const std::vector<BlockRenderInfo>& renderInfo
)
{
    if (blockId == 0 || blockId > renderInfo.size())
    {
        return;
    }

    const BlockRenderInfo& info =
        renderInfo[blockId - 1];

    float iconSize =
        info.guiScale;

    glm::mat4 projection =
        glm::ortho(
            0.0f,
            static_cast<float>(m_screenWidth),
            static_cast<float>(m_screenHeight),
            0.0f,
            -100.0f,
            100.0f
        );

    glm::mat4 model(1.0f);

    model =
        glm::translate(
            model,
            glm::vec3(centerX, centerY, 0.0f)
        );

    model =
        glm::rotate(
            model,
            glm::radians(info.guiRotation.x),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

    model =
        glm::rotate(
            model,
            glm::radians(info.guiRotation.y),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

    model =
        glm::rotate(
            model,
            glm::radians(info.guiRotation.z),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

    model =
        glm::scale(
            model,
            glm::vec3(iconSize)
        );

    m_currentIconMvp =
        projection * model;

    blockTextureArray.bind(0);

    float topLayer =
        info.topTextureIndex;

    float sideLayer =
        info.sideTextureIndex;

    float topFace[] =
    {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, topLayer,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, topLayer,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, topLayer,

         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, topLayer,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, topLayer,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, topLayer
    };

    float frontFace[] =
    {
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, sideLayer,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, sideLayer,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, sideLayer,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, sideLayer,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, sideLayer,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, sideLayer
    };

    float rightFace[] =
    {
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, sideLayer,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, sideLayer,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, sideLayer,

         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, sideLayer,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, sideLayer,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, sideLayer
    };

    uploadAndDrawIconFace(frontFace, 6, 0.90f);
    uploadAndDrawIconFace(rightFace, 6, 0.70f);
    uploadAndDrawIconFace(topFace, 6, 1.15f);
}

void HudRenderer::uploadAndDrawIconFace(
    const float* vertices,
    int vertexCount,
    float brightness
)
{
    glUseProgram(m_iconShader);

    int mvpLoc =
        glGetUniformLocation(m_iconShader, "uMvp");

    int textureLoc =
        glGetUniformLocation(m_iconShader, "uTextureArray");

    int brightnessLoc =
        glGetUniformLocation(m_iconShader, "uBrightness");

    glUniformMatrix4fv(
        mvpLoc,
        1,
        GL_FALSE,
        &m_currentIconMvp[0][0]
    );

    glUniform1i(textureLoc, 0);
    glUniform1f(brightnessLoc, brightness);

    glBindVertexArray(m_iconVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_iconVbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(float) * vertexCount * 6,
        vertices
    );

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}