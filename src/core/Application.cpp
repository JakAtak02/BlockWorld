#include "core/Application.h"
#include "player/Player.h"

#include "renderer/BlockSelectionRenderer.h"
#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"
#include "renderer/Shader.h"
#include "renderer/Texture2D.h"

#include "world/BlockRegistry.h"
#include "world/World.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <string>

enum class DebugViewMode
{
    Off = 0,
    ChunkBorders = 1,
    Wireframe = 2,
    ChunkBordersAndWireframe = 3
};

static const char* debugViewModeToString(DebugViewMode mode)
{
    switch (mode)
    {
    case DebugViewMode::Off:
        return "OFF";
    case DebugViewMode::ChunkBorders:
        return "CHUNK BORDERS";
    case DebugViewMode::Wireframe:
        return "WIREFRAME";
    case DebugViewMode::ChunkBordersAndWireframe:
        return "CHUNK BORDERS + WIREFRAME";
    default:
        return "UNKNOWN";
    }
}

Application::Application()
{
    std::cout << "Starting Block World..." << std::endl;

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW." << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = new Window(1280, 720, "BlockWorld");

    if (!m_window->getNativeWindow())
    {
        std::cout << "Window creation failed." << std::endl;
        return;
    }

    glfwMakeContextCurrent(m_window->getNativeWindow());
    glfwSwapInterval(1);

    glfwSetInputMode(
        m_window->getNativeWindow(),
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD." << std::endl;
        return;
    }

    std::cout << "OpenGL Version: "
        << glGetString(GL_VERSION)
        << std::endl;
}

Application::~Application()
{
    delete m_window;
    glfwTerminate();
}

