#pragma once

#include "core/Window.h"

class Application
{
public:
    Application();
    ~Application();

    void run();

private:
    Window* m_window = nullptr;
};