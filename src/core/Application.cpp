#include "core/Application.h"

#include "core/DebugController.h"
#include "core/ResourceManager.h"
#include "core/InputManager.h"
#include "core/ThreadSafeQueue.h"

#include "player/Player.h"

#include "renderer/BlockSelectionRenderer.h"
#include "renderer/DebugRenderer.h"
#include "renderer/Frustum.h"
#include "renderer/HudRenderer.h"
#include "renderer/Shader.h"

#include "world/World.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

static constexpr int WINDOW_WIDTH = 1280;
static constexpr int WINDOW_HEIGHT = 720;

Application::Application()
{
    std::cout << "Starting BlockWorld..." << std::endl;

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW." << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = new Window(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "BlockWorld"
    );

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

    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress))
    {
        std::cout
            << "Failed to initialize GLAD."
            << std::endl;

        return;
    }

    std::cout
        << "OpenGL Version: "
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

    ResourceManager resources;

    if (!resources.initialize())
    {
        return;
    }

    World world(
        resources.getBlockRenderInfo()
    );

    DebugRenderer debugRenderer;
    BlockSelectionRenderer blockSelectionRenderer;
    HudRenderer hudRenderer;
    Frustum frustum;

    if (!hudRenderer.loadHotbarTextureFromJson(
        "resources/data/textures/gui/hud/hotbar.json"))
    {
        return;
    }

    if (!hudRenderer.loadHotbarSelectionTextureFromJson(
        "resources/data/textures/gui/hud/hotbar_selection.json"))
    {
        return;
    }

    DebugController debugController;
    InputManager inputManager;

    Shader shader =
        Shader::fromFiles(
            "resources/shaders/world.vert",
            "resources/shaders/world.frag"
        );

    Player player;

    float lastFrameTime = 0.0f;

    float fpsTimer = 0.0f;
    int fpsCounter = 0;

    float autosaveTimer = 0.0f;
    float streamingDebugTimer = 0.0f;

    constexpr float AUTOSAVE_INTERVAL = 300.0f;
    constexpr float STREAMING_DEBUG_INTERVAL = 1.0f;

    while (!m_window->shouldClose())
    {
        float currentFrameTime =
            static_cast<float>(glfwGetTime());

        float deltaTime =
            currentFrameTime - lastFrameTime;

        lastFrameTime = currentFrameTime;

        fpsTimer += deltaTime;
        fpsCounter++;

        autosaveTimer += deltaTime;

        GLFWwindow* nativeWindow =
            m_window->getNativeWindow();

        debugController.update(nativeWindow);

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

        if (glfwGetKey(
            nativeWindow,
            GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(
                nativeWindow,
                true
            );
        }

        PlayerInputState playerInput =
            inputManager.buildPlayerInput(
                nativeWindow
            );

        player.update(
            playerInput,
            deltaTime
        );

        const Camera& camera =
            player.getCamera();

        float aspect =
            static_cast<float>(WINDOW_WIDTH) /
            static_cast<float>(WINDOW_HEIGHT);

        glm::mat4 projection =
            camera.getProjectionMatrix(aspect);

        glm::mat4 view =
            camera.getViewMatrix();

        frustum.update(
            projection * view
        );

        world.updateAroundPlayer(
            camera.getPosition(),
            camera.getForward(),
            deltaTime
        );

        world.update(deltaTime);

        if (debugController.areStreamingStatsEnabled())
        {
            streamingDebugTimer += deltaTime;

            if (streamingDebugTimer >=
                STREAMING_DEBUG_INTERVAL)
            {
                world.printStreamingDebugStats();

                streamingDebugTimer = 0.0f;
            }
        }
        else
        {
            streamingDebugTimer = 0.0f;
        }

        if (autosaveTimer >= AUTOSAVE_INTERVAL)
        {
            world.saveWorld();

            std::cout
                << "Autosave complete."
                << std::endl;

            autosaveTimer = 0.0f;
        }

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
                if (!player.wouldBlockOverlapPlayer(
                    selectedBlock.previousBlockPosition))
                {
                    world.setBlock(
                        selectedBlock.previousBlockPosition.x,
                        selectedBlock.previousBlockPosition.y,
                        selectedBlock.previousBlockPosition.z,
                        player.getSelectedBlockId()
                    );
                }
            }
        }

        bool wireframeEnabled =
            debugController.isWireframeEnabled();

        bool chunkBordersEnabled =
            debugController.areChunkBordersEnabled();

        glClearColor(
            0.08f,
            0.10f,
            0.14f,
            1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        shader.bind();

        shader.setMat4(
            "uProjection",
            &projection[0][0]
        );

        shader.setMat4(
            "uView",
            &view[0][0]
        );

        shader.setVec3(
            "uCameraPosition",
            camera.getPosition().x,
            camera.getPosition().y,
            camera.getPosition().z
        );

        shader.setVec3(
            "uFogColor",
            0.08f,
            0.10f,
            0.14f
        );

        shader.setFloat(
            "uFogStart",
            96.0f
        );

        shader.setFloat(
            "uFogEnd",
            192.0f
        );

        resources
            .getBlockTextureArray()
            .bind(0);

        if (wireframeEnabled)
        {
            glPolygonMode(
                GL_FRONT_AND_BACK,
                GL_LINE
            );
        }

        world.draw(frustum);

        if (wireframeEnabled)
        {
            glPolygonMode(
                GL_FRONT_AND_BACK,
                GL_FILL
            );
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

        hudRenderer.draw(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            resources.getBlockTextureArray(),
            resources.getBlockRenderInfo(),
            player.getSelectedBlockId()
        );

        m_window->swapBuffers();

        m_window->pollEvents();
    }

    world.saveWorld();
}