void Application::run()
{
    glEnable(GL_DEPTH_TEST);

    BlockRegistry registry;

    if (!registry.loadBlockFromJson("resources/data/textures/blocks/stone.json")) return;
    if (!registry.loadBlockFromJson("resources/data/textures/blocks/dirt.json")) return;
    if (!registry.loadBlockFromJson("resources/data/textures/blocks/sand.json")) return;
    if (!registry.loadBlockFromJson("resources/data/textures/blocks/grass_block.json")) return;

    const BlockType* stone = registry.getBlock("blockworld:stone");
    const BlockType* dirt = registry.getBlock("blockworld:dirt");
    const BlockType* sand = registry.getBlock("blockworld:sand");
    const BlockType* grass = registry.getBlock("blockworld:grass_block");

    if (!stone || !dirt || !sand || !grass)
    {
        std::cout << "Failed to load blocks." << std::endl;
        return;
    }

    Texture2D textures;

    if (!textures.loadArrayFromFiles({
        "resources/" + stone->sideTexturePath,
        "resources/" + dirt->sideTexturePath,
        "resources/" + sand->sideTexturePath,
        "resources/" + grass->topTexturePath,
        "resources/" + grass->sideTexturePath
        }))
    {
        return;
    }

    std::array<BlockRenderInfo, 4> renderInfo;

    renderInfo[0] = { 0.0f, 0.0f, 0.0f };
    renderInfo[1] = { 1.0f, 1.0f, 1.0f };
    renderInfo[2] = { 4.0f, 3.0f, 1.0f };
    renderInfo[3] = { 2.0f, 2.0f, 2.0f };

    World world(renderInfo);

    DebugRenderer debugRenderer;
    BlockSelectionRenderer blockSelectionRenderer;
    Frustum frustum;

    DebugViewMode debugMode = DebugViewMode::Off;
    bool f3WasPressed = false;

    const char* vs = R"(
        #version 460 core

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in float aTextureIndex;

        uniform mat4 uProjection;
        uniform mat4 uView;

        out vec2 vTexCoord;
        flat out float vTextureIndex;

        void main()
        {
            vTexCoord = aTexCoord;
            vTextureIndex = aTextureIndex;

            gl_Position =
                uProjection *
                uView *
                vec4(aPos, 1.0);
        }
    )";

    const char* fs = R"(
        #version 460 core

        in vec2 vTexCoord;
        flat in float vTextureIndex;

        uniform sampler2DArray uTextureArray;

        out vec4 FragColor;

        void main()
        {
            FragColor =
                texture(
                    uTextureArray,
                    vec3(vTexCoord, vTextureIndex)
                );
        }
    )";

    Shader shader(vs, fs);

    Player player;

    float lastFrameTime = 0.0f;

    bool firstMouse = true;
    double lastMouseX = 640.0;
    double lastMouseY = 360.0;

    float fpsTimer = 0.0f;
    int fpsCounter = 0;

    while (!m_window->shouldClose())
    {
        float currentFrameTime =
            static_cast<float>(glfwGetTime());

        float deltaTime =
            currentFrameTime - lastFrameTime;

        lastFrameTime = currentFrameTime;

        fpsTimer += deltaTime;
        fpsCounter++;

        GLFWwindow* nativeWindow =
            m_window->getNativeWindow();

        bool f3Pressed =
            glfwGetKey(nativeWindow, GLFW_KEY_F3) == GLFW_PRESS;

        if (f3Pressed && !f3WasPressed)
        {
            int nextMode =
                static_cast<int>(debugMode) + 1;

            if (nextMode >
                static_cast<int>(DebugViewMode::ChunkBordersAndWireframe))
            {
                nextMode =
                    static_cast<int>(DebugViewMode::Off);
            }

            debugMode =
                static_cast<DebugViewMode>(nextMode);

            std::cout
                << "Debug mode: "
                << debugViewModeToString(debugMode)
                << std::endl;
        }

        f3WasPressed = f3Pressed;

        if (fpsTimer >= 1.0f)
        {
            std::string title =
                "BlockWorld - FPS: " +
                std::to_string(fpsCounter);

            glfwSetWindowTitle(
                nativeWindow,
                title.c_str()
            );

            fpsCounter = 0;
            fpsTimer = 0.0f;
        }

        if (glfwGetKey(nativeWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(nativeWindow, true);
        }

        PlayerInputState playerInput;

        playerInput.moveForward =
            glfwGetKey(nativeWindow, GLFW_KEY_W) == GLFW_PRESS;

        playerInput.moveBackward =
            glfwGetKey(nativeWindow, GLFW_KEY_S) == GLFW_PRESS;

        playerInput.moveRight =
            glfwGetKey(nativeWindow, GLFW_KEY_D) == GLFW_PRESS;

        playerInput.moveLeft =
            glfwGetKey(nativeWindow, GLFW_KEY_A) == GLFW_PRESS;

        playerInput.moveUp =
            glfwGetKey(nativeWindow, GLFW_KEY_SPACE) == GLFW_PRESS;

        playerInput.moveDown =
            glfwGetKey(nativeWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        playerInput.breakBlockPressed =
            glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        playerInput.placeBlockPressed =
            glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        double mouseX;
        double mouseY;

        glfwGetCursorPos(nativeWindow, &mouseX, &mouseY);

        if (firstMouse)
        {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }

        float xOffset =
            static_cast<float>(mouseX - lastMouseX);

        float yOffset =
            static_cast<float>(lastMouseY - mouseY);

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        playerInput.lookDeltaX = xOffset;
        playerInput.lookDeltaY = yOffset;

        player.update(playerInput, deltaTime);

        const Camera& camera = player.getCamera();

        float aspect = 1280.0f / 720.0f;

        glm::mat4 projection =
            camera.getProjectionMatrix(aspect);

        glm::mat4 view =
            camera.getViewMatrix();

        frustum.update(projection * view);

        world.update();

        BlockRaycastHit selectedBlock;

        world.raycastBlock(
            camera.getPosition(),
            camera.getForward(),
            6.0f,
            selectedBlock
        );

        if (selectedBlock.hit)
        {
            if (player.wantsToBreakBlock())
            {
                world.setBlock(
                    selectedBlock.blockPosition.x,
                    selectedBlock.blockPosition.y,
                    selectedBlock.blockPosition.z,
                    0
                );
            }

            if (player.wantsToPlaceBlock())
            {
                world.setBlock(
                    selectedBlock.previousBlockPosition.x,
                    selectedBlock.previousBlockPosition.y,
                    selectedBlock.previousBlockPosition.z,
                    1
                );
            }
        }

        bool wireframeEnabled =
            debugMode == DebugViewMode::Wireframe ||
            debugMode == DebugViewMode::ChunkBordersAndWireframe;

        bool chunkBordersEnabled =
            debugMode == DebugViewMode::ChunkBorders ||
            debugMode == DebugViewMode::ChunkBordersAndWireframe;

        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();

        shader.setMat4("uProjection", &projection[0][0]);
        shader.setMat4("uView", &view[0][0]);

        textures.bind(0);

        if (wireframeEnabled)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        world.draw(frustum);

        if (wireframeEnabled)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (chunkBordersEnabled)
        {
            world.drawChunkBorders(
                debugRenderer,
                projection,
                view
            );
        }

        if (selectedBlock.hit)
        {
            blockSelectionRenderer.drawBlockOutline(
                selectedBlock.blockPosition,
                projection,
                view
            );
        }

        m_window->swapBuffers();
        m_window->pollEvents();
    }
}