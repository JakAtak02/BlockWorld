#pragma once

struct GLFWwindow;

struct PlayerInputState;

class InputManager
{
public:
    PlayerInputState buildPlayerInput(
        GLFWwindow* window
    );

private:
    bool m_firstMouse = true;

    double m_lastMouseX = 640.0;
    double m_lastMouseY = 360.0;
};