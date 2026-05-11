#pragma once

struct GLFWwindow;

class DebugController
{
public:
    void update(GLFWwindow* window);

    bool isWireframeEnabled() const;
    bool areChunkBordersEnabled() const;

private:
    enum class Mode
    {
        Off = 0,
        ChunkBorders = 1,
        Wireframe = 2,
        ChunkBordersAndWireframe = 3
    };

private:
    static const char* modeToString(Mode mode);

private:
    Mode m_mode = Mode::Off;
    bool m_f3WasPressed = false;
};