#include "core/DebugController.h"

#include <GLFW/glfw3.h>

#include <iostream>

void DebugController::update(GLFWwindow* window)
{
    bool f3Pressed =
        glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;

    if (f3Pressed && !m_f3WasPressed)
    {
        int nextMode =
            static_cast<int>(m_mode) + 1;

        if (nextMode >
            static_cast<int>(Mode::StreamingStats))
        {
            nextMode =
                static_cast<int>(Mode::Off);
        }

        m_mode =
            static_cast<Mode>(nextMode);

        std::cout
            << "Debug mode: "
            << modeToString(m_mode)
            << std::endl;
    }

    m_f3WasPressed = f3Pressed;
}

bool DebugController::isWireframeEnabled() const
{
    return m_mode == Mode::Wireframe ||
        m_mode == Mode::ChunkBordersAndWireframe;
}

bool DebugController::areChunkBordersEnabled() const
{
    return m_mode == Mode::ChunkBorders ||
        m_mode == Mode::ChunkBordersAndWireframe;
}

bool DebugController::areStreamingStatsEnabled() const
{
    return m_mode == Mode::StreamingStats;
}

const char* DebugController::modeToString(Mode mode)
{
    switch (mode)
    {
    case Mode::Off:
        return "OFF";
    case Mode::ChunkBorders:
        return "CHUNK BORDERS";
    case Mode::Wireframe:
        return "WIREFRAME";
    case Mode::ChunkBordersAndWireframe:
        return "CHUNK BORDERS + WIREFRAME";
    case Mode::StreamingStats:
        return "STREAMING STATS";
    default:
        return "UNKNOWN";
    }
}