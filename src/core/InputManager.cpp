#include "core/InputManager.h"

#include "player/Player.h"

#include <GLFW/glfw3.h>

PlayerInputState InputManager::buildPlayerInput(
    GLFWwindow* window
)
{
    PlayerInputState input;

    input.moveForward =
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

    input.moveBackward =
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

    input.moveRight =
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    input.moveLeft =
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    input.moveUp =
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    input.moveDown =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    input.breakBlockPressed =
        glfwGetMouseButton(
            window,
            GLFW_MOUSE_BUTTON_LEFT
        ) == GLFW_PRESS;

    input.placeBlockPressed =
        glfwGetMouseButton(
            window,
            GLFW_MOUSE_BUTTON_RIGHT
        ) == GLFW_PRESS;

    input.selectSlot1Pressed =
        glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;

    input.selectSlot2Pressed =
        glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;

    input.selectSlot3Pressed =
        glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;

    input.selectSlot4Pressed =
        glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;

    input.selectSlot5Pressed =
        glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS;

    input.selectSlot6Pressed =
        glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS;

    input.selectSlot7Pressed =
        glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS;

    input.selectSlot8Pressed =
        glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS;

    double mouseX;
    double mouseY;

    glfwGetCursorPos(
        window,
        &mouseX,
        &mouseY
    );

    if (m_firstMouse)
    {
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;

        m_firstMouse = false;
    }

    float xOffset =
        static_cast<float>(
            mouseX - m_lastMouseX
            );

    float yOffset =
        static_cast<float>(
            m_lastMouseY - mouseY
            );

    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;

    input.lookDeltaX = xOffset;
    input.lookDeltaY = yOffset;

    return input;
}