#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

class Window
{
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    GLFWwindow* getNativeWindow() const;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
    std::string m_title;
};